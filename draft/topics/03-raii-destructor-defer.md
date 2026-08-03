# 议题 03：析构、RAII 与 `defer`

> 状态：已确认，析构函数名称经议题 06 修订，动态销毁经议题 27 补充，枚举载荷经议题 32 补充，异常展开经议题 34、42 修订，议题 48、51、53、54 补充异步任务，Parser 议题 09 确认独立 block 作用域  
> 确认日期：2026-08-01

## 1. 确定性析构

Ink 对具有析构函数的局部对象采用 RAII 和确定性析构。

对象成功完成初始化后，编译器为它注册清理动作。以下结构化控制流离开对象所在作用域时，必须执行清理：

- 正常到达作用域末尾；
- `return`；
- `break`；
- `continue`；
- 议题 34 定义的异常展开。

以下非结构化终止不保证执行清理：

- trap；
- 进程强制终止；
- 无法恢复的硬件故障；
- 外部代码直接终止进程。

议题 42 的未捕获异常进程级 fail-fast 也属于不保证继续清理的终止路径。一旦运行时确定异常已经到达致命执行边界，程序不能依赖尚未执行的局部析构、`defer`、线程局部析构或全局析构完成。

## 2. 析构函数的声明

析构函数使用固定的内建名称 `destructor`，并保留 Ink 的 `func` 声明和显式接收者语法：

```ink
[noncopyable]
class File {
    handle: OsHandle;

    func destructor(this: File&) {
        if this.handle.is_valid() {
            os.close(this.handle);
        }
    }
}
```

析构函数必须满足以下规则：

- 名称固定为 `destructor`；
- 接收者固定为 `this: Type&`；
- 不接受其他参数；
- 不返回值；
- 隐式满足 `[nothrow]`，函数体和所调用清理不能让异常逃逸；
- 不能重载；
- 每个类型最多声明一个析构函数；
- 声明析构函数的类型必须显式标记 `[noncopyable]`。

析构函数可以读取和修改 `this`，但不得延长即将结束的对象生命周期。

议题 54 的原始 `this` 捕获只适用于异步成员任务。同步 `destructor(this: Type&)` 仍使用本议题的短期接收者，不把引用保存到未来执行，也不因异步方法规则改变语法或 ABI。

## 3. 析构顺序

作用域中的局部对象按照成功初始化顺序的逆序析构。

对于具有用户析构函数的聚合体：

1. 先执行用户定义的 `destructor`；
2. 再按照字段声明顺序的逆序自动析构字段。

```ink
[noncopyable]
class Server {
    listener: Socket;
    log: File;

    func destructor(this: Server&) {
        metrics.server_stopped();
    }
}
```

`Server` 的销毁顺序是：

```text
destructor(Server)
destructor(File log)
destructor(Socket listener)
```

用户析构函数不应直接析构字段；字段清理由编译器自动完成。

销毁带载荷枚举时，编译器读取当前有效判别分支，并且只析构该活动分支中已经构造完成的载荷。非活动分支没有对象生命周期，不能执行其析构。具体枚举规则由议题 32 规定。

派生类完整对象的销毁从最派生类开始，依次执行当前类的用户 `destructor`、当前类字段的逆序清理，然后进入直接父类，直到具体父类链结束。通过基类进行动态销毁时也必须保持相同顺序，具体入口由议题 27 规定。

## 4. 部分初始化

如果对象构造过程中某个字段初始化失败，只析构已经成功初始化的字段，并按照成功初始化顺序的逆序执行。

```ink
var server = Server {
    listener: Socket.listen(8080),
    log: File.open("server.log"),
};
```

如果 `listener` 初始化成功而 `log` 初始化失败，编译器只析构 `listener`。由于完整的 `Server` 从未构造成功，不调用 `Server.destructor`。

## 5. 不允许直接调用析构函数

析构函数只能由语言的生命周期机制调用，程序不能直接写：

```ink
file.destructor(); // 编译错误
```

允许直接调用析构函数会产生“对象已销毁但绑定仍然存在”的状态，与 Ink 不提供已移动变量的设计相冲突。

议题 27 的编译器生成动态销毁入口属于语言生命周期机制，不是用户可直接调用的普通方法。

需要提前释放资源时，应使用以下方式之一：

- 进入更小的词法作用域；
- 调用普通的 `close()`、`reset()` 等方法，使对象进入类型定义的合法状态。

```ink
[noncopyable]
class File {
    handle: OsHandle;

    func close(this: File&) {
        if this.handle.is_valid() {
            os.close_checked(this.handle);
            this.handle = OsHandle.invalid;
        }
    }

    func destructor(this: File&) {
        if this.handle.is_valid() {
            os.close(this.handle);
        }
    }
}
```

`close()` 可以报告错误，并在成功后把对象变成合法的关闭状态。随后执行 `File.destructor` 时不会重复关闭资源。

也可以通过嵌套作用域精确结束生命周期：

```ink
{
    var file = File.open("data.txt");
    process(file);
} // 自动执行 File.destructor

continue_work();
```

Parser 议题 09 正式确认这种独立 `{ ... }` 是合法的 `block statement`，它建立普通词法作用域且不产生表达式值。

## 6. `defer`

执行到 `defer` 语句时，编译器在当前作用域注册一个清理动作。局部对象析构和 `defer` 使用同一个后进先出的清理栈。

```ink
{
    var first = Resource.create(); // 注册 destructor(first)
    defer log("A");                // 注册 defer A

    var second = Resource.create();// 注册 destructor(second)
    defer log("B");               // 注册 defer B
}
```

离开作用域时的执行顺序是：

```text
defer B
destructor(second)
defer A
destructor(first)
```

只执行控制流实际到达并成功注册的 `defer`。循环每次迭代建立的作用域拥有独立的清理记录。

与析构函数相同，`defer` 清理隐式满足 `[nothrow]`，不得让异常逃逸。需要可靠处理的失败必须在正常控制流中提前完成，或者在清理体内捕获全部异常。

## 7. 清理期间的 trap

如果析构函数或 `defer` 动作触发 trap，或者违反 `[nothrow]` 让异常逃逸，程序立即终止，不再执行尚未完成的清理动作。

因此：

- 可恢复失败不能依赖析构函数报告；
- 需要确认成功的资源释放应使用显式 `close()`；
- 析构函数只负责不可失败的释放，或者在失败时接受立即终止。

## 8. 取消请求不触发隐式清理

议题 48 的协作式取消只设置请求状态，不是 `return`、异常展开、trap 或作用域退出，因此请求本身不执行析构函数或 `defer`。

任务观察请求后选择普通 `return` 或抛出业务异常时，才按照本议题的既有规则离开作用域并运行清理。任务忽略请求继续执行时，局部对象和 `defer` 保持正常活动状态。

## 9. `Task` 析构不能等待

议题 51 规定内建 `Task<T>` 在 `created`、`succeeded` 或 `failed` 状态下执行确定性清理；若析构时仍为 `pending`，立即触发致命 trap。

析构函数不隐式 join、detach 或请求取消。`request_cancel()` 不保证任务结束，程序必须在正常异步控制流中显式 `await` 后再让运行中的任务离开生命周期。

议题 53 规定 `failed` 任务析构只释放 `ExceptionBox` 和帧，不检查失败是否曾被观察，也不在析构期间打印、抛出或进入 fail-fast。
