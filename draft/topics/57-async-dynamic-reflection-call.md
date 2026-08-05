# 议题 57：异步动态反射调用返回惰性任务

> 状态：已确认，议题 58、60、61 补充；拥有型动态任务 API 待定  
> 确认日期：2026-08-02

## 1. `FunctionInfo` 区分同步与异步函数

动态反射函数描述符必须记录函数种类，至少区分：

```text
FunctionKind =
    sync
    async
```

异步函数的描述符记录源码声明的逻辑结果类型，而不是把调用表达式产生的任务伪装成同步返回类型：

```ink
[reflect]
async func load() -> Data*;
```

其概念描述为：

```text
kind        = async
result_type = Data*
```

而不是 `kind = sync, result_type = Task<Data*>`。这样反射签名继续保留议题 44、55 和 56 的惰性执行、创建期分派、创建期异常以及异步覆盖兼容性。

`async` 是反射调用约定的一部分。把已有函数从同步改成异步或从异步改成同步，属于反射和普通调用 ABI 变化。

议题 60 规定异步覆盖不支持协变结果，`Task<T>` 对 `T` 不变型。因此 `call_async[R]` 的 `R` 必须与描述符逻辑结果类型完全一致；最终动态实现不能改变 `DynamicTaskOut` 的结果类型。

## 2. 异步反射使用独立的 `call_async`

普通用户通过强类型便捷 API 调用被反射异步函数：

```ink
[reflect]
class Service {
    [reflect]
    virtual async func load(id: u64) -> Data*;
}

if (match .some(function) = type.function("load")) {
    var task = function.call_async[Data*](&service, 42u64);
    const data = await task;
}
```

概念签名为：

```ink
func FunctionInfo.call_async[R](
    receiver,
    arguments...
) -> Task<R>;
```

`call_async` 自身是同步函数和任务构造 API，不是 `async func`。它在调用表达式中完成描述符检查、动态分派和任务构造，然后返回处于 `created` 状态的 `Task<R>`。把该 API 本身声明为异步会无意义地形成 `Task<Task<R>>`，因此禁止这种解释。

概念流程为：

```text
FunctionInfo.call_async[R]
    → verify FunctionInfo.kind == async
    → verify logical result type == R
    → verify receiver and arguments
    → enter the ordinary static, virtual, or interface task-construction path
    → construct Task<R> directly in final result storage
    → return the lazy task without driving it
```

反射适配器不能启动、等待或同步执行目标异步函数体。第一次直接 `await`、组合等待或库级驱动仍按照议题 44 把任务从 `created` 推进到 `pending`。

议题 58 的异步装饰器属于目标函数体。`call_async` 构造最终实现完整的装饰后任务，但不在反射调用表达式中执行装饰器代码；描述符来源处的装饰器也不会作为额外反射包装层加入。

## 3. `call` 与 `call_async` 不互相替代

对异步描述符调用普通 `call[R]` 必须抛出结构化反射调用种类错误：

```ink
function.call[Data*](&service, 42u64); // 错误：目标是 async
```

正确调用为：

```ink
function.call_async[Data*](&service, 42u64);
```

反方向同样禁止：同步描述符不能通过 `call_async` 调用。反射层不能根据调用点是否立即出现 `await`、调用者期待的类型或函数名称猜测调用种类。

同步函数显式返回 `Task<T>` 时仍是普通同步任务工厂：

```ink
[reflect]
func make_task() -> Task<Data*>;
```

其描述符和调用方式为：

```text
kind        = sync
result_type = Task<Data*>
```

```ink
var task = function.call[Task<Data*>](arguments...);
```

`call[Task<T>]` 进入同步函数体并由该函数自行构造任务；`call_async[T]` 直接构造被反射异步函数的惰性任务。两种路径不能互相满足或自动转换。

## 4. `DynamicTaskOut` 直接承接最终任务

议题 21 的同步 `DynamicOut` 表示普通函数结果存储。异步反射调用需要独立的概念输出描述：

```text
DynamicTaskOut {
    address,                 // 未初始化的最终 Task<R> 存储
    expected_result_type,    // R，而不是 Task<R>
    task_abi,
    initialization_state,
}
```

强类型 `call_async[R]` 把自身隐藏的 `Task<R>` 返回位置转换为 `DynamicTaskOut`。适配器验证逻辑结果类型和任务 ABI 后，让普通异步任务构造入口直接在 `address` 中建立最终任务：

