# 议题 48：协作式取消请求

> 状态：已确认，议题 49、50、52 与 Parser 议题 25 补充；底层取消感知 I/O ABI 待定
> 确认日期：2026-08-02

## 1. 取消是请求，不是任务终态

Ink 把取消定义为对正在执行任务的一项协作式请求。请求本身不结束任务，也不决定任务最终结果。

议题 43、44 的状态集合保持不变：

```text
TaskState::<T> =
    created
    pending
    succeeded(ResultStorage::<T>)
```

第一版不增加 `cancelled` 终态。一个收到取消请求的任务最终仍然只能：

- 通过普通返回值进入 `succeeded`；
- 暂时忽略请求并继续保持 `pending`。

## 2. 请求不触发隐式控制流

设置取消请求不会自动：

- 穿过调用栈进行非局部跳转；
- 运行 `defer` 或析构函数；
- 销毁协程帧；
- 把任务发布为完成；
- 强制中止线程或 I/O。

取消请求只是任务控制状态中的信息。只有任务自己的普通控制流可以决定何时结束以及返回什么值。

## 3. 任务显式查询取消请求

任务可以在适合中止工作的边界显式查询当前执行上下文是否收到取消请求。概念接口为：

```ink
cancel_requested() -> bool
```

示例：

```ink
async func process() -> ProcessResult {
    while (has_more_work()) {
        if (cancel_requested()) {
            cleanup_partial_work();
            return ProcessResult.cancelled;
        }

        process_next();
    }

    return ProcessResult.completed;
}
```

`ProcessResult.cancelled` 是该 API 自己定义的普通结果值。任务从语言运行时角度成功完成，不存在特殊取消终态。

查询函数的最终命名和所属核心模块可以在标准库命名阶段调整；其语义必须是只观察当前请求状态，不隐式清除请求，也不执行用户代码。

## 4. 任务自行选择响应方式

收到请求的任务可以根据自身不变量选择行为。

### 4.1 返回普通结果

任务可以返回完整结果、部分结果或显式包含取消分支的业务结果：

```ink
if (cancel_requested()) {
    return DownloadResult.partial(bytes_received);
}
```

返回仍遵守普通类型和复制规则，不由语言生成默认 `T`。

### 4.2 延迟或忽略请求

处于不可取消提交阶段、设备临界操作或必须完成清理协议的任务可以继续执行：

```ink
async func commit_transaction() {
    begin_commit();
    finish_commit(); // 该阶段明确不响应取消请求
}
```

取消请求不是强制抢占，调用方不能依赖任务一定提前结束。

## 5. 普通控制流负责清理

取消请求本身不触发清理。任务决定通过普通 `return` 结束时，既有语言规则自然运行清理：

```ink
async func work() -> WorkResult {
    var file = File.open("temp.dat");
    defer remove_temporary_file();

    if (cancel_requested()) {
        return WorkResult.cancelled;
    }

    return WorkResult.completed;
}
```

这里 `defer` 和局部对象析构是因为函数执行普通 `return`，不是因为取消请求具有隐式展开语义。

如果任务忽略请求且继续运行，就不会因为请求标志自动执行任何清理。

## 6. 不自动跨任务传播

取消请求属于被请求任务的执行上下文。普通函数调用共享当前任务，因此可以查询同一请求状态；创建另一个惰性 `Task` 会建立另一个任务状态。

第一版不因为一个任务正在 `await` 另一个任务，就自动把前者的取消请求传播给后者。议题 50 提供显式的 `await cancel_on_request(child)` 组合等待；库定义的任务关系或未来取消令牌也必须明确表达传播。

这避免普通 `await` 暗中改变被等待任务的执行意图，也避免同一个可重复等待任务因某个等待者取消而影响其他等待者。

## 7. 取消感知异步 I/O

任务可能挂起在 I/O、计时器、锁或库级调度队列中，无法主动轮询。支持取消的异步操作可以在挂起期间登记取消通知，使请求到达时唤醒任务。

被唤醒后，该操作仍按自己的 API 合同决定：

- 返回普通取消或部分结果；
- 完成已经无法撤回的操作并返回结果；
- 继续等待。

语言不要求所有异步操作都支持取消，也不把不支持取消的平台行为伪装成已经取消。取消通知注册、竞态消解和 I/O 后端接口留给运行时议题。

## 8. 并发与状态发布

取消请求可能由另一个线程或任务发出。请求状态必须满足：

- 一旦 `cancel_requested()` 观察到 `true`，以后不会重新变回 `false`；
- 并发重复请求是幂等的；
- 请求不能改写已经发布的 `succeeded` 终态；
- 请求与任务完成并发发生时，最终任务状态由任务实际完成路径决定；
- 取消通知不能在任务和等待对象已经销毁后继续访问它们。

实现通常可以把请求位放入已有任务控制状态的原子字中，不要求单独分配取消对象。

## 9. 成本模型

不使用取消的普通同步代码没有成本。普通异步任务可能需要在已有控制状态中保留一个请求位；如果目标 ABI 无法复用现有状态字，精确布局成本留给任务 ABI。

只有显式调用 `cancel_requested()` 的代码执行请求状态读取。Ink 不在每个函数入口、Parser 议题 25 定义的循环迭代、普通 `await` 或普通 I/O 后自动插入取消检查。

取消感知等待可能额外承担通知登记、原子竞态处理和唤醒成本；不支持取消的操作不需要建立这些状态。

## 10. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。取消请求可以降低为普通原子位操作和可选通知回调：

```text
request_cancel(task):
    atomic_or task.control_flags, CANCEL_REQUESTED
    notify registered cancellable wait if present

cancel_requested():
    return atomic_load current_task.control_flags & CANCEL_REQUESTED
```

任务响应请求时生成的 `return` 使用已有普通控制流和协程 ABI。LLVM 不需要理解取消请求。

## 11. 后续问题

以下内容留给后续议题：

- 取消通知登记与解除的竞态协议；
- 取消感知 I/O、计时器和锁的标准库接口；
- 显式取消令牌及批量跨任务传播；
- 超时如何转换为取消请求；
- 请求状态在热更新和调试器中的表示；
- freestanding 目标是否只提供轮询查询。

议题 49 已确定普通 `Task::<T>.request_cancel()` 的公开请求语义，以及对 `created`、`pending` 和最终状态任务的行为。

后台调度设施由谁持有请求控制对象、关闭时如何请求任务停止，均由议题 52 划归具体库契约。

