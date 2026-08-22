# 议题 55：异步虚函数在任务创建时完成动态分派

> 状态：已确认，议题 56—58、60 补充异步接口、反射、装饰器与结果不变型；Parser 议题 29 补充异步函数类型
> 确认日期：2026-08-02

## 1. 类可以声明异步虚函数

Ink 允许具体类使用 `virtual async func` 和 `override async func`：

```ink
class Loader {
    virtual async func load() -> Data* {
        return default_load(this);
    }
}

class NetworkLoader : Loader {
    override async func load() -> Data* {
        return network_load(this);
    }
}
```

该语法只把既有虚派发与议题 43、44 的惰性 `Task::<T>` 结合，不让异步函数体在调用表达式中立即执行。

## 2. 动态分派发生在调用时

通过基类指针调用异步虚函数时，在创建返回任务的调用表达式中立即完成一次虚派发：

```ink
var concrete: NetworkLoader;
var loader: Loader* = &concrete;

var task = loader->load(); // 此处选择 NetworkLoader.load
// NetworkLoader.load 的异步函数体仍未执行

const data = await task;     // 恢复已经选定的任务
```

概念顺序为：

```text
evaluate receiver and arguments
    → enter protected dispatch and load receiver vptr
    → select the final override slot and acquire its module-version pin
    → invoke that slot's task-construction thunk with the source receiver
    → let the thunk adjust this for the selected override
    → construct that override's lazy Task::<T> in the final result storage
    → transfer the acquired pin into the completed task
    → return created task
```

虚派发选择实现，但不执行异步方法体。任务在创建完成后仍处于 `created`。选择版本、取得固定和进入版本专用构造入口必须由稳定入口、epoch、hazard pointer 或等价机制组成一个受保护过程；实现不能先取得可能被卸载的裸入口，再在其后补做固定。

## 3. 第一次 `await` 不重新虚派发

第一次直接 `await`、组合等待或库级任务驱动接口只把已经构造好的任务从 `created` 推进到 `pending`，并调用任务保存的 coroutine resume 入口。

它们不能：

- 重新读取接收对象的 vptr；
- 根据当前动态类型重新选择覆盖；
- 因为热更新已经发生而改用新方法体；
- 用另一个覆盖解释已有任务帧。

因此虚派发只发生一次。后续恢复可能通过任务控制块中的普通 resume 函数指针间接调用，但那是协程状态机分派，不是再次进行面向对象虚派发。

## 4. vtable 槽是任务构造入口

同步虚函数槽直接进入同步实现；异步虚函数槽进入该覆盖的任务构造入口。概念 ABI 为：

```text
async virtual slot:
    construct_task(
        result_storage: Task::<T>*,
        dispatch_this: DeclaringClass*,
        ...arguments
    )
```

槽中可以直接保存适配 thunk，也可以保存会进入适配 thunk 的稳定入口。调用方在动态分派之前不知道最终覆盖，因此不能负责最终覆盖所需的 `this` 地址调整；该槽所选择的 thunk 负责从声明该虚槽的基类子对象地址得到实现真正需要的接收者地址。

任务构造入口负责：

- 调整接收者并保留正确的 `const` 限定；
- 按议题 44 保存实参；
- 按议题 54 保存非拥有原始 `this` 指针；
- 建立最终覆盖所需的具体 coroutine frame；
- 保存对应 resume 和 destroy 入口；
- 把分派过程已经取得的模块版本固定转移给任务；
- 在调用方提供的最终 `Task::<T>` 存储中完成构造。

这里的 `Task::<T>*` 是概念上的隐藏返回位置，不是把一个已经存在的命名任务通过指针交给方法，也不引入任务移动。

## 5. 具体帧布局可以不同

不同覆盖的方法体可以拥有不同数量和类型的局部变量，因此其 coroutine frame 大小与布局不必相同。调用方只依赖统一的 `Task::<T>` 公开表示和虚槽任务构造 ABI，不直接假定具体帧布局。

最终覆盖的任务构造入口可以取得所需帧存储，并把类型擦除后的 resume、destroy 和状态信息放入 `Task::<T>` 控制状态。这样无需让所有覆盖共享最大帧大小，也不需要在 vtable 中公开具体帧结构。

任务构造入口只有在参数、帧和运行时状态均完整建立后才向调用者交付任务，不会返回半构造任务。未完成构造的内部清理路径必须释放分派过程已经取得的模块版本固定。可恢复的资源检查必须由显式同步工厂先以普通返回状态表达，调用者确认后再创建任务；宿主内存耗尽或运行时状态损坏属于进程级致命错误。方法体需要报告业务问题时，把状态编码在逻辑结果 `T` 中并通过普通 `return` 发布，任务本身仍进入 `succeeded`。

## 6. `async` 是函数种类、函数类型和覆盖签名的一部分

覆盖必须同时满足既有虚函数签名规则和相同的函数种类：

```ink
class Base {
    virtual async func read() -> Data*;
}

class Good : Base {
    override async func read() -> Data*; // 合法
}

class BadSync : Base {
    override func read() -> Data*;       // 编译错误
}

class AlsoBad : Base {
    override func read() -> Task::<Data*>; // 编译错误
}
```