```text
async reflection adapter
    → validate DynamicTaskOut
    → select ordinary task-construction thunk
    → construct Task<R> directly in final storage
```

该协议不产生任务临时量、任务复制、隐藏移动、`Task<Task<R>>` 或拥有型 `Any`。`Task<void>` 虽然没有成功载荷，仍必须提供任务对象存储；它不能像同步 `void` 返回那样完全省略输出位置。

调用失败且最终任务尚未完成构造时，不运行任务析构。任务构造入口在部分初始化后失败时必须清理自己已经建立的帧、参数状态和版本固定，并让 `initialization_state` 保持未构造。

`DynamicTaskOut` 属于反射运行时 ABI，不要求普通业务代码手动构造。其精确位宽、对齐和 `task_abi` 表示留给目标 ABI。

## 5. 参数在反射调用时检查并捕获

`call_async` 的强类型便捷层可以继续在调用者栈上建立议题 21 的临时参数数组：

```text
receiver: optional DynamicRef
arguments: DynamicRef[]
```

异步反射适配器必须在返回任务前完成全部检查，并进入普通任务构造 thunk。它不能把 `DynamicRef[]`、反射调用者的临时借用或参数数组地址保存进任务，等第一次 `await` 再读取。

因此：

- 实参表达式在调用点立即求值；
- 参数数量、类型、传递方式和可变性错误同步抛出；
- 可复制按值参数由普通异步调用规则复制进任务帧；
- 原始指针和原始切片按值保存，并继续由程序员保证目标生命周期；
- 普通引用可以按照议题 04、44 作为非拥有别名保存，动态反射不会延长目标生命周期；
- 安全切片和命名 `[noncopyable]` 值不能借反射绕过议题 44 的对应长期捕获限制；
- 动态反射不执行用户定义隐式构造或数值强制转换。

异步函数声明本身不满足普通异步参数和结果约束时已经是编译错误；添加 `[reflect]` 不会放宽该检查。

## 6. 普通、虚和接口目标使用既有任务构造路径

非虚自由函数、静态函数或非虚成员函数的异步反射适配器在完成检查后，直接进入该声明版本的普通任务构造入口。非虚成员任务继续保存议题 54 的非拥有原始 `this`。

异步虚函数遵守议题 25 和 55 的两阶段选择：

```text
base async FunctionInfo
    → validate receiver and arguments
    → read receiver vtable at call_async time
    → select the final override task-construction thunk
    → construct that override's Task<R>
```

即使描述符来自基类，最终仍选择接收对象最具体的覆盖。覆盖没有自己的 `[reflect]` 描述符时，也可以通过继承的基类描述符到达；元数据来源和最终实现身份仍彼此独立。第一次 `await` 不重新读取 vtable。

异步接口方法遵守议题 28 和 56：

```text
interface async FunctionInfo
    → validate or construct the interface view
    → read the interface slot at call_async time
    → construct the selected Task<R>
```

槽选择类实现时，任务保存调整后的原始 `this`；槽选择接口默认方法时，任务保存规范化到默认方法声明接口的胖接收者。完全动态 `DynamicRef` 仍须按照议题 28 从已注册实现关系构造正确接口视图，不能从无类型地址猜测。

反射适配器不能在第一次恢复时重新执行名称查找、类虚分派或接口分派，也不能因为类实现同时是虚函数而在接口槽之后增加第二次主 vtable 分派。

## 7. 创建期异常与任务失败分离

以下问题发生在任务成功创建前，按照普通同步异常传播：

- `FunctionInfo.kind` 与所用调用 API 不匹配；
- 调用者期待的逻辑结果类型 `R` 不匹配；
- 接收对象、参数数量、类型、传递方式或可变性错误；
- 描述符、对象布局、接口表或模块版本不兼容；
- 目标模块已经开始卸载；
- 实参捕获、任务帧取得或任务基础状态建立失败。

其中合法反射 API 输入产生的种类、类型和版本错误使用议题 21 的结构化反射异常。资源取得和用户参数复制本身产生的异常保持其普通动态类型，不强制包装成反射异常。

任务成功构造后，目标异步函数体以后抛出的异常由议题 43 的任务边界保存为：

```text
Task<R>.failed(ExceptionBox)
```

等待该任务时重新传播原始业务异常。反射层不能把目标函数体异常包装成统一的 `ReflectionInvocationError`，也不能丢失原异常的动态类型、接口、原因链、抛出位置或 traceback。

如果 `call_async` 表达式本身位于另一个异步函数体内，其同步创建期异常可能使外层任务失败；这仍是外层函数执行期间的异常，不是尚未成功创建的内层任务失败。

