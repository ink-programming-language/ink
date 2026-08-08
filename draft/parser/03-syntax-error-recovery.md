# Parser 议题 03：语法错误恢复

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 前置条件与目标

Parser 只接收 Tokenizer 的成功结果。非法编码、非法字符、错误字面量或其他导致词法失败的输入不进入 Parser；失败 Token 流由 Tokenizer 自己保留。

Parser 的错误恢复只处理“每个 Token 都词法有效，但 Token 组合不符合语法”的情况。遇到语法错误后，Parser 应尽可能继续构造完整 CST，而不是在第一个错误处终止。

## 2. 两种错误结构

Parser 使用两种 CST 结构表达语法错误：

```text
MissingToken(expected, anchor_byte_offset)
ErrorNode(children)
```

- `MissingToken` 表示当前位置缺少语法要求的 Token；
- `ErrorNode` 保存源码中真实存在但无法纳入正常语法结构的 Token；
- 两者都只表达语法恢复结果，不修改 Tokenizer Token 列表。

## 3. 缺失 Token

如果当前 Token 可以明确开始后续结构，而前一结构只缺少一个确定的结束或分隔 Token，Parser 可以在当前位置插入零宽度 `MissingToken`，并且不消费当前真实 Token。

例如：

```ink
func test(value: i32 {
    return;
}
```

在 `{` 前缺少 `)`：

```text
ParameterList
├─ Symbol('(')
├─ Parameter
└─ MissingToken(')')

Block
└─ Symbol('{') ...
```

`{` 仍交给函数体 Parser 正常消费。`MissingToken` 没有源码字节，恢复原始源码时必须忽略。

## 4. 多余或无法归类的 Token

源码中真实存在但不符合当前位置语法的 Token 必须进入 `ErrorNode`，不能删除。

例如：

```ink
func test(, value: i32)
```

可以恢复为：

```text
ParameterList
├─ Symbol('(')
├─ Error
│  └─ Symbol(',')
├─ Parameter
└─ Symbol(')')
```

恢复完成后，Parser 从能够重新进入正常语法的位置继续。

## 5. 上下文同步集合

每类语法结构定义自己的恢复同步集合。例如：

```text
顶层声明：下一个声明起始关键字或 EOF
参数列表：',' 或 ')'
语句：';'、'}' 或下一个语句起始 Token
类型参数：',' 或 '>'
数组结构：',' 或 ']'
代码块：'}'
```

恢复时消费的真实 Token 全部保存在 `ErrorNode` 中。同步 Token 通常不进入错误节点，而是留给正常语法继续消费。

同步集合是语法规则的一部分，不是 Tokenizer 规则。

## 6. 保护外层结束符

内层 Parser 必须知道哪些 Token 可能结束外层结构，并且不能在局部恢复时吞掉它们。

概念上，子语法可以接收停止集合：

```text
parse_type(stop = { ',', ')', ']', '>', '=' })
```

遇到停止 Token 时，内层 Parser 将控制权返回外层；外层再决定正常消费该 Token，或者为内层结构插入一个 `MissingToken`。

该规则避免一个局部错误破坏后续的大段有效语法。

## 7. 强制前进不变量

任何重复解析或错误恢复循环，每轮必须满足以下条件之一：

1. 至少消费一个真实 Token；
2. 插入 `MissingToken` 后立即离开当前分支；
3. 识别到属于外层的停止 Token并立即把控制权返回外层。

Parser 不能在同一源码位置反复插入同一种 `MissingToken`。如果当前 Token 既不能正常解析，也不是外层停止 Token，就必须消费至少一个真实 Token并把它放入 `ErrorNode`。

该不变量保证任意有限 Token 流上的 Parser 都会终止。

## 8. AST lowering

CST lowering 将无法形成正常语义结构的区域转换为明确的 AST 错误占位节点，例如：

```text
ErrorDeclaration
ErrorStatement
ErrorExpression
ErrorType
```

后续名称绑定和类型检查可以识别这些占位节点，避免针对同一个语法缺失继续派生大量无意义错误。

## 9. 确定性与局部性

相同 Token 流和相同 Parser 版本必须产生相同 CST 恢复结构。恢复决策应基于当前语法上下文、局部前瞻和明确同步集合，不能依赖不受限制的全文件猜测。

局部确定的恢复策略有利于 IDE 编辑时保持未修改区域的 CST 稳定，也有利于增量解析复用子树。

相同输入产生的结构化 Parser 诊断也必须具有相同的 Kind、`PrimarySpan`、类型化 `Arguments` 和 `Related`。格式化后的自然语言正文、有效 severity 和具体输出布局不属于 Parser 恢复结构。

## 10. 公共诊断模型

本议题只定义 `MissingToken`、`ErrorNode`、同步集合和前进规则等语法树行为。Parser 诊断使用 [`Diagnostics 议题 01`](../diagnostics/01-diagnostic-model-and-codes.md) 定义的公共模型，并在 Parser 域中登记稳定 `INK-P` 编号。Parser 作为 producer 只提交 Kind、`PrimarySpan`、类型化 `Arguments` 和结构化 `Related`；最终正文、note 和默认 severity 由公共 `DiagnosticFormatter` 生成，行列号及 terminal、JSON、LSP 布局由对应 Renderer/Consumer 决定。

## 11. 确认结论

Ink Parser 只恢复词法有效输入中的语法错误。缺失内容用零宽度 `MissingToken` 表示，多余内容完整保存在 `ErrorNode` 中；上下文同步集合和外层停止集合限制恢复范围，强制前进规则保证 Parser 终止。具体错误诊断统一使用公共 DiagnosticKind 和稳定 `INK-P` 编号。
