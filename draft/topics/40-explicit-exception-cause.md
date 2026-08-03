# 议题 40：显式异常原因链与 `from`

> 状态：已确认，议题 41—43、47 补充  
> 确认日期：2026-08-02

## 1. `from` 显式建立原因

处理器构造并抛出一个新异常时，可以使用 `from` 保留当前异常作为直接原因：

```ink
try {
    parse_config();
} catch ParseError as error {
    throw ConfigurationError {
        message: "invalid configuration",
    } from error;
}
```

正在传播的新异常是 `ConfigurationError`，原 `ParseError` 记录成为它的直接 `cause`。

`from` 是 `throw` 语句中位于新异常表达式之后的上下文关键字，不是普通二元运算符，也不能在其他表达式位置使用。

## 2. 原因必须是当前处理器绑定

`from` 后的标识符必须是词法上最近一层活动 `catch` 直接建立的绑定，可以是：

- 具体异常类的只读捕获绑定；
- 异常接口的只读捕获绑定；
- `catch as error` 建立的 `ExceptionView` 绑定。

```ink
catch IoError as error {
    throw ServiceError { operation: "load" } from error;
}
```

普通局部异常变量、捕获绑定的别名、外层处理器绑定、已经结束的处理器绑定和任意 `ExceptionView` 值都不能作为 `from` 来源。该限制使编译器始终能够确定唯一的当前 `ExceptionRecord`。

无绑定的 `catch {}` 若需要建立原因链，应改写为 `catch as error`。Ink 不另外提供隐式的 `from current` 形式。

## 3. 不复制或移动原异常载荷

`from error` 不读取并复制异常对象，也不把捕获绑定转换成普通可移动值。运行时直接连接两个异常记录：

```text
new ExceptionRecord
    cause_record -> current ExceptionRecord
```

原记录仍包含原始载荷、动态类型、异常接口集合、描述符、销毁入口和模块版本。原异常可以是 `[noncopyable]`，原因链接不要求任何复制能力。

这不是 Ink 的通用移动语义。被重新归属的是运行时本来就拥有的异常记录，不是用户变量或普通对象。

议题 43 的失败 `await` 为当前等待者建立独立活动记录，该记录内部持有共享 `ExceptionBox`。对这种记录使用 `from error` 时，新异常取得当前活动记录作为原因；任务仍保留自己的 box，其他等待者也不受影响。原始异常载荷没有被复制，原因记录只保留既有的运行时内部持有。

## 4. 连接的提交顺序

概念执行顺序为：

```text
取得新异常记录
    → 直接构造或复制新异常载荷
    → 将当前记录连接为 cause_record
    → 开始传播新异常
```

只有新异常载荷成功完成构造后，运行时才提交原因链接。此后控制流不会返回当前处理器，旧记录的所有权从当前处理过程转交给新记录。

新异常表达式求值或构造自身再次抛出时的精确清理与原因策略仍属于异常构造失败议题；实现不得在新载荷尚未完成时留下半连接原因链。

## 5. 普通 `throw` 不自动建立原因

处理器内抛出新异常但没有写 `from` 时，不保存当前异常：

```ink
catch ParseError as error {
    throw ConfigurationError {
        message: "invalid configuration",
    }; // 没有 cause
}
```

原异常按照当前处理器离开规则销毁。Ink 不采用 Python 式隐式 context 链，也不会仅因为抛出发生在 `catch` 中就增加记录保留、模块固定或诊断输出。

显式语法使原因链的时间、内存和热更新成本在源码中可见。

## 6. `throw;` 保留现有原因链

议题 37 的 `throw;` 复用当前异常记录，因此也保留该记录已经拥有的全部原因链：

```ink
catch ConfigurationError as error {
    log(error.message());
    throw; // 原 cause 链保持不变
}
```

`throw; from error` 和没有新异常表达式的其他 `from` 形式都非法。重新抛出不会新增原因节点，也不会把当前异常指向自身。

## 7. 原因链不参与捕获匹配

运行时只使用最外层、当前正在传播的异常记录选择处理器：

```text
ConfigurationError
    cause -> ParseError
```

上述链只匹配 `ConfigurationError` 的类、异常父类和异常接口。即使调用栈中存在 `catch ParseError`，运行时也不会越过外层异常自动捕获其原因。

需要依据原因诊断或分类时，应捕获外层异常后显式查询 `ExceptionView`，或者在异常包装处选择不创建新的异常并使用 `throw;`。

## 8. `ExceptionView.cause()`

