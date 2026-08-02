# 议题 43：可重复等待的 `Task`、结果约束与 `ExceptionBox`

> 状态：已确认，议题 44—61、69 补充，公开 `ExceptionBox` API 待定  
> 确认日期：2026-08-02

## 1. 本议题的范围

本议题确定异步任务完成状态与异常之间的语言级最低契约：

- `Task<T>` 默认允许重复 `await`；
- 完成状态不会因为第一次 `await` 被消费；
- 按值任务结果要求 `T: Copy`；
- 不可复制资源通过原始指针或未来显式拥有型指针返回；
- 失败任务使用只读、共享的 `ExceptionBox` 长期保存异常；
- 每次失败的 `await` 创建独立的活动展开记录。

异步函数声明语法、协程帧布局、运行时调度机制、组合等待的失败策略和取消请求 API 不在本议题中确定。议题 52 已将高层调度、后台任务和任务组划归库设计。

## 2. 任务保存不可逆的完成状态

`Task<T>` 至少具有以下逻辑状态：

```text
TaskState<T> =
    created
    pending
    succeeded(ResultStorage<T>)
    failed(ExceptionBox)
```

议题 44 将 `created` 确定为尚未执行的惰性状态。任务通过第一次直接 `await`、议题 45 的 `await all(...)` 或调度库使用的底层任务驱动协议从 `created` 进入 `pending`，再转换为一个最终状态。`Task<T>` 不提供公开的 `start()`。最终状态发布后不可重新写回、替换或恢复为 `pending`。

成功结果和失败载荷都在发布后只读。任务状态的发布必须与所有成功等待者同步，使等待者能够看到任务完成前对结果存储所做的初始化。

议题 48 确定取消只是一项协作式请求，不增加独立终态或内建取消异常。任务响应请求后仍通过普通返回进入 `succeeded`，或通过普通业务异常进入 `failed`。

## 3. `Task<T>` 默认可重复 `await`

对同一个已完成任务重复 `await` 是合法操作：

```ink
var task = load_async();

let first = await task;
let second = await task;
```

第一次 `await` 不会：

- 使 `task` 绑定失效；
- 从任务中取走完成状态；
- 把任务变成“已经移动”或“已经观察”的特殊状态；
- 阻止后续等待者读取同一终态。

这与议题 02 不提供通用移动语义的规则一致。`await` 是观察任务的终态，不是消费任务变量。

如果任务仍处于议题 44 的 `created` 状态，第一次直接 `await` 或组合等待还负责只驱动一次函数体；后续等待者不会重新执行任务。

## 4. 成功结果按普通复制返回

对于非 `void` 的按值任务结果，`T` 必须满足 `Copy`。每次成功的 `await` 都按照议题 02 的普通复制规则，从任务的只读结果存储构造当前表达式结果：

```ink
var task: Task<int> = calculate_async();

let first: int = await task;
let second: int = await task;
```

这种复制不能隐藏：

- 内存分配；
- 引用计数修改；
- 用户复制函数；
- 资源复制；
- 可能失败的操作。

`Task<void>` 不包含成功载荷，不受 `T: Copy` 约束。

安全引用和安全切片不能作为长期任务结果保存，因为它们不能逃逸同步词法范围。原始指针和原始切片如果自身满足 `Copy`，可以作为任务结果；其目标生命周期仍由程序员负责。

## 5. 不可复制结果通过指针表达

`Task<T>` 不因为异步返回而为不可复制类型引入隐藏移动：

```ink
var invalid: Task<File>;  // 编译错误：File 不可复制
var valid: Task<File*>;   // 合法：复制并返回原始指针
```

重复等待 `Task<File*>` 返回相同的指针值，不复制 `File`，也不使第一次取得的指针失效。

`File*` 仍然是非拥有指针。任务销毁不会仅因结果类型是 `File*` 而销毁 `File`；创建者或库定义的其他拥有者必须保证目标对象存活。未来的显式拥有型指针可以作为另一种任务结果，但其所有权交接规则必须单独设计，不能通过 `Task<T>` 隐式引入移动。

## 6. `Task<T>` 本身不通过复制隐藏共享成本

默认 `Task<T>` 是不可复制的运行时对象。概念声明为：

```ink
[noncopyable]
class Task<T: comptime type> {
    // 运行时私有状态
}
```

异步调用可以利用议题 02 的保证原地构造，直接在接收位置建立任务对象：

```ink
var task = load_async();
```

把 `Task<T>` 赋给另一个命名变量、按值传参或存入 `Vector<Task<T>>` 不会隐式增加任务状态引用计数，而是编译错误。需要从其他位置长期访问同一任务时使用 `Task<T>*`；指针使用者负责保证任务对象生命周期。核心语言不定义共享任务句柄，库可以在现有生命周期规则上定义并明确支付自己的包装成本。

“任务可被重复等待”只描述同一任务对象的终态观察能力，不等于任务句柄可以隐式复制。

议题 60 规定任务结果类型同样不参与方差转换。`Task<Derived*>` 与 `Task<Base*>` 是两个不兼容的具体任务类型；继承指针转换必须在异步函数发布结果前完成，不能通过任务赋值或隐式代理发生。

