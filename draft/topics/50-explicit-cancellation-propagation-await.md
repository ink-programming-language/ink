# 议题 50：显式取消请求传播等待

> 状态：已确认，议题 51、52 补充；底层取消感知 I/O ABI 和组合器中间对象 ABI 待定  
> 确认日期：2026-08-02

## 1. 普通 `await` 不传播取消请求

直接等待另一个任务只观察其结果，不建立取消请求传播关系：

```ink
let result = await child;
```

等待期间当前任务收到议题 49 的请求时：

- 当前任务的请求位变为已设置；
- `child` 不因此收到请求；
- 当前任务继续等待 `child` 的实际终态；
- 编译器不自动调用 `child.request_cancel()`。

这保证同一个可重复等待任务不会仅因为某一个等待者收到请求而隐式影响其他等待者。

## 2. 使用 `cancel_on_request` 显式传播

需要把当前任务的取消请求转发给被等待任务时，使用核心组合操作：

```ink
let result = await cancel_on_request(child);
```

其含义是：在本次等待持续期间，如果当前任务已经收到或以后收到取消请求，则向 `child` 发出一次幂等的 `request_cancel()`。

名称中的 `on_request` 表明传播由当前任务收到请求触发；该操作本身不会无条件取消 `child`。

## 3. 已存在请求必须立即传播

建立传播等待时，当前任务可能已经收到请求：

```ink
if cancel_requested() {
    // 请求可能早于下一行发生
}

let result = await cancel_on_request(child);
```

`cancel_on_request` 必须在登记传播关系时处理该情况。如果当前请求位已经设置，立即向 `child` 发布请求，然后继续等待其实际结束。

实现不能只登记未来通知而漏掉登记前已经发生的请求。

## 4. 登记与请求竞态必须无丢失

“检查当前请求位”和“登记传播关系”之间可能并发发生新请求。实现必须使用原子握手或等价调度器串行化保证：

```text
current request before registration
    → registration observes requested and requests child

current request during registration
    → either requester or registrar requests child

current request after registration
    → registered notification requests child
```

允许同一竞态路径内部重复尝试请求，因为 `Task.request_cancel()` 已按议题 49 定义为幂等；不允许完全丢失请求。

## 5. 等待结束时解除传播关系

当 `child` 到达 `succeeded` 或 `failed`，当前等待恢复前必须解除传播登记。解除以后到达的当前任务请求不能再访问已经结束的登记节点或已销毁的临时任务。

完成与请求并发时，运行时必须保证以下结果之一：

- 请求先取得有效登记并安全调用 `child.request_cancel()`；
- 完成先解除登记，请求不再访问 `child`；
- 双方通过等价引用、epoch 或调度器串行化完成安全交接。

不能发生 use-after-free、重复释放或把请求转发给复用同一地址的新任务。

## 6. 传播请求不结束等待

向 `child` 请求取消后，当前任务仍然等待 `child` 到达真实终态：

```text
current task receives request
    → child.request_cancel()
    → child may return, throw, delay, or ignore
    → current task remains suspended until child is final
```

`cancel_on_request` 不直接销毁子任务、不伪造返回值，也不产生特殊取消异常。子任务正常返回时，本次等待得到普通结果；子任务抛出业务异常时，按照议题 43 从等待点传播该异常。

议题 51 进一步要求：传播等待必须保持目标任务存活，直到目标到达最终状态并解除登记；不能在发出请求后析构仍为 `pending` 的目标任务。

## 7. 对共享任务的影响是显式的

同一个命名任务可以被多个等待者通过 `Task<T>*` 观察：

```ink
var child = operation();

// waiter A
await cancel_on_request(child);

// waiter B
await child;
```

如果 waiter A 收到请求，它会明确向共享的 `child` 发出请求，因此 waiter B 也可能观察到由 `child` 响应请求所造成的普通结果或业务异常。

这不是普通 `await` 的隐藏副作用，而是 waiter A 显式选择 `cancel_on_request` 的结果。需要彼此独立的执行意图时，应创建不同任务，不能让多个等待者共享同一可取消执行实例。

## 8. 传播只跨越当前显式边

`cancel_on_request(child)` 只把当前任务的请求传播到 `child`。如果 `child` 又直接等待 `grandchild`：