`core.ExceptionView` 提供只读、无分配的直接原因查询：

```ink
[nothrow]
func cause() -> Optional<const ExceptionView&>;
```

典型用法：

```ink
catch as error {
    log(error.type_name());

    if let .some(cause) = error.cause() {
        log(cause.type_name());
    }
}
```

`cause()` 只返回直接原因。调用者可以对返回视图再次调用 `cause()` 遍历整条链。

该查询不要求 `[reflect]`，不复制载荷，不增加引用计数，也不暴露 `ExceptionRecord*`。虽然外层使用普通 `Optional<T&>` 表示可选引用，但 `ExceptionView` 是议题 37 定义的异常运行时专用受限视图；返回的 `Optional<const ExceptionView&>` 仍不能越过当前处理器生命周期。这是 `ExceptionView` 自身的规则，不是普通 `T&` 的通用限制。

## 9. 原因链不可变且无环

原因链接在新异常开始传播前一次性建立，之后不能由捕获处理器修改、替换或删除。议题 38 的只读规则同时适用于链中每个异常载荷和每个 `ExceptionView`。

因为新记录只能把当前最近处理器的记录作为原因，而且提交后控制流不会返回，合法程序不能构造原因环，也不能让两个新异常同时拥有同一原因记录。第一版不需要为原因链接增加共享引用计数。

## 10. 销毁与存储释放

最外层异常最终处理完成时，运行时释放整条拥有链。对每个记录的概念顺序是：

```text
保存 cause_record
    → destroy_entry(payload)
    → 释放当前 ExceptionRecord 存储
    → 继续释放 cause_record
```

因此当前异常析构期间，其原因记录仍然有效。运行时可以迭代释放长链，避免让原因深度直接消耗宿主调用栈。

析构和释放继续遵守议题 03、27、34 和 36；原因记录的存在不会允许析构函数抛出。

## 11. 模块版本与热更新

链中每个异常记录分别固定自己的：

- 异常描述符和布局版本；
- 异常接口表与 thunk；
- 销毁入口及其代码；
- 名称与可选动态反射信息。

只要外层异常或任一原因仍然存活，相关模块版本就不能完成卸载。新异常来自新模块版本、原因来自旧模块版本是合法的；运行时不能把整条链强制解释成同一加载代次。

## 12. 未捕获异常诊断

议题 42 的默认未捕获异常报告器应能够从最外层异常开始遍历原因链，并以“由……导致”或等价结构报告每层完全限定异常类型，然后执行不可恢复的进程级 fail-fast。

精确文本格式、是否显示业务消息、最大显示深度、符号化以及敏感数据裁剪属于运行时诊断策略，不改变原因链的所有权语义。报告器应防御损坏的外部 ABI 记录和过深链，不能因打印诊断再次递归失败。

## 13. traceback 的关系

原因链和 traceback 是两个独立概念：

- `cause` 连接多个异常记录；
- traceback 描述某个异常记录的抛出和传播位置。

议题 41 规定 `throw;` 保留当前记录原有的抛出位置和 traceback，不把重新抛出位置写成新的起点。`throw NewError { ... } from error` 为新记录捕获新的位置与可选 traceback，同时完整保留原因记录自己的诊断数据，因此报告器可以分别显示包装异常和原因异常。

## 14. 成本与 LLVM 实现

没有异常发生时，原因链没有正常路径成本。真正抛出但未使用 `from` 时，实现只需把逻辑 `cause_record` 置空；精确 ABI 可以使用记录字段、标志控制的扩展头或等价旁表。

议题 47 的组合等待即使观察到多个失败，也只按参数顺序传播一个任务的原始异常，不把其他失败隐式加入原因链或 suppressed 列表。需要建立原因关系的用户代码仍必须在处理器中显式使用 `from`。

使用 `from` 时的成本是保留一个记录链接、延长原因记录和其模块版本的生命周期，并在最终处理时遍历释放。普通同步原因记录保持唯一所有权，不要求引用计数；若原因来自议题 43 的失败 `await`，其活动记录继续持有已经存在的共享 box 内部引用，直到外层原因链释放。

LLVM 继续只负责异常控制流和栈展开。Ink 前端验证 `from` 来源并生成运行时链接调用，Ink 运行时管理记录所有权；无需修改 LLVM 源码。

## 15. 后续问题

以下内容仍留给后续议题：

- 新异常表达式构造失败时传播哪个异常以及是否保留当前原异常；
- 跨动态库原因记录的精确 ABI；
- freestanding、内核和内存紧张环境的诊断裁剪。
