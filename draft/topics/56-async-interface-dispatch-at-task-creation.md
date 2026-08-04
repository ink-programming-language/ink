# 议题 56：异步接口方法在任务创建时完成接口分派

> 状态：已确认，议题 57、58、60 补充异步反射、装饰器与结果不变型  
> 确认日期：2026-08-02

## 1. 接口可以声明异步方法

Ink 允许接口把必需方法和默认方法声明为 `async func`：

```ink
interface DataSource {
    async func load(name: StringView) -> Data*;

    async func reload(name: StringView) -> Data* {
        return await load(name);
    }
}
```

实现类使用 `override async func` 满足异步接口要求或覆盖异步默认方法：

```ink
class HttpSource : DataSource {
    override async func load(name: StringView) -> Data* {
        return await http_get(name);
    }
}
```

异步接口用于在不知道具体实现类型时表达可能暂停的操作。文件、网络、数据库、定时器、消息队列和测试替身可以实现同一契约；调用方可以统一等待或组合它们的任务。`async` 本身不创建线程、不自动并发，也不决定任务在哪个执行器上运行，调度仍遵守议题 44、45 和 52。

## 2. 接口分派发生在任务创建时

通过接口胖引用调用异步方法时，调用表达式立即读取接口槽并构造槽所选择实现的惰性任务：

```ink
async func import_data(source: DataSource&) -> Data* {
    var task = source.load("users.dat"); // 此处完成接口分派并创建任务
    // HttpSource.load 的异步函数体仍未执行
    return await task;                  // 驱动已经选定的任务
}
```

概念顺序为：

```text
evaluate interface receiver and arguments
    → enter protected interface dispatch
    → read the async interface slot and acquire the selected version pin
    → invoke the slot's task-construction thunk
    → normalize or adjust the receiver required by the selected implementation
    → construct the selected implementation's lazy Task<T>
    → transfer all required implementation/table pins into the task
    → return the created task
```

接口分派只选择实现并建立任务，不执行异步方法体。任务创建后仍处于 `created`。第一次直接 `await`、组合等待或库级任务驱动只恢复已建立的任务，不能重新读取原始接口槽或改选另一个实现。

## 3. 异步接口槽是任务构造入口

同步接口槽进入同步实现或适配 thunk；异步接口槽进入最终实现的任务构造 thunk。概念 ABI 为：

```text
async interface slot:
    construct_task(
        result_storage: Task<T>*,
        object: void*,
        dispatch_table: InterfaceTable*,
        arguments...
    )
```

`object` 和 `dispatch_table` 来自议题 26 的两字接口胖引用。`Task<T>*` 是隐藏最终返回位置，不表示把已有命名任务借给方法，也不引入任务移动。

议题 57 的异步接口反射适配器在调用点验证 `DynamicRef`、逻辑结果类型和接口实现关系，然后进入同一个槽构造 ABI。反射路径不能改变任务接收者表示，也不能推迟接口分派。

议题 58 的异步装饰器属于槽最终选择实现的任务执行体。类实现只运行类方法自己的装饰器；接口默认实现只运行默认方法自己的装饰器。没有函数体的接口要求不能用普通异步函数体装饰器建立契约级拦截。

任务构造 thunk 负责：

- 保留接收者和实参的可变性及 `const` 限定；
- 按议题 44 在调用时保存实参；
- 建立最终实现所需的具体 coroutine frame；
- 保存对应 resume、destroy 和异常边界入口；
- 转移受保护分派已经取得的版本固定；
- 在调用方提供的最终 `Task<T>` 存储中完成构造。

不同接口实现和默认方法可以具有不同帧布局。调用方只依赖统一的 `Task<T>` 表示和接口槽构造 ABI，不依赖具体帧大小。

## 4. 类实现只保存调整后的原始 `this`

异步接口槽最终指向类实现时，适配 thunk 根据完整对象规范地址得到实现类所需的接收者地址，并按议题 54 把它作为非拥有原始 `this` 指针保存进任务帧：

```text
{ object, Class-as-DataSource table }
    → select HttpSource.load construction thunk
    → adjust object to HttpSource*
    → construct HttpSource.load task containing raw this
```

类异步方法体使用普通类成员语义，不需要仅仅因为调用来源是接口就永久保存接口表。任务构造成功并取得实现版本固定后，如果具体类帧以后不再读取该接口表，创建期的接口表分派保护可以释放。