```ink
async func child() {
    await grandchild;
}
```

请求不会自动继续传播到 `grandchild`。`child` 必须自行查询 `cancel_requested()`，或者显式写：

```ink
await cancel_on_request(grandchild);
```

因此取消传播图由源码中明确的边组成，不由运行时根据调用栈或任务创建关系推断。

## 9. 临时任务的生命周期

异步调用可以直接作为传播等待的参数：

```ink
let result = await cancel_on_request(socket.read(buffer));
```

临时 `Task<T>` 必须直接构造在本次组合等待的稳定存储中，并至少存活到：

- 子任务到达最终状态；
- 传播登记已经解除；
- 当前等待不再可能向其发送请求。

实现不能通过复制或隐藏移动保存 `[noncopyable] Task<T>`。`cancel_on_request(...)` 中间对象能否脱离直接 `await` 保存，留给组合器 ABI。若组合器内部保存普通引用或指针，它不延长任务对象生命周期，调用者必须保证目标在组合操作结束前有效。

## 10. 运行时通知不能执行任意用户代码

取消请求可能来自任意线程。传播登记被触发时只能执行有界、线程安全且不可失败的运行时动作，例如：

- 设置目标任务请求位；
- 把调度器唤醒事件入队；
- 通知已登记的操作系统异步请求；
- 发布后续由任务或调度器处理的工作。

它不能在请求者线程上同步执行目标任务主体、普通用户回调、可能阻塞的清理或可抛异常代码。用户对请求的响应仍发生在目标任务自己的正常执行路径中。

## 11. 与取消感知 I/O 的关系

如果 `socket.read()`、计时器或异步锁已经挂起，仅设置任务请求位不足以恢复任务。支持取消的底层操作可以在任务挂起期间登记运行时通知；目标任务收到 `request_cancel()` 时，运行时尝试撤销或唤醒该操作。

平台或具体操作不支持撤销时，请求位仍然保持，但操作可以继续等待自然完成。Ink 不伪造取消成功，也不保证 `cancel_on_request(...)` 降低等待延迟。

被唤醒的 I/O API 仍按议题 48 自行选择返回普通状态、部分结果、业务异常或继续操作；语言不加入统一取消结果。

## 12. 与组合等待的关系

`all_cancel_on_error(...)` 在输入失败后直接向兄弟任务发布请求，不需要先把组合器自身伪装成收到请求。它与 `cancel_on_request(...)` 共用议题 49 的目标任务请求协议，但触发条件不同：

- `all_cancel_on_error`：由某个输入失败触发；
- `cancel_on_request`：由当前等待任务收到请求触发。

普通 `all(...)` 和普通 `await` 都不会自动建立当前任务到输入任务的请求传播。

## 13. 成本模型

普通 `await task` 不为取消传播登记通知节点，也不执行额外请求位检查。

显式 `cancel_on_request(task)` 可能承担：

- 当前任务请求位的一次原子读取；
- 一个临时传播登记节点；
- 登记和解除时的同步；
- 请求与完成竞态中的原子操作；
- 请求实际发生时的一次目标 `request_cancel()`。

固定、非逃逸的登记节点可以内联到当前协程帧，不必单独分配。精确实现不得把这些成本施加给没有写出传播组合器的普通等待。

## 14. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。前端可以把组合等待降低为普通运行时登记和 coroutine suspension：

```text
prepare child task
register current-request → child-request link
recheck current request without lost wakeup
start or await child
suspend current task

on child completion:
    unregister link
    resume current task
```

登记节点同步可以使用普通原子操作、锁、epoch 或单线程调度器串行化。底层 OS 取消 API 由 Ink 运行时和标准库调用；LLVM 不需要理解请求传播语义。

## 15. 后续问题

以下内容留给后续议题：

- `cancel_on_request` 的精确核心模块和泛型签名；
- 组合器中间对象能否保存或必须立即 `await`；
- Windows IOCP、Linux io_uring/epoll 等后端的取消 ABI；
- 计时器、异步锁和通道的请求通知规则；
- 一次请求控制多个任务的显式 `CancelToken`；
- 超时组合器与请求传播的关系；
- freestanding 目标只支持轮询时的行为。

库级调度控制对象是否建立同类传播，由议题 52 规定为该库自己的 API 契约。