同步函数即使显式返回 `Task::<T>`，也不是异步函数覆盖。两者的虚槽调用约定、函数体执行时机和语言语义不同。

Parser 议题 29 使用 `async func(parameters) -> result` 表示异步函数值类型，其中箭头后的类型是逻辑结果，不是外层 `Task`：

```ink
async func fetch(path: StringView) -> Data;

const entry: async func(StringView) -> Data = &fetch;
var task: Task::<Data> = entry("data.bin");
```

它与同步任务工厂类型 `func(StringView) -> Task::<Data>` 不同，不能互相赋值或替代。这里的 `async` 是语言关键字，也是函数类型和调用种类的一部分。

反方向同样禁止：`async func` 不能覆盖同步虚函数。返回类型、参数、可变性和可见性等其他兼容规则继续服从普通虚覆盖规范；本议题不放宽它们。

议题 60 进一步要求异步覆盖的逻辑结果类型完全一致。即使覆盖结果可以转换为基槽结果，也不能让 `Task::<Derived*>` 代替 `Task::<Base*>`；实现应在自身返回前完成普通结果上转型并直接构造基槽声明的准确任务类型。

## 7. 接收对象生命周期

虚派发在任务创建时完成，不代表接收对象随后可以销毁。最终覆盖的任务帧仍保存议题 54 的非拥有原始 `this` 指针。

完整接收对象必须保持在原地址并存活到任务到达最终状态。基类子对象指针、最派生对象地址调整以及以后所有成员访问都依赖该完整对象仍然有效。编译器能静态证明违约时给 warning，否则由程序员负责。

## 8. 热更新规则

任务创建时以受保护的分派操作读取当时有效的虚槽，并在进入版本专用入口前固定最终覆盖的实现版本：

```text
V1 vtable selects V1 NetworkLoader.load
    → construct and pin V1 task
    → hot update publishes V2 vtable slot
    → await old task
    → resume V1 task with V1 frame ABI
```

新调用可以进入 V2；已经创建的 V1 任务继续使用 V1 的 resume、destroy、类型元数据和调试信息，直到任务完全销毁后解除模块固定。

更新系统不能只替换任务的初始虚槽入口，却卸载仍被旧任务引用的后续入口。增加、删除异步虚槽或改变同步/异步种类属于 vtable ABI 变化，遵守议题 26 的版本兼容规则。

## 9. 显式基类调用

如果语言的普通虚函数语法允许覆盖实现显式调用某个基类实现，该调用不再次根据动态类型选择最终覆盖，而是直接构造指定基类异步方法的任务。它仍然只创建惰性任务，是否立即等待由源码中的 `await` 决定。

精确的基类限定调用拼写服从未来完整成员访问语法，本议题只确定它不会形成无限虚派发递归。

## 10. 与异步接口和反射的关系

本议题只确定通过具体类继承链进行的普通源码虚调用。

议题 56 规定接口胖引用调用异步方法时也在任务创建阶段完成一次接口槽分派。接口槽最终选择类实现时，任务仍保存本议题和议题 54 的调整后原始 `this`；接口默认实现则保存规范化胖接收者。两种分派都不能推迟到第一次 `await`，但接口调用不会为了进入同时为虚函数的类实现而再读取一次主 vtable。

议题 57 规定 `[reflect]` 异步虚函数使用独立的 `call_async::<T>` 和 `DynamicTaskOut`。反射适配器在调用点验证参数并进入本议题的同一虚槽任务构造入口；函数体保持惰性，需要表达的业务状态仍属于逻辑结果 `T`。它不把 `Task::<T>` 强行塞进同步反射返回槽。

议题 58 规定异步函数装饰器包围最终覆盖的任务执行体，不包围本议题的同步任务构造过程。虚槽选择最终覆盖时直接建立该覆盖完整的装饰后 coroutine frame；第一次 `await` 才进入装饰器链和原始函数体。基类装饰器不会沿覆盖关系自动叠加。

## 11. 成本模型

未去虚拟化的异步虚调用在任务创建时承担一次普通 vtable 槽间接调用，以及本来就需要的任务和帧构造成本。

任务以后每次恢复不再读取 vptr。恢复仍可能通过 `Task::<T>` 保存的 resume 入口进行一次普通间接调用；非虚异步任务也可能需要相同机制，因此它不是重复虚派发成本。

编译器知道接收对象最终动态类型且不破坏热更新语义时，可以去虚拟化任务构造入口。普通非虚异步方法和同步非虚调用不因为语言支持异步虚函数增加对象或调用成本。

## 12. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。前端生成普通 vtable 间接调用，把隐藏的任务返回位置、虚槽声明类型的接收者指针和实参传给覆盖专用构造 thunk；thunk 再完成最终覆盖所需的接收者地址调整。

构造入口使用 LLVM coroutine intrinsics 或 Ink 自有状态机建立帧，并把 resume/destroy 函数指针写入任务控制状态。热更新入口、模块固定和任务状态由 Ink 运行时管理；LLVM 无需理解虚函数与异步任务之间的关系。

## 13. 后续问题

以下内容留给独立议题：

- vtable 异步任务构造槽的精确目标 ABI；
- 帧内联优化与虚派发之间的限制。
