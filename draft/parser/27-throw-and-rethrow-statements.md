# Parser 议题 27：`throw`、重新抛出与显式原因

> 状态：已确认
> 确认日期：2026-08-04

## 1. 基本产生式

Ink 的新异常抛出、重新抛出和显式原因子句使用以下 EBNF：

```ebnf
throw_statement =
    "throw",
    ( ";"
    | expression, [ throw_cause_clause ], ";" ) ;

throw_cause_clause =
    "from", identifier ;
```

`throw` 和 `from` 都是硬关键字。产生式中的 `"from"` 直接匹配 `Keyword(from)`；Parser 不按 Identifier 拼写建立上下文词，也不重新分类 Token。

`expression` 使用已经确认的完整表达式文法，`identifier` 表示任意普通 Identifier Token。三种 `throw_statement` 都以真实分号结束，换行与注释仍然只是 Trivia。

## 2. 三种源码形态

创建并传播一个新异常记录：

```ink
throw FileNotFound { path };
```

创建新异常，并把当前捕获的异常记录连接为直接原因：

```ink
try {
    parse_config();
} catch ParseError as error {
    throw ConfigurationError {
        message: "invalid configuration",
    } from error;
}
```

重新传播当前正在处理的异常记录：

```ink
try {
    load();
} catch IoError as error {
    log(error);
    throw;
}
```

`throw expression;`、`throw expression from identifier;` 和 `throw;` 是同一个语句节点的三种语法形状；三者共享 `throw`，原因形态额外使用硬关键字 `from`。

## 3. `throw expression;` 创建新异常

`throw expression;` 的表达式只求值一次。后续语义检查要求其结果是可以作为新异常载荷抛出的具体异常类对象；普通非异常值、异常接口引用和 `ExceptionView` 都不能仅凭该语法成为新异常载荷。

```ink
throw ParseError { position }; // 直接构造新载荷

var error = ParseError { position };
throw error;                   // 从命名值复制新载荷
```

直接构造、命名值复制、不可复制类型以及“不进行隐藏移动”的规则由语义议题 02、34—36 规定。Parser 不根据表达式类型选择不同语法节点。

即使位于 `catch` 中，普通 `throw expression;` 也不会自动保留当前异常：

```ink
catch ParseError as error {
    throw ConfigurationError {}; // 新记录没有 cause
}
```

需要保留原因时必须显式写出 `from error`。

## 4. `from` 原因子句

`throw_cause_clause` 只能跟在一个已经完成的新异常表达式后。`from` 后准确接受一个普通标识符，不接受成员访问、函数调用、括号表达式或任意通用表达式：

```ink
throw ServiceError {} from error;          // 合法形状
throw ServiceError {} from holder.error;   // 非法
throw ServiceError {} from current();      // 非法
throw ServiceError {} from (error);        // 非法
```

后续名称与控制流检查还要求该标识符是词法上最近一层活动 `catch` 直接建立的绑定。它可以来自具体异常捕获、异常接口捕获或 `catch as error`，但不能是捕获绑定的别名、普通局部变量、外层处理器绑定或已经结束的处理器绑定。

```ink
catch IoError as error {
    throw ServiceError {} from error;
}

catch {
    throw ServiceError {} from error; // 非法：当前 catch 没有 error 绑定
}
```

无绑定的 `catch {}` 需要原因链时必须改写为 `catch as error`。Ink 不提供隐式 `from current`，`from` 也不是能够出现在普通表达式中的二元运算符。

新异常表达式成功建立载荷后，运行时才把当前异常记录提交为新记录的 `cause`，然后开始传播；不会复制或移动原异常载荷。精确记录所有权继续由议题 36 和 40 规定。

## 5. 硬关键字 `from` 的确定性边界

`from` 在 Token 流中始终是 Keyword Token，不能匹配普通 `identifier`。因此以下把它用作绑定名、操作数或成员名的形式都是语法错误：

```ink
const from = fallback_error;       // 非法：绑定名必须是 identifier
throw wrap(from);                  // 非法：from 不能作为操作数
throw factory.from(error);         // 非法：from 不能作为成员名
```

只有 Parser 已经在 `throw` 后完成一个表达式，并且在该语句最外层定界深度遇到下一个显著 Token `Keyword(from)` 时，才开始解析 `throw_cause_clause`：

```ink
throw Wrapper {} from error;
```

实现不需要回溯。`throw` 后立即遇到 `;` 时选择重新抛出分支；否则先按正常表达式状态完成 `expression`，再检查当前定界深度的下一个显著 Token 是否为 `Keyword(from)`。表达式文法本身不能消费该 Keyword 作为名称，因此不存在“表达式中的同名 Identifier”与原因标记之间的歧义；嵌套定界结构内出现非法的 `from` 仍在其所在表达式位置报告，不能提前结束外层表达式。

## 6. `throw;` 重新抛出当前记录

`throw;` 不创建新异常，也不重新求值、复制或移动当前载荷。它继续传播词法上最近一层正在处理的异常记录，并保留其：

- 动态异常类型与接口集合；
- 已有原因链；
- 原始抛出位置与 traceback；
- 运行时记录和载荷身份。

```ink
try {
    outer();
} catch OuterError {
    try {
        inner();
    } catch InnerError {
        throw; // 重新抛出 InnerError
    }

    throw;     // 重新抛出 OuterError
}
```

