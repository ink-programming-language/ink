# Parser 议题 28：`try` 与 `catch` 语句

> 状态：已确认
> 确认日期：2026-08-04

## 1. 基本产生式

Ink 的异常处理区域使用以下 EBNF：

```ebnf
try_statement =
    "try", statement_block, catch_sequence ;

catch_sequence =
      typed_catch_clause, { typed_catch_clause },
      [ catch_all_clause ]
    | catch_all_clause ;

typed_catch_clause =
    "catch", type, [ catch_binding ], statement_block ;

catch_all_clause =
    "catch", [ catch_binding ], statement_block ;

catch_binding =
    "as", identifier ;
```

`try`、`catch` 和 `as` 都是硬关键字。`statement_block` 使用议题 09 的完整花括号语句块，`identifier` 使用议题 04 的 Identifier Token 类别，`type` 使用议题 29 确认的统一类型非终结符。

`catch_sequence` 是必需结构，因此每个 `try_statement` 至少带一个处理器。类型化处理器可以出现一个或多个；catch-all 最多出现一次，并且只能位于最后。

## 2. `try` 必须带处理器

正常形式为：

```ink
try {
    load();
} catch IoError as error {
    log(error);
}
```

裸 `try` 非法：

```ink
try {
    load();
} // 非法：缺少 catch
```

Ink 不提供独立的“仅建立异常展开边界但不处理异常”的 `try`。没有处理器时，普通语句块、RAII 和 `defer` 已经能够表达作用域与清理；异常会自然继续传播。

`try` 后必须直接跟随 `statement_block`，不能省略花括号：

```ink
try load(); // 非法
```

## 3. 四种 `catch` 形态

类型选择和绑定是否存在相互独立，形成四种处理器形态：

```ink
catch IoError as error {
    log(error);
}

catch IoError {
    record_io_failure();
}

catch as error {
    log(error.type_name());
}

catch {
    recover_without_inspection();
}
```

语义分别为：

```text
catch Type as name -> 匹配 Type，建立只读类型化绑定
catch Type         -> 匹配 Type，不建立绑定
catch as name      -> 匹配全部 Ink 异常，建立只读 ExceptionView 绑定
catch              -> 匹配全部 Ink 异常，不建立绑定
```

`catch Type { ... }` 不是 catch-all；它只是不需要读取异常对象。允许省略绑定可以避免为了忽略载荷而制造未使用变量，并与 C++ 的无名称类型化处理器能力一致。

## 4. 类型化处理器

`typed_catch_clause` 在语法上读取一个完整 `type`。后续语义检查要求它解析为议题 35 的异常类或异常接口：

```ink
catch network.NetworkTimeout as error {
    retry(error);
}

catch IoError {
    mark_failed();
}
```

普通类、普通接口、内建类型、枚举、元组、指针、引用以及其他不具有异常捕获资格的类型都非法。Parser 仍保存完整 `type` CST；它不查询类型声明或异常资格。

一个类型化处理器准确指定一个类型。不接受类型列表、类型联合、模式或隐式公共类型推导：

```ink
catch Timeout | Disconnected as error { } // 非法
catch Timeout, Disconnected { }          // 非法
catch (Timeout, Disconnected) { }        // 非法
```

需要统一处理多个异常时，使用已声明的异常基类或异常接口。

## 5. 捕获绑定

类型化处理器和 catch-all 都使用同一个显式 `as identifier` 形状：

```ink
catch ParseError as error {
    report(error.position);
}

catch as error {
    report(error.type_name());
}
```

绑定的类型和只读性质由语义议题 35、37、38 确定：

```text
catch ConcreteError as error  -> error: const ConcreteError&
catch ErrorInterface as error -> error: const ErrorInterface&
catch as error                -> error: const core.ExceptionView&
```

`catch_binding` 不写 `var` 或 `const`。全部异常捕获绑定固定只读，语法不能请求可变引用：

```ink
catch ParseError as var error { } // 非法
catch var ParseError as error { } // 非法
catch ParseError& as error { }    // 非法捕获类型
```

无绑定形式不创建隐藏名称、占位符或不可引用的临时绑定。

## 6. Catch-all 必须最后

类型化处理器按照源码顺序出现，最后可以追加一个 catch-all：

```ink
try {
    run();
} catch NetworkTimeout as error {
    retry(error);
} catch IoError {
    recover_io();
} catch as error {
    log(error.type_name());
}
```

`catch {}` 和 `catch as name {}` 都匹配全部 Ink 异常，因此它们后面不能再写任何处理器：

```ink
try {
    run();
} catch {
    recover();
} catch IoError { // 编译错误：catch-all 必须最后
    recover_io();
}
```

这不是单纯的不可达 warning，而是违反 `catch_sequence` 的错误。该限制与 C++ 的 catch-all 最后规则一致，同时使后续处理器不会成为看似有效但永远不执行的源码。

多个 catch-all 同样非法：

```ink
try {
    run();
} catch as error {
    log(error.type_name());
} catch { // 非法：第二个 catch-all
    recover();
}
```

## 7. 类型化处理器顺序

多个类型化处理器仍按源码顺序检查，并执行第一个匹配动态异常的处理器。异常类、父类和异常接口之间的匹配由议题 35 定义。

