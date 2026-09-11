# Tokenizer 与 Parser

词法以 `lexer.bnf` 为准，语法以 `grammar.bnf` 为准；`reconstruct_grammar.bnf` 提供等价的预测解析组织。实现分别位于 `src/lib/tokenizer` 和 `src/lib/parser`，公共接口位于 `src/include/ink`。

## Token 流

- 48 个关键字统一使用 `TokenKind::Keyword` 和 `KeywordKind`，包含内置类型名、`true`、`false`、`null`。46 个符号使用 `SymbolKind`，`::`、`>>=`、`...` 等各占一个 token。
- `token.def` 集中维护固定拼写。数字仅记录进制并保留原始文本，不计算数值，也没有数字后缀或字符字面量。
- Token 提供 `is(TokenKind / KeywordKind / SymbolKind)`、`isOneOf(...)`、`keyword()` 和 `symbol()`。Parser 的 `at`、`atAny`、`accept`、`expect` 与配对分隔符均使用枚举；固定拼写只在词法识别、诊断和输出时使用。关键字与符号查找使用 LLVM `StringSwitch`，符号最长匹配直接遍历定义表，不依赖写死的最大长度或表项顺序。
- 扫描遵循最长匹配。例如 `123abc` 是整数与标识符，`0x` 是 `0` 与 `x`，`1e+` 是 `1`、`e`、`+`；它们在表达式中是否成立由 parser 判断。
- 四类字符串均保留原始文本并提供解码值。三引号不要求首尾换行、不裁切缩进；识别到三引号开头后，缺少结束符必须报错。`\xNN` 延续项目原有语义，产生 U+00NN 的 UTF-8 编码。
- 默认忽略空白和注释。`TokenizerOptions::PreserveTrivia = true` 提供完整字节分区，供需要逐字节处理源码的工具使用。两种模式都保留错误 token，并且有效源缓冲区恰有一个 EOF。
- 标识符遵守 XID 和 NFC，直接复用固定版本 Clang 与 utf8proc。文件外层 BOM 不是 trivia；字符串和注释中的合法 Unicode scalar 按文法处理。

源码由 Core `SourceManager` 持有。token 使用 UTF-8 字节半开区间 `SourceRange`，行号支持 CR、LF 和 CRLF。token buffer 与解析结果保留源身份和共享存储。

## 解析与恢复

Parser 直接生成未类型化的语法 AST，不生成或保留 CST。节点保存声明、语句和表达式的实际结构，通过具名子节点角色表达条件、主体、操作数、参数等关系；标识符、字面量和操作符保存为节点载荷。优先级产生的包装规则、普通标点、空白和注释不作为 AST 子节点。原始源码单独保留，可通过节点的 `SourceRange` 取得对应文本。

Parser 接受成功的精简或完整 token 流，不重新扫描源码补齐 trivia。词法失败的输入直接返回失败，不重复发布词法诊断。AST 描述语法结构，不包含名字绑定、推导类型或运行时求值结果。

`Parser::parse` 返回 `ParsedFile`，通过 `ast()` 取得 `AstTree`。`root()` 返回根节点 ID，`size()` / `empty()` 查询节点数量与空树状态，`node(Id).get<CallExpression>()` 等接口访问具体节点的具名字段。`span(Id)` 直接读取已存储的范围。标识符和字面量引用 `lexedFile()` 中的绝对 token 索引；缺失或可选字段使用 `InvalidAstNodeId` / `InvalidAstTokenId`。词法失败或源上下文不匹配时，返回空树和无效根 ID，调用方应先检查失败状态。

AST 使用继承：`AstNode` 保存范围、错误标记和不可变的 `AstKind`；`AstDeclaration`、`AstStatement`、`AstExpression`、`AstPattern` 表达节点类别，具体节点为 `final` 派生类。`SourceFile`、`Error` 和参数节点直接继承 `AstNode`。表达式语句仍通过 `ExpressionStatement` 显式包装表达式，不引入语义阶段的类型或绑定信息。

参照 Clang 的种类标签与 `classof` 约定，`as<T>()` 对不匹配类型返回空指针，`get<T>()` 要求类型匹配，且可直接使用 LLVM `isa` / `dyn_cast`。`visitAstNode` 按具体派生类型分派，`children(Id)` 从具名字段派生遍历视图；无需 AST variant、虚函数或 C++ RTTI。基类禁止外部按值构造、复制和销毁，避免切片或通过非虚析构函数删除派生对象。

`AstTree` 独占所有节点，节点之间通过非拥有的 ID 相连。每个节点按实际派生类型分配和销毁，节点地址不受索引数组扩容影响；销毁不会沿子节点递归，也会正确释放节点内的字符串与容器。树与 `ParsedFile` 只支持移动，不支持隐式复制；移动保留节点地址，移出后的树为空且根 ID 无效。需要复制时应显式设计克隆及 ID 映射。

调用与对象构造共用 `CallExpression`；类型位置解析普通表达式，类型合法性、构造选择及编译期可求值性由后续语义阶段负责。语句开头的 `comptime` 覆盖完整语句，表达式内部的 `comptime` 仅消费一个一元操作数。

列表、运算链、一元前缀、连续 `comptime` 和 `else if` 使用循环。真正嵌套的语法结构受 `MaxSyntaxNestingDepth` 限制；块注释用循环扫描，`MaxBlockCommentDepth = 0` 表示不设置额外层数上限。

二元运算采用优先级爬升，优先级由具名枚举和符号 `switch` 定义，保留 Ink 文法中比较运算不得连续链接的规则。`atAny` 对当前 token 只做一次查找，完整 token 流中的 trivia 不会为每个候选枚举重复扫描。

错误通过 Core 诊断引擎和显式失败状态报告，源码错误使用准确字节范围。语法恢复通过错误节点、缺失结构的锚点和诊断保留失败位置，并保证继续消费或退出；AST 不复刻错误 token 列表。交互模式将可由追加输入完成的 EOF 错误标为 `Incomplete`；已确定的错误仍为 `Complete`。

## 工具与测试

`ink-tokenize [--trivia] [INPUT]` 输出 token 类型、字节范围和转义后的原始文本。`ink-parse [INPUT]` 以显式栈遍历 AST，输出节点种类、具名子节点角色、节点载荷、源码字节范围和错误状态。工具统一使用 `ink::cli::Application`、`writeOutput` 和 Core 诊断格式；退出码 0 表示成功，1 表示源码错误，2 表示调用或 I/O 错误，3 表示内部错误。

`src/testcase/tokenizer` 校验规范词汇、UTF-8、NFC、最长匹配、字符串与错误恢复；`src/testcase/parser` 校验文法、优先级、AST 节点角色与载荷、源码范围、深度限制及恢复。集成语料继续作为解析正例保留，词法与语法文法不会为旧语料放宽。
