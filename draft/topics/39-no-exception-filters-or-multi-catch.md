# 议题 39：无异常过滤器与单类型捕获

> 状态：已确认，议题 40—43 与 Parser 议题 28 补充
> 确认日期：2026-08-02

## 1. 一个处理器只有一个匹配目标

Ink 的一个类型化 `catch` 只能指定一个异常类或一个异常接口：

```ink
try {
    request();
} catch NetworkTimeout as error {
    handle_timeout(error);
} catch NetworkFailure {
    mark_network_failure();
}
```

Parser 议题 28 允许类型化处理器选择是否写出 `as name`，但省略绑定不会改变“一个处理器只有一个匹配类型”的规则。

议题 37 的两种 catch-all 是另外两个完整形式：

```ink
catch {
}

catch as error {
}
```

处理器语法中不存在类型列表、类型联合、逗号分隔目标或异常模式组合。

## 2. 不提供多类型捕获

以下写法都非法：

```ink
catch Timeout | Disconnected as error
catch (Timeout, Disconnected) as error
catch Timeout, Disconnected as error
catch [Timeout, Disconnected] as error
```

编译器将其诊断为语法或类型错误，而不是把多个类型隐式合成为公共基类、接口或 `ExceptionView`。

## 3. 使用异常接口或异常基类进行分类

多个异常需要共享一个处理器时，应在类型系统中明确表达共同语义：

```ink
[exception]
interface NetworkFailure {
    func message() -> StringView;
}

[exception]
class Timeout : NetworkFailure {
}

[exception]
class Disconnected : NetworkFailure {
}

try {
    request();
} catch NetworkFailure as error {
    log(error.message());
}
```

异常接口说明这些异常为什么能够由同一代码处理，并为绑定提供明确的静态方法集合。它不会产生“两个无关类型的绑定到底是什么类型”的问题。

具有共同具体状态和单继承关系的异常也可以使用异常基类分类。应使用基类还是接口取决于是否需要共享具体表示；Ink 不为了缩短 `catch` 列表自动创建匿名联合类型。

## 4. 可以重复处理器并调用共同函数

没有合理共同异常类型时，可以保留独立处理器并调用同一个不依赖具体异常值的辅助函数：

```ink
try {
    request();
} catch Timeout as error {
    log_timeout(error);
    mark_request_failed();
} catch Disconnected as error {
    log_disconnected(error);
    mark_request_failed();
}
```

这种重复保持每个绑定的准确类型。编译器不会为了合并相同处理器尾部而改变可观察行为，但普通控制流优化可以消除最终机器码中的重复分支。

## 5. 不提供异常过滤器

`catch` 头部不能包含布尔条件、模式守卫或用户谓词：

```ink
catch HttpError as error if error.status == 404 // 非法
catch IoError as error when error.retryable()   // 非法
catch as error if should_handle(error)           // 非法
```

处理器匹配只读取异常描述符中的类型、父类和异常接口关系，不执行任意用户代码。

## 6. 在处理器体内进行值判断

匹配某个异常类型后，处理器可以使用普通 `if`、`match` 和函数调用检查其只读状态：

```ink
try {
    request();
} catch HttpError as error {
    if error.status == 404 {
        use_default_page();
    } else {
        report_http_error(error);
    }
}
```

所有读取继续遵守议题 38 的只读捕获规则。条件位于普通处理器控制流中，可以正常使用 RAII、`defer` 和异常传播规则。

## 7. `throw;` 不会尝试后续同级处理器

在处理器体中判断后执行 `throw;`，不等价于异常过滤器返回 `false`：

```ink
try {
    request();
} catch HttpError as error {
    if !error.retryable() {
        throw;
    }

    retry();
} catch IoError as error {
    // 即使 HttpError 实现 IoError，也不会接收上面重新抛出的同一个异常
}
```

一旦第一个处理器被选中，同一 `try` 的处理器列表就已经结束选择。`throw;` 按照议题 37 从外层处理区域继续搜索，不回到同一列表。

假设 `HttpError` 实现 `IoError`，如果非重试错误需要由另一个处理器接收，应建立外层处理区域：

```ink
try {
    try {
        request();
    } catch HttpError as error {
        if error.retryable() {
            retry();
        } else {
            throw;
        }
    }
} catch IoError as error {
    handle_non_retryable(error);
}
```

## 8. 不提供过滤器的原因

许多异常过滤器模型会在正式选择处理器和执行栈清理之前运行谓词。允许任意 Ink 表达式进入该阶段会引入额外问题：

- 谓词可以抛出异常，需要定义第二个异常如何处理；
- 谓词可以修改外部状态，并可能在多个栈帧中依次执行；
- 调试器观察到的调用栈仍处于未展开状态，但用户代码已经运行；
- `[nothrow]`、模块卸载和热更新需要分析过滤器调用；
- Itanium 风格 personality 与 Windows EH 对用户过滤代码的自然降低方式不同。

Ink 将处理器搜索限制为描述符匹配，把所有普通用户代码放到处理器正式开始之后。

## 9. 不提供多类型捕获的原因

对于两个无关异常类，多类型处理器必须在以下选择中任取一种：

- 把绑定降级为只能查看名称的 `ExceptionView`；
- 推导可能不唯一的公共异常接口；
- 生成匿名联合引用及新的成员访问规则；
- 只允许不绑定异常的特殊形式。

这些规则都会增加语法和类型推导成本。异常基类与异常接口已经能够显式表达有意义的共同分类，因此 Ink 不增加隐式多类型绑定。

## 10. 成本与实现

该设计不增加正常路径或异常路径的运行时机制：

- personality 只做类、父类、接口或 catch-all 匹配；
- 不执行过滤谓词；
- 不生成联合捕获绑定；
- 不需要额外异常描述符字段；
- 不修改 LLVM 源码。

处理器体中的普通条件按照普通 Ink 控制流降低。重复处理器调用相同辅助函数时，LLVM 可以进行常规尾合并和内联优化。

## 11. 后续问题

以下内容仍留给后续议题：

- freestanding、内核和禁用异常构建模式；