如果前面的类型化处理器静态覆盖后面的处理器，后者保留为合法 CST，但编译器产生不可达 warning：

```ink
try {
    read();
} catch IoError {
    recover_io();
} catch NetworkTimeout as error {
    // warning：若 NetworkTimeout 实现 IoError，则该处理器被遮蔽
    report(error);
}
```

这与 catch-all 后续处理器的硬错误不同。类型化接口可能重叠但互不包含，源码顺序本身也是程序选择优先级的明确机制。

一个处理器开始执行后，同一 `try` 的后续处理器不再参与。处理器中执行 `throw;` 或产生新异常时，从外层处理区域继续查找，不能回到当前 `catch_sequence` 尝试下一个同级处理器。

## 8. 处理器体与词法作用域

`try` body 和每个 `catch` body 都必须是完整 `statement_block`；不支持单语句简写：

```ink
try {
    load();
} catch IoError recover(); // 非法
```

每个 block 建立独立词法作用域：

- `try` body 中声明的局部名称不在处理器中可见；
- 捕获绑定只在对应 `catch` body 内可见；
- 一个处理器的局部名称和绑定不在后续处理器中可见；
- 空 `try` block 和空处理器 block 在语法上合法。

```ink
try {
    const path = current_path();
    load(path);
} catch IoError as error {
    log(error); // path 在这里不可见
}
```

进入处理器前，异常传播已经按照议题 03 清理被离开的 `try` body 局部对象和已注册 `defer`。处理器自身的局部对象与 defer 再按其 block 的普通规则管理。

## 9. 不提供过滤器、`finally` 或 `else`

`catch` 头部不能附加布尔过滤器、模式守卫或任意用户谓词：

```ink
catch HttpError as error if error.status == 404 { } // 非法
catch IoError as error when error.retryable() { }   // 非法
```

需要条件处理时在处理器 block 内使用普通 `if`，需要把未处理情况继续传播时使用 `throw;`。

Ink 也不提供 `finally` 或 `try` 的 `else` 分支：

```ink
try {
    work();
} finally { // 非法
    cleanup();
}
```

跨正常返回、提前跳转和异常展开都必须执行的清理由 RAII 与 `defer` 表达。只在成功路径执行的代码写在 `try_statement` 之后，或通过普通控制流显式组织。

## 10. `try` 是语句

`try_statement` 不产生结果，不能作为初始化器、实参或其他表达式的操作数：

```ink
const value = try { calculate(); } catch { recover(); }; // 非法
```

`try` 也不是调用前缀；普通可能抛出的调用不写 `try`：

```ink
try load();        // 非法
const value = load(); // 合法，异常自然传播
```

整个 `try_statement` 由最后一个处理器的 `statement_block` 结束，后面不写分号：

```ink
try {
    run();
} catch {
    recover();
}; // 非法：多余分号不是空语句
```

## 11. CST 形状与确定性解析

CST 至少使用以下节点：

```text
TryStatement
TypedCatchClause
CatchAllClause
CatchBinding
```

所有节点按源码顺序保存关键字、类型、标识符、statement block 和全部 Trivia。省略绑定时不创建虚假的 `as` 或 Identifier Token；无绑定的类型化处理器仍保存自己的 `Type` 子节点。

`try` 后固定解析一个 `statement_block`，随后至少解析一个 `catch`。消费 `catch` 后，根据下一个显著 Token 确定形态：

```text
'{'       -> unbound catch-all
Keyword(as) -> bound catch-all
otherwise -> parse typed catch, then optional 'as' identifier
```

类型完成后必须遇到 `as` 或处理器的 `{`。这一分派不依赖名称解析、异常资格或 Parser 回溯。

解析到 catch-all 后，当前 `catch_sequence` 完成。随后再遇到 `catch` 时必须诊断顺序错误；恢复实现可以继续构造带错误标记的真实处理器节点，但不能把它视为合法同级处理器。

## 12. 错误恢复与确认结论

`try` block 后缺少任何处理器时，Parser 在该结构边界建立缺失 `CatchClause` 恢复节点；REPL 在输入可能继续补全处理器时按照统一规则报告 `Incomplete`。不能把裸 `try` 静默降格为普通 block。

代表性恢复情况包括：

```ink
try { run(); } catch as { }          // 缺少绑定 identifier
try { run(); } catch IoError as { }  // 缺少绑定 identifier
try { run(); } catch IoError error { } // 缺少 as
try { run(); } catch IoError          // 缺少 statement_block
```

缺失 Token 使用议题 03 的零宽度 `MissingToken`；真实的类型、名称、`catch`、`as` 和花括号必须保存在 CST 或 `ErrorNode` 中。catch-all 后的额外处理器同样必须完整保留，以便诊断和格式化工具准确显示错误源码。

Ink 的 `try` 必须拥有至少一个 `catch`。类型化处理器支持有绑定和无绑定两种形式，catch-all 同样支持有 `ExceptionView` 绑定和无绑定两种形式；所有绑定固定只读。类型化处理器按源码顺序选择并允许静态遮蔽 warning，catch-all 最多一个且必须最后。`try` 与每个处理器都要求完整花括号 block，不提供过滤器、多类型捕获、`finally`、`else`、调用前缀或有值 `try` 表达式。
