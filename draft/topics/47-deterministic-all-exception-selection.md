# 议题 47：组合等待的确定性异常选择

> 状态：已确认，议题 48、53 补充；`all_settled` 结果 API 待定  
> 确认日期：2026-08-02

## 1. 按参数顺序选择失败

当 `all(...)` 的多个输入任务失败时，Ink 等全部输入到达最终状态，然后按源码参数顺序从左到右检查任务，传播第一个失败任务保存的异常：

```ink
await all(task_a(), task_b(), task_c());
```

如果 `task_a()` 和 `task_c()` 都失败，`all` 传播 `task_a()` 的异常。哪个任务在运行时先完成、先发布失败或先唤醒组合 continuation，不改变选择结果。

## 2. 不按完成时间选择

并发任务的完成先后可能受以下因素影响：

- OS 线程调度；
- 运行时或库级调度队列状态；
- I/O 完成顺序；
- CPU 数量和负载；
- 目标平台与优化结果。

如果传播“最先完成的异常”，相同输入可能在不同运行中进入不同 `catch` 分支。Ink 不让异常动态类型和控制流结果依赖这种非确定调度顺序。

参数位置是源码中稳定、无需额外 ID 的顺序，因此作为默认选择依据。

## 3. 等全部结束后再扫描

议题 45、46 已经要求组合等待在传播异常前确保全部输入进入最终状态。组合器在这一时刻执行确定性扫描：

```text
wait until every input is final
    → for input in source argument order
        → if input is failed: select it and stop scanning
    → if no input failed: assemble success tuple
```

该扫描不需要延迟尚未结束任务的完成发布，也不要求运行时为了参数顺序串行执行任务。任务仍然并发前进；只有最终异常选择按参数位置确定。

## 4. 传播原始任务异常

被选中的失败任务已经按照议题 43 保存一个只读 `ExceptionBox`。`all` 不复制异常载荷，不创建同类型替代对象，也不改写原始 `throw_site`。

在 `await all(...)` 处传播时，运行时为当前等待者建立独立的活动展开记录：

```text
selected Task.ExceptionBox
    → ActiveUnwindRecord at await all
    → ordinary Ink catch matching and unwinding
```

异常动态类型、接口匹配、原因链和生产者 traceback 与直接等待该失败任务时相同。组合等待位置只作为本次异步传播位置记录，不伪装成新的原始抛出点。

## 5. 不默认建立聚合异常

普通 `all` 和 `all_cancel_on_error` 不因为多个输入失败而构造聚合异常类型，也不把其他失败自动附加为：

- 隐式 `from` 原因链；
- suppressed exception；
- 运行时自动日志；
- 隐藏的用户可见异常数组。

这与议题 40 的显式原因链一致，也避免默认成功或单失败路径承担聚合容器、额外引用计数和新的捕获规则。

如果以后提供显式聚合错误 API，它必须使用不同的组合操作或结果类型，不能改变既有 `all` 的异常动态类型。

## 6. 组合器检查其他失败但不写回任务

没有被选中传播的失败仍然参与组合等待，且 `all` 必须检查它们的最终状态才能完成选择。议题 53 规定核心任务没有 observation bit，因此该检查不写回命名任务，也不存在需要清除的“未观察失败”诊断状态。

组合器完成清理时：

- 临时任务未被选中的 `ExceptionBox` 按普通内部所有权规则释放；
- 命名任务继续保存各自不可变的失败终态；
- 组合器不修改、清除或重置任何命名任务的状态。

对命名任务，调用方以后仍可分别 `await` 并观察其原始异常：

```ink
var first_task = first_async();
var second_task = second_async();

try {
    await all(first_task, second_task);
} catch as error {
    // 这里传播参数顺序中的第一个失败
}

// 如果需要，可以再分别处理命名任务的终态。
```

## 7. `all_cancel_on_error` 使用同一选择顺序

议题 46 的 `all_cancel_on_error(...)` 可以在运行期间由某个较早完成的失败触发取消请求，但最终向调用方传播的异常仍按参数顺序选择，而不是按“谁触发了取消”选择。

这把两个概念分开：

- 首个被组合器检测到的运行时失败：用于尽早发出协作式取消请求；
- 全部输入结束后的源码顺序失败：用于确定最终异常传播。

议题 48 确定取消请求没有特殊任务终态。任务响应请求后正常返回就是 `succeeded`；任务选择抛出的业务异常就是普通 `failed`，与其他失败一样参与参数顺序选择。语言不根据异常是否发生在取消请求之后改变其优先级。

## 8. 显式收集全部状态

需要取得每个任务的成功或失败状态时，不应依赖普通 `all` 抛出一个聚合异常。未来可以提供概念上的显式操作：

```ink
const first_state, second_state =
    await all_settled(first_async(), second_async());
```

`all_settled` 应等待全部任务并把每项终态作为普通结果返回，而不是从任务失败中自动抛出。其状态类型、异常拥有型视图、不可复制结果规则和动态异构元组接口尚待单独设计。

本议题只保留该扩展方向，不把未设计的 `all_settled` API 视为已经可用的语言功能。

## 9. 成本模型

组合器已经需要保存每个输入的任务状态或状态地址。确定性选择额外只需要在全部完成后执行一次按参数顺序的线性扫描：

```text
time:  O(number of inputs)
space: no aggregate exception required
```

被选中异常仍使用议题 43 的共享 `ExceptionBox` 和当前传播专用展开记录。没有被选中的临时失败正常释放内部持有，不需要复制异常载荷。

编译器可以展开固定参数数量的扫描，但不得根据完成顺序提前决定最终传播对象。

## 10. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。前端或 Ink 运行时在组合未完成计数归零后生成普通顺序检查：

```text
if task_0.failed: propagate(task_0.exception_box)
else if task_1.failed: propagate(task_1.exception_box)
...
else: construct result tuple
```

传播被选中的 box 使用议题 43 已确定的异常载体、personality 和平台展开 ABI。LLVM 不需要理解参数顺序的语言含义或聚合异常。

## 11. 后续问题

以下内容留给后续议题：

- 当前等待任务收到外部取消请求时如何显式响应；
- `all_settled` 的终态类型和异常观察 API；
- 动态任务集合是否保持容器索引顺序；
- 调度提交失败在参数顺序中的位置；
- 调试器如何展示未被选中但已由组合器检查的失败。
