# 议题 03：析构、RAII 与 `defer`

> 状态：已确认，析构函数名称经议题 06 修订
> 确认日期：2026-08-01

## 1. 确定性析构

Ink 对具有析构函数的局部对象采用 RAII 和确定性析构。

对象成功完成初始化后，编译器为它注册清理动作。以下结构化控制流离开对象所在作用域时，必须执行清理：

- 正常到达作用域末尾；
- `return`；
- `break`；
- `continue`；
- `throws` 错误传播。

以下非结构化终止不保证执行清理：

- trap；
- 进程强制终止；
- 无法恢复的硬件故障；
- 外部代码直接终止进程。

## 2. 析构函数的声明

析构函数使用固定的内建名称 `destructor`，并保留 Ink 的 `func` 声明和显式接收者语法：

```ink
[noncopyable]
struct File {
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
- 不得声明 `throws`；
- 不能重载；
- 每个类型最多声明一个析构函数；
- 声明析构函数的类型必须显式标记 `[noncopyable]`。

析构函数可以读取和修改 `this`，但不得延长即将结束的对象生命周期。

## 3. 析构顺序

作用域中的局部对象按照成功初始化顺序的逆序析构。

对于具有用户析构函数的聚合体：

1. 先执行用户定义的 `destructor`；
2. 再按照字段声明顺序的逆序自动析构字段。

```ink
[noncopyable]
struct Server {
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

## 4. 部分初始化

如果对象构造过程中某个字段初始化失败，只析构已经成功初始化的字段，并按照成功初始化顺序的逆序执行。

```ink
var server = Server {
    listener: try Socket.listen(8080),
    log: try File.open("server.log"),
};
```

如果 `listener` 初始化成功而 `log` 初始化失败，编译器只析构 `listener`。由于完整的 `Server` 从未构造成功，不调用 `Server.destructor`。

## 5. 不允许直接调用析构函数

析构函数只能由语言的生命周期机制调用，程序不能直接写：

```ink
file.destructor(); // 编译错误
```

允许直接调用析构函数会产生“对象已销毁但绑定仍然存在”的状态，与 Ink 不提供已移动变量的设计相冲突。

需要提前释放资源时，应使用以下方式之一：

- 进入更小的词法作用域；
- 调用普通的 `close()`、`reset()` 等方法，使对象进入类型定义的合法状态。

```ink
[noncopyable]
struct File {
    handle: OsHandle;

    func close(this: File&) throws {
        if this.handle.is_valid() {
            try os.close_checked(this.handle);
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
    var file = try File.open("data.txt");
    process(file);
} // 自动执行 File.destructor

continue_work();
```

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

与析构函数相同，`defer` 清理不得传播错误。需要可靠处理的失败必须在正常控制流中提前完成。

## 7. 清理期间的 trap

如果析构函数或 `defer` 动作触发 trap，程序立即终止，不再执行尚未完成的清理动作。

因此：

- 可恢复失败不能依赖析构函数报告；
- 需要确认成功的资源释放应使用显式 `close()`；
- 析构函数只负责不可失败的释放，或者在失败时接受立即终止。
