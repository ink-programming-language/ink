# 议题 53：核心 `Task` 不记录失败观察状态

> 状态：已确认  
> 确认日期：2026-08-02

## 1. `failed` 只有一个终态

议题 43 的失败终态保持为：

```text
failed(ExceptionBox)
```

核心 `Task<T>` 不把它细分为 `unobserved_failed`、`observed_failed` 等状态，也不保存 `observed` 标志、观察次数或“第一个观察者”。

任务是否失败是任务自身的不可变结果；某个等待者是否读取、捕获或继续传播该失败，是该等待者的一次控制流事件，不写回任务。

## 2. `await` 不标记任务

对失败任务执行 `await` 时，继续遵守议题 43：

```text
await failed task
    → retain ExceptionBox for this propagation
    → create an independent active unwind record
    → propagate from this await site
```

该过程不设置 observation bit。重复等待和多个并发等待者之间没有“第一次观察”带来的特殊行为：

```ink
try {
    await task;
} catch as first_error {
    // 只处理本次传播
}

try {
    await task;
} catch as second_error {
    // 同一失败可以再次独立传播
}
```

一个等待者捕获失败，不会改变其他等待者以后得到的结果。

## 3. 普通核心流程不需要未观察检查

核心异步流程已经覆盖失败传播：

- 从未被驱动的 `created` 任务没有执行函数体，因此不可能产生函数体异常；
- 直接 `await` 在等待点传播失败；
- 议题 45—47 的 `all` 和 `all_cancel_on_error` 检查全部输入终态，再按既定规则传播一个失败；
- 被组合器检查但未被选中传播的失败直接按普通所有权规则清理，不产生第二次传播。

因此核心语言无需为了发现“忘记观察”而给每个任务添加运行时状态。

## 4. 销毁 `failed` 任务只清理资源

议题 51 的失败析构分支最终确定为：

```text
destroy failed Task<T>:
    release task's hold on ExceptionBox
    destroy remaining frame state
    unpin compatible module version
```

析构不会：

- 打印异常或 traceback；
- 调用全局未观察失败处理器；
- 重新抛出 `ExceptionBox` 中的异常；
- 启动新的异常展开；
- 触发 warning、trap 或进程级 fail-fast。

最后一个内部持有释放后，异常载荷和原因链按议题 36、40、43 的普通规则销毁。

## 5. 析构期间不能报告失败

`Task<T>` 可能因为普通作用域退出，也可能在另一个异常已经展开时析构。若失败任务析构尝试抛出、调用任意用户处理器或执行复杂日志，会引入：

- 析构期间的第二个异常；
- 隐藏 I/O、分配和锁；
- 不确定的程序终止点；
- 正常代码与调试构建之间的行为差异；
- 多等待者之间没有唯一答案的“是否已经观察”竞态。

因此失败任务析构保持不可失败、有限且只负责资源释放。

## 6. 与进程级 fail-fast 的边界

议题 42 的 fail-fast 只在异常实际越过 `main`、普通 OS 线程入口、`[nothrow]` 边界或其他致命执行边界时发生。

异步函数异常到达任务边界后已经被捕获进 `ExceptionBox`，不会仅因任务发布 `failed` 或稍后析构而重新成为“未捕获异常”。只有某次 `await` 把它重新传播，并最终越过致命边界时，才进入进程级 fail-fast。

## 7. 组合等待没有全局观察副作用

议题 47 所说“组合器观察全部失败”只表示组合器为了完成自己的异常选择而检查了全部输入终态。它不要求在每个输入 `Task` 中写入观察标志。

未被选中的失败：

- 不加入隐式原因链或 suppressed 列表；
- 不再次传播；
- 不产生未观察失败诊断；
- 临时任务中的 box 随组合存储正常释放；
- 命名任务仍保留原始 `failed(ExceptionBox)`，以后仍可重复 `await`。

## 8. 后台任务策略属于库

议题 52 允许调度库自己定义后台任务、句柄和关闭规则。库若希望报告无人查询的失败，可以在自己的 job、future 或控制块中显式保存观察状态，并选择：

- 记录日志；
- 调用库定义的处理器；
- 在库的 join 或 shutdown API 中返回失败；
- 采用库明确说明的 fail-fast 策略。

这些字段、原子操作、引用计数和报告成本属于该库对象，不写入核心 `Task<T>`，也不改变普通 `Task` 析构语义。

## 9. 编译器诊断

编译器仍可以对明显创建后立即丢弃、从未驱动的惰性任务给出 warning。这用于发现“异步函数体根本没有执行”，不是未观察异常诊断。

核心语言不要求编译器或运行时对 `failed` 任务维护以下诊断：

- “任务失败但从未 await”；
- “异常只被部分等待者观察”；
- “组合器没有选择该异常”；
- “失败任务析构时仍未标记 observed”。

调试器可以在对象仍存活时显示其 `failed` 状态和 `ExceptionBox`，但不能把缺少 observation bit 解释成程序错误。

## 10. 成本模型

本设计不会为核心 `Task<T>` 增加：

- observation bit 或观察计数；
- 每次成功或失败 `await` 的原子写入；
- 析构时的额外状态检查；
- 全局未观察失败注册表；
- 隐式日志、分配或回调。

失败 `await` 仍只承担既有的 box 持有、活动展开记录和异常传播成本。失败析构只承担本来就需要的资源释放。

## 11. LLVM 与运行时实现

该决定不需要修改 LLVM 源码。任务最终状态仍只保存 `failed(ExceptionBox)`；`await` 的 lowering 读取 box 并建立本次展开记录，不生成写回任务的 observation 操作。

析构 lowering 直接进入议题 51 的失败清理分支。LLVM 无需理解“是否观察过异常”，Ink 运行时也不需要为核心任务实现对应状态机。

## 12. 后续问题

仍待确定：

- `ExceptionBox` 是否提供用户可见的只读查询 API；
- 调试器展示任务失败和异步传播链的格式；
- 编译器未使用惰性任务 warning 的精确范围和抑制方式。

库级后台任务的未观察失败策略属于议题 52 的库设计，不再作为核心语言待定项。