一个类方法同时是类虚函数和接口实现时，具体类—接口表直接为该动态具体类选择最终类覆盖的任务构造 thunk。接口调用不能先选择接口槽、再在任务第一次恢复时读取主 vtable 进行第二次分派。

## 5. 默认异步方法保存接口胖接收者

接口默认方法没有具体类 `this`。如果默认方法以后会调用同一接口的其他方法，其任务帧必须保存非拥有接口胖接收者：

```text
default async receiver = {
    object: pointer,
    receiver_table: pointer,
}
```

例如：

```ink
interface Storage {
    async func read(path: StringView) -> Data*;
    async func write(path: StringView, data: Data*);

    async func copy(source: StringView, destination: StringView) {
        const data = await read(source);
        await write(destination, data);
    }
}
```

调用 `copy` 只创建默认方法任务。该任务以后开始执行时，`read` 和 `write` 才各自通过保存的接口表进行新的正常接口分派。这些嵌套调用不是对原始 `copy` 的重新分派。

## 6. 默认接收者规范化到声明接口

议题 30 规定不同接口具有逻辑上独立的接口表，父接口表不是子接口表的内存前缀。因此，继承来的默认异步方法不能把调用点的任意子接口表直接当作其声明接口表使用。

```ink
interface A {
    async func read() -> Data*;

    async func reload() -> Data* {
        return await read();
    }
}

interface B : A {
    override async func read() -> Data* {
        return await read_from_b();
    }
}
```

通过 `B&` 调用继承的 `A.reload` 时，`B` 接口槽的适配 thunk 必须把接收者规范化为同一具体实现类对应的 `A` 接口视图，再把它保存进默认任务：

```text
{ object, Concrete-as-B table }
    → select inherited A.reload
    → normalize to { object, Concrete-as-A table }
    → construct A.reload default task
```

`Concrete-as-A` 表按照该具体类实现 `B` 后的有效方法集合生成，因此其中的 `A.read` 槽仍指向由 `B` 或具体类选定的最终实现。这样共享的 `A.reload` 函数体可以按照 `A` 的固定表布局调用 `read`，同时保留子接口覆盖语义。

规范化可以由子接口表的祖先转换项、具体类—接口表专用 thunk 或等价常数时间机制完成。语言只要求得到正确的声明接口视图，不规定精确二进制实现。

如果默认实现由子接口重新声明，则任务保存该新默认方法声明接口的视图。重新抽象的方法没有默认函数体，必须由类实现，因而遵守第 4 节。

## 7. `async` 是接口实现兼容性的一部分

异步接口要求只能由兼容的异步方法满足：

```ink
interface Reader {
    async func read() -> Data*;
}

class Good : Reader {
    override async func read() -> Data*; // 合法
}

class BadSync : Reader {
    override func read() -> Data*;       // 编译错误
}

class AlsoBad : Reader {
    override func read() -> Task<Data*>; // 编译错误
}
```

同步返回 `Task<T>` 的普通函数是任务工厂，不是 `async func`。它可能在调用表达式中执行同步函数体，并具有不同的接口槽调用约定和创建期异常边界。

异步默认方法只与兼容的异步要求和异步默认方法参与议题 29、30 的满足、覆盖、合并、冲突消解及重新抽象。同步方法不能静默满足异步要求，异步方法也不能静默满足同步要求。同步与异步同名声明能否作为普通重载共存，继续服从完整函数重载规则；本议题不使用 `await` 是否出现或期望的 `Task` 结果来隐式消歧。

议题 60 规定其中的结果兼容要求为完全一致。异步接口实现、默认覆盖和重新抽象不能使用协变逻辑结果，也不能生成接口视图专用的结果转换任务。

## 8. 直接类调用

类显式实现异步接口方法后，可以按普通类方法语法调用该实现；此时接收者和任务帧遵守议题 54。

类只继承一个无冲突异步默认方法时，也可以通过类值直接调用。编译器必须选择议题 29、30 已确定的接口实现关系，构造相应接口视图，并建立与接口调用等价的默认任务。编译器可以在保持内部接口分派、接收者规范化和热更新语义时省略物化胖引用或去虚拟化任务构造入口。

## 9. 接收对象和接口表生命周期