议题 69 的元组结果按照元素组合复制能力。`Task<(A, B)>` 只有在整个元组可复制且不包含会逃逸的安全引用时才合法；任务不会为了异构结果引入隐藏移动或拥有型元组装箱。

## 7. 任务边界捕获失败

异步任务执行入口外围由编译器或运行时建立内部 catch-all 边界：

```text
run async body
    → success: publish ResultStorage<T>
    → exception: capture current exception into ExceptionBox
```

该边界属于任务结果协议，不是议题 42 的普通线程致命边界。异常到达任务边界时：

- 不立即终止进程；
- 不把异常展开到启动任务的线程；
- 不转换成某个固定公共异常基类；
- 保留原动态类型、接口、原因链、抛出位置和 traceback；
- 将任务原子发布为 `failed`。

任务边界之外的库调度工作线程或普通 OS 线程入口仍必须满足议题 42。运行任务所用的线程不能让捕获逻辑之外的异常逃出普通线程入口。

## 8. `ExceptionBox` 的定位

`core.ExceptionBox` 是运行时拥有、不可复制、不可变的长期异常容器。它不是异常类、异常接口或 `ExceptionView`，也不能作为普通异常对象直接 `throw`。

概念内容为：

```text
ExceptionBoxControl {
    atomic_internal_references
    exception_descriptor
    immutable_payload
    cause_graph
    throw_site
    traceback
    module_version_pins
}
```

用户代码不能：

- 直接构造或伪造 `ExceptionBox`；
- 从任意普通对象建立 `ExceptionBox`；
- 把当前 `catch` 的借用视图强制转换成拥有型 box；
- 修改 box 中的异常载荷、原因链或诊断数据；
- 通过普通赋值复制 box 并隐藏引用计数。

任务边界处理的是运行时已经拥有的活动异常记录。它可以把该记录及其完整原因链重新归属到 `ExceptionBox`，这与议题 40 的运行时记录重新归属相同，不是源语言对象移动。

## 9. 已经 box 化的异常可以直接复用

异步函数可能因为 `await` 另一个失败任务而再次失败。此时活动异常已经引用一个 `ExceptionBox`：

```text
inner Task.ExceptionBox
    → failed await carrier
    → exception escapes outer async body
    → outer task boundary
```

外层任务边界不复制异常载荷，也不创建第二份同类型异常对象。运行时保留原 box 的内部所有权，并把外层任务发布为引用该失败信息的 `failed` 状态；当前活动展开载体随后可以释放。

运行时可以为异步传播链保存额外边界诊断，但不能改写共享 box 中最初的 `throw_site` 或把某个 `await` 位置伪装成新的原始抛出点。

## 10. 每次失败的 `await` 使用独立展开记录

同一个平台异常展开对象不能同时在两个调用栈上活动。因此每次对失败任务执行 `await` 时，运行时建立当前传播专用的逻辑记录：

```text
shared ExceptionBox
    ├── ActiveUnwindRecord A → waiter A
    └── ActiveUnwindRecord B → waiter B
```

每个 `ActiveUnwindRecord` 至少包含：

- 目标平台要求的展开头或等价状态；
- 对共享 `ExceptionBox` 的内部持有；
- 当前 `await` 位置；
- 当前消费者侧传播所需的私有状态。

异常匹配读取 box 保存的动态异常描述符。类型化 `catch` 借用 box 中的只读载荷，接口捕获使用 box 固定的接口表，`catch as error` 建立当前活动记录的 `ExceptionView`。

捕获语法和匹配结果与同步抛出一致；程序不需要知道当前异常来自同步 `throw` 还是失败任务。

## 11. 重新抛出与 `from`

对异步失败建立的活动记录执行 `throw;` 时，只继续当前等待者的展开：

- 不修改共享 `ExceptionBox`；
- 不影响其他等待者；
- 不从任务中移除失败状态；
- 不重新复制异常载荷。

处理器执行 `throw NewError { ... } from error` 时，议题 40 的新异常记录取得当前活动记录作为原因。该原因记录唯一属于新异常链，但它内部持有共享 box；因此原始不可复制载荷仍然不被复制，任务和其他等待者也仍可观察原失败。

普通 `catch` 结束且没有继续传播时，只释放当前活动展开记录对 box 的内部持有。任务自己的失败状态仍然保留。

## 12. 抛出位置与异步传播位置分离

`ExceptionBox` 永久保留异常最初的：

- 动态类型；
- `throw_site`；
- 生产者侧 traceback；
- 完整显式原因链。

每次失败的 `await` 可以在活动展开记录中保存当前 `await` 源码位置和消费者侧同步传播信息。它们是一次观察的传播上下文，不写回共享 box，也不改变议题 41 的“重新抛出不重写原始位置”规则。

诊断器可以把生产者侧异常栈和本次等待者侧传播栈分段显示。异步边界链的精确公开视图和符号化格式留给诊断 ABI。

## 13. 生命周期、模块固定与跨线程同步