合法性是语义约束：`throw;` 必须位于当前可执行函数体的一层活动 `catch` 处理器中。词法上写在处理器内部、但属于另一个独立可调用函数、异步体或其他独立控制流主体的代码，不能跨越该边界重新抛出外层处理器的记录。Parser 仍可先建立节点，再由控制流检查报告非法位置。

## 7. 新抛出、原因链与重新抛出的区别

三种形态的运行时含义不能互换：

```text
throw error;
    → 创建新记录，并从命名值复制载荷

throw Wrapper {} from error;
    → 创建新记录，并把当前记录连接为 cause

throw;
    → 复用当前记录并继续传播
```

因此 `throw error;` 即使 `error` 恰好是当前捕获绑定，也不是重新抛出；它仍受复制能力约束，并建立新的抛出位置。`throw;` 不增加原因节点，`throw expression;` 不隐式建立原因节点。

`throw; from error;` 非法：重新抛出分支在第一个 `;` 已经结束，后续 `from error;` 不能附着到它。原因子句必须拥有一个显式的新异常表达式。

## 8. 控制流、清理与 `[nothrow]`

所有 `throw_statement` 都不会正常到达下一条语句，也不产生可供表达式消费的值。新异常表达式求值失败时传播它自身产生的异常；新记录成功建立后，当前作用域才在传播过程中按照议题 03 执行逆序析构和已注册 `defer`。

`throw expression from error;` 的概念顺序为：

```text
求值并成功建立新异常载荷
→ 提交当前记录为 cause
→ 开始传播新记录
→ 展开并清理离开的作用域
```

`throw;` 直接从当前处理器继续传播既有记录，然后执行相同的外层展开与清理。

在 `[nothrow]` 函数中，任何可能让新异常或重新抛出的异常逃离函数边界的路径都是语义错误。局部 `try`/`catch` 可以完整处理新抛出的异常；是否确实覆盖所有逃逸路径不由 Parser 判断。议题 26 的 `defer` 动作也继续要求整体不让异常逃逸。

## 9. `throw` 是语句而不是表达式

三种形态都只能出现在接受 `statement` 的位置，不能作为初始化器、实参或普通运算对象：

```ink
const value = throw error; // 非法
consume(throw error);      // 非法
```

`match_statement` 的分支可以直接使用 `throw_statement`：

```ink
match (result) {
    .ok(value) => consume(value);
    .error(error) => throw error;
}
```

`match_expression` 的分支若需要抛出，必须使用议题 24 允许的发散 `statement_block`：

```ink
const value = match (result) {
    .ok(value) => value,
    .error(error) => {
        throw error;
    },
};
```

换行不能代替三个形态末尾的真实分号。

## 10. CST 形状与确定性解析

CST 至少使用以下节点：

```text
ThrowStatement
ThrowCauseClause
```

`ThrowStatement` 按源码顺序保存 `throw`、可选 `Expression`、可选 `ThrowCauseClause`、`;` 和全部 Trivia。重新抛出由 Expression 与原因子句同时缺席表示，不需要伪造一个异常表达式或额外 Token。

`ThrowCauseClause` 保存真实的 `Keyword(from)`、来源 `identifier` 以及两者之间的 Trivia，不需要伪造或改写 Token，并保持 full-fidelity 源码恢复。

确定性分派顺序为：

```text
consume Keyword(throw)
→ next significant token is ';' ? rethrow : parse expression
→ expression complete and next outer token is Keyword(from) ? parse cause clause
→ expect ';'
```

`Keyword(from)` 与 Identifier 的 TokenKind 已经不同，Tokenizer 和 Parser 都不需要尝试重分类或回溯。

## 11. 错误恢复

缺少结尾分号时继续使用议题 08 的零宽度 `MissingToken(';')` 恢复；在 REPL 输入末尾是否报告 `Incomplete` 继续遵守统一输入完整性规则。

原因标记后立即遇到分号时建立缺失来源标识符：

```ink
throw ServiceError {} from; // 缺少 identifier
```

来源之后出现成员后缀或调用后缀时，真实 Token 必须保留在 `ErrorNode` 或交还外层同步逻辑，不能把专用来源语法扩展成任意表达式：

```ink
throw ServiceError {} from error.cause;
throw ServiceError {} from current();
```

`throw; from error;` 中第一个分号已经形成完整的重新抛出语句，后续 Token 必须作为新的非法结构保留。`throw from error;` 则在 `from` 前缺少新异常表达式；Parser 不能为了猜测用户意图而把原因子句直接附着到 `throw`，也不能伪造 Identifier 表达式。

Parser 恢复不能删除真实的 `from`、来源标识符或分号，也不能把 `throw expression` 静默改成 `throw;`。

## 12. 确认结论

Ink 使用 `throw expression;` 创建并传播新异常，使用 `throw expression from identifier;` 显式连接词法上最近活动处理器的异常记录，并使用 `throw;` 复用和重新传播当前记录。`throw` 与 `from` 都是硬关键字；`from` 在成员导入开头与新异常表达式之后直接作为终结字符串匹配，不作为 Identifier 使用。三个 `throw` 形态都是必须带真实分号的发散语句；Parser 通过 `;`、完整表达式边界和 `Keyword(from)` 确定解析，不依赖回溯或 Token 重分类。
