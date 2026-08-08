# 议题 59：Ink v0 不提供任务构造装饰器

> 状态：已确认，创建期自定义行为使用显式同步任务工厂  
> 确认日期：2026-08-02

## 1. 只保留同步创建与异步执行两个边界

Ink v0 的异步调用继续分成两个明确阶段：

```text
synchronous creation
    → evaluate and capture arguments
    → select the final implementation
    → allocate and construct the task
    → return created Task::<T>

asynchronous execution
    → run async decorator regions
    → run the original async body
    → publish the final task state
```

议题 58 的 `async decorator` 只参与第二阶段。语言不提供 `task_constructor decorator`、`task decorator` 或其他能够围绕同步任务构造 thunk 插入用户代码的装饰器种类。

因此创建期异常始终按照议题 44 同步传播；异步装饰器和原始函数体的未捕获异常始终由任务边界保存为 `failed(ExceptionBox)`。不增加同时跨越两个阶段的第三种用户 continuation 边界。

## 2. 任务构造不开放用户 continuation

假想的构造装饰器可能写成：

```ink
task_constructor decorator observe() {
    log("creating");
    const task = function(...);
    log("created");
    return task;
}
```

Ink v0 不接受该声明。`Task::<T>` 是议题 43 的不可复制对象，普通局部 `task` 不能复制或隐式移动到最终返回位置。若要支持该语法，特殊 continuation 必须直接取得调用者隐藏返回位置，并额外定义：

- 多层装饰器如何共享任务初始化状态；
- 后置代码抛出时如何销毁已经构造的任务；
- 部分帧、参数和版本固定失败时由哪层清理；
- `Task::<void>` 和不同 `Task::<T>` 布局如何验证；
- 虚槽、接口槽与 `DynamicTaskOut` 如何转发最终存储；
- 热更新如何固定构造装饰器和最终 coroutine frame 的一致版本。

这些规则会形成第二套任务构造 continuation 系统。现有普通任务构造 thunk 只需在最终存储中成功建立一个任务或同步失败，v0 保持该模型。

## 3. 可变调用次数不解决任务构造所有权

议题 16 允许普通函数 decorator 零次、一次或多次进入调用 continuation，但这不能直接扩展到任务构造阶段。任务构造 continuation 操作的是调用者最终返回存储和不可复制的具体 `Task::<T>` 身份，而不是一次普通函数体进入。

任务构造装饰器通常会试图：

- 跳过构造并返回缓存任务；
- 复用或共享已有不可复制任务；
- 重试帧分配或多次构造候选任务；
- 替换任务类型或调度模型；
- 把一个任务同时交给多个独立所有者；
- 隐式建立引用计数任务句柄。

这些行为会直接改变任务身份、所有权、异常和析构协议。普通 decorator 的可变调用次数不提供缓存任务共享、隐藏返回位置重绑定或部分 coroutine frame 回滚能力，因此 v0 仍不增加任务构造 decorator。

## 4. 创建期逻辑使用同步任务工厂

需要在任务创建前后执行用户代码时，程序显式声明同步函数返回 `Task::<T>`：

```ink
async func load_impl(path: StringView) -> Data* {
    return await read_data(path);
}

func load_task(path: StringView) -> Task::<Data*> {
    log("creating load task");
    return load_impl(path);
}
```

同步工厂的任务结果按照议题 02、21、43 的最终返回位置规则直接构造，不要求复制或移动命名任务。调用者从函数种类即可看出创建表达式会执行用户同步代码：

```ink
var task = load_task(path); // 运行同步工厂并创建惰性任务
// load_impl 和其 async decorator 仍未运行
const data = await task;
```

真正的 `async func load_impl` 继续只建立任务；同步工厂负责额外创建期行为。语言不会把同步返回 `Task::<T>` 的工厂重新分类为异步函数。

## 5. 同步工厂可以使用已有装饰器

创建期逻辑需要复用时，给同步任务工厂应用普通同步装饰器：

```ink
@trace_task_creation
func load_task(path: StringView) -> Task::<Data*> {
    return load_impl(path);
}
```

普通 `decorator` 围绕同步工厂体运行。它可以记录调用、检查参数或使用 `try` 观察工厂抛出的同步创建异常。返回任务内部的 `async decorator` 仍等到该任务被驱动后运行。

两层语义显式可见：