异步接口任务不拥有完整实现对象。无论最终选择类实现还是默认实现，对象都必须保持在原地址并存活到任务达到 `succeeded` 或 `failed` 最终状态。提前销毁对象违反任务接收者契约；以后通过保存的原始指针访问属于悬空访问和 UB。编译器能从局部控制流证明违约时给出 warning，否则由程序员负责。

创建任务的源接口引用变量本身可以在调用表达式后结束生命周期。类实现任务已经保存调整后的对象指针；默认实现任务已经复制规范化后的两字接口接收者。这些复制都不取得对象所有权。

任务必须在内部持有足以让其所选构造、resume、destroy、异常边界和以后所需接口表保持有效的版本固定。类实现帧不再使用接口表时不要求继续固定源分派表；默认帧保存 `receiver_table` 时必须同时保持该表的布局版本有效。必要固定可以涉及实现代码和接口表所属的不同模块，公开语义不要求把它们表示成单个或多个用户可见句柄。

实现代码和表所需固定至少持续到任务完全销毁。接收对象只需按照公开契约存活到任务最终状态，因为最终状态后的任务销毁不能再访问其成员状态。

## 10. 热更新

异步接口调用必须在受保护操作中读取当时有效的接口槽、取得所选实现版本固定并进入版本专用任务构造 thunk。实现不能先取得可能被卸载的裸入口，再在其后补做固定。

任务构造成功后，原始方法选择已经固定：

```text
V1 interface slot selects V1 async implementation
    → construct and pin V1 task
    → compatible update publishes V2 slot entry
    → await old task
    → resume V1 task with V1 frame ABI
```

旧任务不能因为接口槽已经更新而把自身帧解释成 V2。默认任务以后从保存的 `receiver_table` 主动调用其他接口方法时，那些调用是新的接口调用，并分别遵守议题 26 的稳定入口和更新事务规则；它们不会改变原始默认任务已经选定的函数体和帧版本。

增加、删除或重新排序接口方法，改变方法签名，改变同步/异步种类，或者改变有效接口表和祖先转换关系，都属于接口 ABI 变化。旧任务保存的表只能按其原布局版本解释，不能直接改作新布局。

任务构造失败时不会返回半构造任务，并必须释放分派过程中已经取得的全部版本固定。

## 11. 异常、取消和调度

实参求值、受保护接口分派和任务帧取得发生在任务创建阶段；其中的失败属于议题 44 的同步调用失败。异步方法体开始执行后抛出的异常由任务边界捕获为 `failed(ExceptionBox)`。

接口分派不改变议题 48—51 的取消语义。请求取消仍只是对已经创建任务的协作式请求；接口默认方法是否显式把请求传播给它等待的子任务，遵守普通异步函数和库组合器规则。

接口 `async` 也不指定调度器、线程池或后台执行策略。调用方可以先从多个接口值创建任务，再使用 `all` 或库级执行器并发驱动；接口本身只提供统一的异步任务契约。

## 12. 成本模型

未去虚拟化的异步接口调用在任务创建时承担一次接口表槽间接调用，以及本来就需要的任务和帧构造成本。第一次和后续恢复通过任务控制状态的 resume 入口进行，不再读取原始接口槽。

最终选择类实现时，任务帧通常只增加一个非拥有接收者指针。最终选择默认实现时，任务帧通常保存对象指针和规范化接口表指针两个机器字，并持有保持相关代码和表有效的内部版本固定。

默认方法体每次调用其他接口方法仍承担正常接口槽分派成本。编译器知道具体类型或最终槽且不破坏覆盖、接收者规范化及热更新语义时，可以去虚拟化或合并重复存储。

## 13. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。前端生成普通接口表间接调用，把隐藏任务返回位置、完整对象地址、分派接口表和实参传给任务构造 thunk。

类实现 thunk 调整对象地址并建立保存原始 `this` 的具体 coroutine frame；默认实现 thunk 取得正确的声明接口表并建立保存胖接收者的帧。resume/destroy 入口、异常边界、模块固定和热更新保护由 Ink 运行时与现有 coroutine lowering 配合实现，LLVM 无需理解接口继承关系。

## 14. 后续问题

以下内容留给独立议题：

- 泛型异步接口方法和默认方法的实例化；
- 同步与异步同名方法在完整重载系统中的可表达性；
- 异步接口槽、表版本固定集合和默认接收者帧的精确目标 ABI；
- 拥有型接口容器如何把对象所有权传递给异步任务。