失败任务持有 `ExceptionBox` 的一个运行时内部所有权。每个活动展开记录在传播期间持有另一个内部所有权。最后一个内部所有权释放时，运行时按顺序：

```text
destroy immutable exception payload and cause graph
    → release diagnostic storage
    → release exception descriptors and module version pins
    → free ExceptionBox storage
```

内部所有权计数必须能够处理跨线程等待，通常使用原子操作或等价同步。它不是 Ink 普通对象复制规则的一部分，用户复制变量不会触发该计数。

异常载荷和原因链在 box 化后永久只读，因此多个等待者可以安全读取同一载荷。旧模块版本必须保持固定，直到任务失败状态和全部活动等待者都释放相关 box；热更新不能用同名新类型解释旧载荷。

## 14. 并发等待

在任务对象和其结果指向对象的生命周期前置条件得到满足时，不同线程或任务可以同时等待同一个 `Task<T>`：

- `pending` 状态可以登记多个 continuation；
- 完成时唤醒所有已登记等待者；
- 成功等待者各自复制只读的 `T`；
- 失败等待者各自获得独立展开记录；
- 一个等待者捕获或重新抛出不改变其他等待者的结果。

通过原始 `Task<T>*` 跨线程访问时，程序员仍负责目标任务对象没有被并发销毁，并满足地址与同步前置条件。议题 51 已确定：销毁前必须解除等待登记并保证任务到达最终状态，析构 `pending` 任务触发致命 trap；绕过同步而使用悬空或竞态原始指针仍可能产生 UB。

## 15. 与进程级 fail-fast 的关系

任务函数异常到达内部任务边界并成功存入 `ExceptionBox`，属于已捕获失败，不触发议题 42 的进程级 fail-fast。

对失败任务执行 `await` 后，异常从等待点开始按照普通未检查异常传播。如果它最终逃出：

- 另一个任务边界，则存入外层任务失败状态；
- `main`、普通 OS 线程或 `[nothrow]` 边界，则按照议题 42 fail-fast；
- 普通 `catch`，则由该处理器正常处理。

议题 53 规定核心 `Task<T>` 不记录失败是否曾被观察。任务进入 `failed` 或在该状态下析构都不打印、不抛出、不触发 warning 或进程级 fail-fast；析构只释放任务对 `ExceptionBox` 的内部持有。库级后台设施可以在自己的对象中实现独立报告策略。

议题 47 的组合等待会检查全部输入任务的最终状态，但只按参数顺序为当前等待者传播第一个失败的 `ExceptionBox`。该检查不向任务写入 observation bit；未被选中的命名任务仍保留各自失败状态，未被选中的临时失败在组合清理时释放，不建立隐式异常链。

## 16. 成本模型

成功任务的 `await` 至少需要任务完成同步和一次普通 `T` 复制；等待尚未完成的任务还需要登记并恢复 continuation。复制成本受 `T: Copy` 限制，不执行隐藏分配、引用计数或用户代码。

失败任务第一次和后续每次 `await` 都可能承担：

- `ExceptionBox` 内部原子持有和释放；
- 当前传播专用展开记录的取得与释放；
- 普通异常处理器搜索和栈展开；
- 当前 `await` 位置的诊断记录。

任务首次失败还需要把活动异常记录转成 box 并长期固定相关模块版本。重复等待不会复制异常载荷或原因链。

这些成本属于明确使用任务和异常传播的成本，不添加到普通同步函数调用、普通对象赋值或从未失败的非异步代码路径。

## 17. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。Ink 可以使用：

- LLVM 现有协程 intrinsics，或由前端直接生成状态机；
- 现有 `invoke`、landing pad、personality 和 Windows funclet 表达任务边界捕获；
- 普通原子指令发布任务终态和管理内部持有；
- 目标已有异常 ABI 为每个失败 `await` 建立独立展开载体。

概念降低为：

```text
task_entry:
    invoke async_body
      normal  → publish_success
      unwind  → capture_or_retain_exception_box
                publish_failure

await task:
    pending   → register continuation and suspend
    succeeded → copy T from immutable result storage
    failed    → retain box, create unwind carrier, start exception propagation
```

LLVM 不需要理解 `ExceptionBox`、Ink 异常类、原因链或任务重复等待语义；这些由 Ink 前端、personality 和运行时实现。

## 18. 后续问题

以下内容留给后续议题：

- `async` 函数、`await` 表达式和任务类型的完整语法与帧 ABI；
- 底层任务驱动、稳定存储构造和 coroutine handle ABI；
- 底层取消通知 ABI 和批量请求传播；
- `ExceptionBox` 是否提供用户可见的拥有型查询 API；
- 未来显式拥有型指针；
- 异步 traceback 边界的精确 API 和显示格式；
- freestanding、内核及无原子运行时目标的任务支持；
- 任务状态和 `ExceptionBox` 的精确二进制 ABI。

高层 `spawn`、detach、join、任务组、调度器关闭和控制句柄已由议题 52 划归库设计，不再作为核心语言待定项。

核心任务的失败观察状态和析构报告策略已由议题 53 确定，不再作为待定项。
