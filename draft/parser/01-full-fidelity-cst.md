# Parser 议题 01：无损 CST

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 定位

Parser 消费 Tokenizer 产生的 full-fidelity Token 流，首先生成无损的具体语法树（Concrete Syntax Tree，CST），而不是直接生成用于语义分析的 AST：

```text
SourceText
    ↓
Success(TokenizedBuffer { source, tokens })
    ↓ Parser
ParsedFile { lexed_file, cst }
    ↓ CST lowering
AST
    ↓
名称绑定、类型检查和 InkIR 生成
```

三层结构的职责分别是：

- Token 描述源码中存在哪些词法单元；
- CST 描述全部 Token 如何组成具体语法结构；
- AST 描述程序参与后续语义分析的抽象结构。

CST 不执行名称解析、类型推断、常量求值或其他语义判断。

## 2. 逻辑结构

CST 的逻辑数据结构为：

```text
CstTree {
    nodes: CstNode[]
    children: CstElement[]
    root: CstNodeId
}

CstNode {
    kind: CstKind
    first_child: uint
    child_count: uint
    token_count: uint
    text_length: uint
    flags: CstNodeFlags
}

CstElement =
    Node(CstNodeId)
    | Token(TokenRef)
    | Missing(MissingToken)
```

`Node` 表示嵌套语法结构；`Token` 表示一个真实的 Tokenizer Token；`Missing` 表示 Parser 为错误恢复合成的零宽度 Token。

`TokenRef` 在逻辑上引用当前 `TokenizedBuffer.tokens` 中的 Token。CST 不复制原始源码字符串。具体实现可以采用 arena、不可变 green tree 或其他等价表示，但对外必须保持本议题定义的语义。

## 3. 节点种类

`CstKind` 描述纯语法结构，例如：

```text
SourceFile
FunctionDeclaration
ParameterList
Parameter
Block
ReturnStatement
BinaryExpression
CallExpression
NameExpression
TypeReference
Operator
Error
```

它可以表达括号、分隔符和复合符号等具体语法结构，但不能包含变量最终指向的声明、表达式类型或选中的重载等语义结果。

## 4. Full-fidelity 不变量

一个有效 CST 必须满足：

1. 成功词法结果中的每个真实 Token，包括 BOM、Whitespace、Comment 和 EOF，都在根节点下准确出现一次；
2. 真实 Token 叶节点的深度优先遍历顺序与 `TokenizedBuffer.tokens` 顺序完全一致；
3. 真实 Token 不被 Parser 拆分、合并、复制或重写；
4. CST 节点只能覆盖连续的 Token 区间；
5. `MissingToken` 没有对应源码字节，不属于 `TokenizedBuffer.tokens`；
6. 忽略 EOF 和 `MissingToken` 后，依次拼接所有真实 Token 的 `raw`，必须逐字节恢复原始源文件。

因此 Token 列表负责保存源码，CST 负责在不破坏源码顺序的前提下增加语法关系。

## 5. 示例

源码：

```ink
a /*x*/ <= b
```

对应 CST 可以表示为：

```text
SourceFile
├─ BinaryExpression
│  ├─ NameExpression
│  │  └─ Identifier("a")
│  ├─ Whitespace(" ")
│  ├─ BlockComment("/*x*/")
│  ├─ Whitespace(" ")
│  ├─ Operator
│  │  ├─ Symbol('<')
│  │  └─ Symbol('=')
│  ├─ Whitespace(" ")
│  └─ NameExpression
│     └─ Identifier("b")
└─ EndOfFile
```

两个单字符 Symbol Token 在 CST 中可以被 `Operator` 节点组织为一个具体语法结构，但原始 Token 本身仍然保持独立。

## 6. 错误表示

Parser 遇到不符合当前语法的真实 Token 时，必须将其保留在 `Error` 节点中，不能删除或覆盖。

Parser 遇到缺失的必要 Token 时，可以创建：

```text
MissingToken {
    expected_kind
    anchor_byte_offset
}
```

`MissingToken` 只存在于 CST，打印原始源码时忽略；诊断和 IDE 可以用它标记预期插入位置。错误恢复后 Parser 应尽可能继续构造后续 CST。

## 7. CST 到 AST 的 lowering

CST lowering 负责生成适合语义分析的 AST。在该阶段可以：

- 丢弃 Trivia；
- 将相邻的多个单字符 Symbol 解释为一个运算符；
- 去除括号、逗号和分号等只服务于具体语法的节点；
- 把错误和缺失结构转换为 AST 错误占位节点；
- 将具体声明、表达式和类型语法转换为对应的 AST 节点。

例如前述 CST 可以 lowering 为：

```text
BinaryExpression {
    operator: LessEqual
    left: Name("a")
    right: Name("b")
}
```

AST 不负责恢复原始源码；需要原始格式、注释或精确 Token 位置的功能必须使用 CST 和 `TokenizedBuffer`。

## 8. 必须支持的工具能力

CST 是 Ink 前端的稳定基础设施，设计必须支持：

- IDE：按语法节点定位声明、表达式和错误区域；
- 格式化器：理解语法结构，同时读取和重写 Trivia；
- 自动重构：只修改目标 Token，保留无关源码和格式；
- 语法高亮：在 TokenKind 之上使用语法上下文区分 Token 角色；
- 完整而稳定的错误恢复：错误节点和缺失节点不能破坏后续语法树；
- 增量解析：源码局部改变后允许复用未改变的不可变子树；
- 保留注释的代码生成：变换语法结构时仍能访问并迁移原始注释。

因此 CST 节点在构造完成后必须视为不可变；节点应保存可组合的 `token_count`、`text_length` 和错误标志，不把绝对源码位置或父节点直接固化到可复用的节点内容中。绝对 Token 索引、字节位置和父子导航可以由当前 `ParsedFile` 上的轻量视图计算。增量重词法分析、子树复用和编辑后的 Trivia 归属规则将在后续独立议题中定义。

## 9. 确认结论

Ink Parser 只接收成功的词法结果，先生成无损 CST，再独立 lowering 为 AST。CST 的全部真实叶节点引用 Tokenizer Token，保留 Trivia、语法错误和 EOF；合成的缺失 Token 仅服务于错误恢复。该结构是 IDE、格式化、重构、语法高亮、稳定错误恢复、增量解析和保留注释代码生成的共同语法基础。
