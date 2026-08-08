# 议题 46：组合等待的失败取消策略

> 状态：已确认，议题 47—50 补充；底层取消感知 I/O ABI 待定  
> 确认日期：2026-08-02

## 1. 两种操作必须分离

一个并发任务失败后，“继续等待其他任务自然完成”和“请求其他任务取消”是两种可观察行为。Ink 不让同一个名称根据实现、运行时调度方式或优化级别隐式选择策略，而是提供两个语义明确的组合操作：

```ink
const (a, b) = await all(task_a(), task_b());

const (c, d) = await all_cancel_on_error(task_c(), task_d());
```

- `all`：不因某个输入失败而取消其他输入；
- `all_cancel_on_error`：观察到首个输入失败后，向其他尚未结束的输入请求取消。

两者在全部输入成功时具有相同的结果顺序、元组类型和复制规则。

## 2. `all` 等待全部自然结束

`all` 明确表示全部输入任务都应当完成。某个任务失败不会改变其他任务的执行意图：

```ink
const (log_result, save_result) =
    await all(write_log(), save_data());
```

如果 `write_log()` 失败，`save_data()` 仍然继续运行，直到成功或自行失败。`all` 等全部输入都到达最终状态后才向调用方传播失败。

编译器和运行时不得仅因为：

- 某个任务先失败；
- 调用方不可能再取得成功元组；
- 取消看起来可以降低延迟；
- 某个目标平台提供廉价取消；

就自动改变其他任务的执行。

## 3. 不把独立失败误当作依赖关系

并发任务可能具有相互独立的外部副作用。一个任务失败并不意味着其他任务的工作失去意义：

```ink
await all(flush_metrics(), persist_snapshot());
```

语言无法根据函数名、返回值或可见数据流判断两项工作是否构成事务。`all` 不替程序员推断这种业务依赖，也不声称能够回滚已经发生的 I/O、设备操作或数据库提交。

如果两个操作存在“任一失败就停止其余工作”的关系，调用方必须在源码中选择 `all_cancel_on_error`，或者使用更高层事务协议。

## 4. `all_cancel_on_error` 只请求取消

`all_cancel_on_error` 在观察到第一个失败终态后，对其他尚未结束的输入发出取消请求：

```ink
const (left, right) = await all_cancel_on_error(load_left(), load_right());
```

议题 48 确定取消只是协作式请求，不是从外部线程强制销毁任务，也不是立即释放协程帧。Ink 不增加 `cancelled` 任务终态或内建取消异常。一个任务可以：

- 查询请求后返回普通结果；
- 查询请求后抛出 API 自己定义的普通异常；
- 先执行必要清理再结束；
- 因处于不可取消操作而延迟响应；
- 明确忽略请求并正常完成。

因此该名称表达“失败时请求取消”，不承诺其他任务在首个失败出现时已经停止。

## 5. 两种操作都必须完成收尾

无论使用哪一种组合操作，异常都不能在仍有输入任务使用组合存储时离开等待表达式：

```text
all:
    first failure
        → do not request sibling cancellation
        → wait until every input reaches a final state
        → propagate failure

all_cancel_on_error:
    first failure
        → request cancellation of unfinished siblings
        → wait until every input reaches a final state
        → propagate failure
```

如果某个任务没有响应取消请求，`all_cancel_on_error` 继续等待它自然结束。取消不能成为让任务越过组合边界继续运行的理由。

这条规则保证议题 45 中直接构造的临时任务、结果槽、异常状态和组合 continuation 在离开表达式前不再被输入任务访问。

## 6. 已完成任务不受取消请求影响

首个失败被观察到时，其他输入可能已经成功或自行失败：

- 已经成功的任务保持成功终态；
- 已经失败的任务保持自己的 `ExceptionBox`；
- 只有尚未结束的任务接收取消请求；
- 取消请求不得把已发布的最终状态改写成其他状态。

命名任务在组合结束后仍遵守议题 43 的重复观察规则。组合操作不消费或重置它们的终态。

## 7. 失败传播与取消请求

本议题只决定何时请求取消以及异常离开前必须等待全部输入结束。议题 47 进一步确定：全部输入结束后，`all` 与 `all_cancel_on_error` 都按源码参数顺序传播第一个失败任务的原始异常，不按完成时间选择，也不构造默认聚合异常。

议题 48 不提供特殊取消终态：任务响应请求后正常返回就是成功，主动抛出业务异常就是真正失败。一个任务先失败、另一个在响应请求时也抛出异常时，两个任务都处于普通 `failed`，由议题 47 按参数顺序选择。语言不猜测异常是否“只是取消产生的”。

等待 `all_cancel_on_error` 的当前任务同时收到外部取消请求时如何显式响应，仍留给跨任务取消传播议题。

## 8. 成本模型

普通 `all` 只需要议题 45 的组合完成计数、结果槽和失败记录，不需要仅为了兄弟任务失败而建立组取消传播。

`all_cancel_on_error` 额外可能需要：

- 一个组合取消状态；
- 向每个输入传递或关联取消观察入口；
- 首个失败发布时的一次原子取消请求；
- 唤醒或通知等待可取消操作的任务；
- 等待取消后的清理完成。

这些成本只由显式选择 `all_cancel_on_error` 的代码承担。实现可以消除能够证明无用的取消状态，但不得把普通 `all` 优化成失败取消，也不得把取消版本优化成自然等待版本。

## 9. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。`all` 使用普通组合完成状态机；`all_cancel_on_error` 在相同状态机上使用议题 49 的请求协议：

```text
child publishes failed
    → atomic publish group cancellation request once
    → notify unfinished children
    → continue decrementing remaining on every final completion
    → resume parent only when remaining == 0
```

取消检查、I/O 唤醒和任务最终状态由 Ink 运行时及库级调度设施实现。LLVM 只需要普通协程、原子操作、控制流和异常处理能力，不需要理解组合取消语义。

## 10. 后续问题

以下内容留给后续议题：

- 底层取消感知 I/O 的登记和唤醒 ABI；
- 取消通知、清理协议和超时；
- 当前任务取消向组合输入传播的规则；
- `all_cancel_on_error` 的最终标准库位置和精确泛型签名；
- 调度失败是否触发同一取消策略。