```text
sync factory decorator
    → synchronous factory body
        → construct decorated async task
    → sync factory after code

later await
    → async decorator
        → async body
```

同步工厂装饰器不能捕获以后发生的任务失败；异步装饰器不能捕获任务成功建立前的工厂或帧构造失败。

## 6. 创建异常由工厂显式处理

同步任务工厂可以按普通异常规则观察或转换创建期失败：

```ink
func load_task(path: StringView) -> Task::<Data*> {
    try {
        return load_impl(path);
    } catch AllocationError as error {
        log_creation_failure(error);
        throw;
    }
}
```

它可以处理实参转换之前由工厂自己执行的逻辑，以及调用异步实现时的参数捕获、帧取得、模块固定和任务基础状态创建失败。任务成功返回后，异步函数体异常只存在于该任务的 `ExceptionBox`，不会再次展开到已经返回的同步工厂。

工厂本身标记 `[nothrow]` 时，完整同步工厂和同步装饰器链继续遵守议题 34；它不能让任务创建异常越过公开不抛出边界。

## 7. 不建立接口契约级构造拦截

没有函数体的接口要求不能添加任务构造装饰器：

```ink
interface Loader {
    @count_task_creation
    async func load() -> Data*; // v0 不存在这种契约级拦截
}
```

否则必须额外决定通过接口调用和直接类调用是否一致、子接口是否继承、类覆盖能否移除、多接口共同满足同一方法时的顺序，以及默认方法与类实现是否叠加。这不是函数体装饰，而是接口调用策略或中间件系统。

需要契约级拦截的库可以定义显式代理类、包装接口实现或同步任务工厂。核心语言不把它隐藏在接口声明装饰器中。

## 8. 虚函数、接口和反射 ABI 不增加包装层

不提供任务构造装饰器后，普通和反射异步调用继续使用：

```text
static, virtual, interface, or reflection dispatch
    → selected implementation task-construction thunk
    → construct one decorated coroutine frame
    → return one Task::<T>
```

vtable 和接口表不增加构造装饰器槽；议题 57 的 `DynamicTaskOut` 不需要多层转发初始化状态。最终实现的 `async decorator` 已经编译进其具体 coroutine frame，但在任务创建期间不执行。

同步任务工厂如果自身是普通虚函数或普通接口方法，只遵守同步虚/接口调用规则。它显式返回 `Task::<T>`，仍不覆盖或满足 `async func ... -> T`。

## 9. 热更新和稳定入口保持现有含义

异步函数稳定入口继续表示最终装饰后实现的任务构造入口。编译期强类型注册记录引用异步函数的 `function.entry` 时同样取得该入口；它不会先进入用户任务构造 decorator。

任务构造时固定最终装饰后 coroutine frame 的模块版本。同步工厂及其同步装饰器是另一项普通函数实现，可以独立遵守稳定入口和热更新事务；工厂调用哪个异步稳定入口由其自身版本代码决定。

核心异步热更新协议不需要增加构造装饰器活动计数、部分初始化 region 或额外卸载等待者。

## 10. 内部观测钩子不属于语言装饰器

编译器、运行时、调试器或性能分析器可以提供受控的内部事件：

```text
task frame allocated
task created
task first driven
task completed
task destroyed
```

这类事件可以由调试构建、profiling hook 或运行时遥测设施实现。它们不能在语言语义上替换任务、改变参数、吞掉构造失败或参与普通用户异常控制流，因此不构成 `task_constructor decorator`。

工具插桩必须保持程序在关闭插桩时的语言行为，并遵守模块卸载、线程安全和重入约束；精确工具 API 不由本议题决定。

## 11. 成本模型

不提供任务构造装饰器意味着普通异步任务构造路径不增加用户 continuation、额外状态字、包装任务或分派槽。只有程序显式调用同步任务工厂时才支付工厂和其同步装饰器代码成本。

编译器可以内联显式工厂或消除无可观察作用的包装，但不能提前执行返回任务的异步装饰器和函数体，也不能把工厂的同步副作用推迟到第一次 `await`。

## 12. 后续问题

以下内容仍可由未来独立机制讨论，但不属于 v0 普通函数装饰器：

- 拥有型或共享任务句柄及其缓存；
- 接口契约级中间件和调用拦截器；
- 任务构造阶段的缓存、替换或多次构造协议；
- 运行时任务观测和 profiling API；
- 后台调度器对任务创建与接管的库级钩子。