## 8. 接收者、参数和描述符生命周期

创建期使用的 `DynamicRef`、参数数组和 `FunctionInfo` 描述符借用只需保持到 `call_async` 返回或抛出。任务构造成功后，任务已经保存普通异步 ABI 所需的参数快照和接收者表示，不再保存反射调用者栈上的动态包装。

这不延长接收对象或原始指针目标的生命周期。异步成员接收对象仍必须保持在原地址并存活到任务最终状态；原始指针参数继续服从其公开前置条件。接口默认任务复制胖接收者并固定所需表版本，但不取得完整对象所有权。

任务必须持有最终所选构造、resume、destroy、异常边界和接口表所需的版本固定。反射适配器和描述符不被任务继续使用时，其创建期借用或固定可以在任务构造完成后释放。

## 9. 热更新

以下状态必须作为兼容的模块更新事务发布：

- `FunctionInfo.kind`、逻辑结果类型和参数描述；
- 同步或异步反射调用适配器；
- 对应普通入口、虚槽或接口槽；
- 具体实现关系和必要接口表；
- 反射名称注册项。

`call_async` 必须在受保护过程内验证描述符、取得适配器并进入最终任务构造入口。实现不能先保存可能被卸载的裸适配器或槽入口，再在其后补做版本固定。

任务创建成功后固定最终实现和帧 ABI。兼容热更新只影响以后发起的反射调用；已经创建的旧任务继续使用旧 resume、destroy、异常描述符和调试信息。默认接口任务还按照议题 56 保持其规范化接口表布局有效。

增加、删除或改变参数，改变逻辑结果类型、同步/异步种类、虚槽或接口槽调用约定，均属于反射调用 ABI 变化。旧描述符、旧对象或旧任务不能按新版本布局盲目解释。

## 10. 不引入拥有型 `DynamicTask`

普通用户通过 `call_async[R]` 已能在编译期指定期待的结果类型，并取得正常 `Task<R>`。本议题不向核心语言增加拥有型、可复制或引用计数的 `DynamicTask`/`AnyTask`。

结果类型完全到运行时才知道的编辑器、RPC 框架或插件系统可以使用底层 `DynamicTaskOut`、类型描述符和未来确定的动态任务驱动 ABI。公开的拥有型动态任务封装如果需要，还必须单独决定：

- 类型擦除结果存储和析构；
- 任务所有权、共享与复制成本；
- 动态等待后的结果访问；
- 后台调度与销毁规则；
- 堆分配和引用计数是否必需。

这些成本不能由普通异步反射调用隐式引入。

## 11. 成本模型

与普通异步调用相比，异步反射调用额外承担：

- 名称或描述符查找；
- 函数种类、逻辑结果、接收对象和参数检查；
- 一次反射适配器间接调用；
- 目标原本需要的静态、虚槽或接口槽任务构造路径。

它不要求额外任务对象、拥有型 `Any`、结果装箱或反射层堆分配。目标任务本身取得 coroutine frame 和运行时状态的成本仍与普通异步调用相同。

任务以后每次恢复不再经过反射适配器。编译器在描述符和具体实现可证明且不破坏热更新语义时可以消除部分检查或间接层，但不能改变错误检查、最终分派和任务版本固定语义。

## 12. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。前端和反射运行时生成同步的类型擦除适配器，适配器验证 `DynamicRef[]` 与 `DynamicTaskOut`，然后调用普通异步任务构造 thunk。

最终 thunk 使用现有 coroutine lowering 或 Ink 自有状态机直接在输出位置建立 `Task<R>`。函数体异常仍由任务内部 catch-all 边界捕获；创建期反射异常通过普通 LLVM `invoke`、landing pad、personality 或 Windows funclet 路径传播。

LLVM 无需理解 `FunctionInfo`、逻辑异步结果类型、接口继承或 `ExceptionBox`。这些由 Ink 前端、反射描述符和运行时协议实现。

## 13. 后续问题

以下内容留给独立议题：

- `DynamicTaskOut`、任务 ABI 句柄和初始化状态的精确二进制布局；
- 拥有型 `DynamicTask`/`AnyTask` 及完全动态结果访问 API；
- 泛型异步函数和泛型异步接口方法按照议题 61 在编译期形成闭合实例；运行时反射只调用已经生成并登记的闭合实例，不执行新的泛型实例化；
- 可变参数、生成器和异步生成器的动态调用；
