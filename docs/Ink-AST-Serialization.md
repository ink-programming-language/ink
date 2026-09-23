# AST 二进制序列化

实现入口是 `src/include/ink/parser/ast_serialization.h`，由 `ink::parser` 提供：

```cpp
ASTSerializeResult serializeAST(core::FrontendContext &Context, const ParseResult &Parsed, ASTArchiveLimits Limits = {});
ASTDeserializeResult deserializeAST(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits = {});
```

保存范围为完整的语法快照：全部 AST 节点字段、嵌入记录、源码名称和原始字节、Token 及解码后的 payload、语法恢复记录、`ParseStatus` 和 `HasSyntaxErrors`。不包含声明索引、符号表、语义绑定、实例结果或诊断消费者中已经输出的诊断。

`ASTSerializeResult::Bytes` 是可能含 NUL 的二进制字符串。文件读写由调用方以二进制方式完成。保存失败时 `Bytes` 为空；恢复失败时 `Parsed.Unit` 为空，接收方的 SourceManager 不增加源码。

`ASTDeserializeResult::succeeded()` 表示成功恢复快照。快照可以来自存在语法错误、词法失败、预算耗尽或取消的解析；是否为成功解析仍须检查 `Result.Parsed.succeeded()`。反序列化不会重新分词、重新解析、重放诊断或执行语义分析。

## 所有权与节点重建

Writer 根据源码遍历顺序，以后序为节点分配从 1 开始的存档局部编号；0 表示可选子节点为空。节点编号和源码 SourceId 都不是跨模块声明身份。原 AST 的地址、Arena 存储、`std::span` 和 `string_view` 的对象表示均不写入文件。

Reader 要求子节点引用指向已经恢复的节点，并校验具体类型或节点类别；共享子节点、自引用、前向引用和游离节点被拒绝。最终必须是一棵以最后一个 `ModuleAST` 为根的树。按后序调用现有构造函数，并通过 `ASTContext::copyArray` 恢复数组，不需要给节点增加 setter 或序列化虚函数。树的遍历和重建不按 AST 深度递归。

`ast_serialization_fields.def` 按构造函数实参顺序描述全部 66 种节点和 9 种嵌入记录，读写共享字段定义。`NameToken`、`RestBinding`、范围、可选值和数组有独立编码。新增 AST 节点而遗漏字段定义会导致编译失败。

恢复的名称文本存放在新的 ASTContext；源码和 Token/payload 由恢复的 TokenBuffer 持有。原 ParseResult、原编译上下文、输入字节和接收方上下文释放后，恢复的 ParsedUnit 仍然有效。成功时源码注册到接收方 SourceManager，获得新的 SourceId；行起始位置由原始源码重新计算。

Tokenizer 的 `TokenizedBuffer::fromSnapshot` 校验源码大小、成功源码的 UTF-8/NUL/BOM 规则、Token 顺序和范围、EOF 契约、种类与 payload 匹配、进制、字符串模式和 Unicode 标量。检查成功后才注册源码。它不会重新运行词法分析，不保证重新推导并比对每个 Token 的词法内容。

## V1 格式

文件以四个字节 `IAST` 开始，其后是 LLVM Bitstream block 8，记录编码宽度为 3，块长度按标准 Bitstream 规则以 32 位字为单位。V1 只接受未缩写记录，不接受其他块、缩写定义或额外尾部数据。底层使用仓库已有 LLVM 的公开 Bitstream 接口，不使用 Clang AST 私有接口。

| 顺序 | Record code | 内容 |
| --- | --- | --- |
| 1 | 1 | 格式版本、ParseStatus、HasSyntaxErrors、词法成功标志、Token 数、节点数、恢复记录数 |
| 2 | 2 | 源码名称，各字节作为一个记录字段 |
| 3 | 3 | 原始源码，各字节作为一个记录字段 |
| 4 | 4 | 每个 Token：种类、范围、payload 标签及其字段 |
| 5 | 5 | 每个 AST 节点：稳定 ASTKind 编号及构造顺序字段 |
| 6 | 6 | 每个恢复记录：节点编号、预期 TokenKind、可选实际 TokenId、范围、ExpectStatus、可选跳过范围 |

各记录字段使用无符号 VBR6 整数。普通字符串为字节数后接字节序列，支持 UTF-8 和解码后的 NUL。范围使用两个明确的 32 位 `SourceLocation` 编码值，即起止字节偏移分别加 1，区间为 `[Start, End)`。必需范围必须有效并位于保存的源码内。

布尔值只能是 0 或 1；可选值以布尔存在标志开头；数组为元素数后接元素字段。普通 TokenId 保存零起始下标，`NameToken::Id` 使用 0 表示 InvalidTokenId，否则保存下标加 1。字面量 TokenId 及其种类和范围必须与 Token 表对应。名称文本及其范围必须与引用的 Token 和源码对应。

Token payload 使用明确标签：0=无数据、1=IdentifierInfo、2=NumericInfo、3=StringInfo、4=CharInfo，不依赖 `std::variant` 的 alternative 顺序。`ASTKind` 使用 ASTNodes.def 中的固定编号；其余枚举使用 `ast_serialization_enums.def` 中独立固定的文件编号，不依赖 C++ 枚举声明顺序。新增 TokenKind 但未补全映射会触发静态断言。

V1 严格匹配 `ASTArchiveVersion`，不做旧版本迁移或未知字段跳过。改变字段含义、数量、顺序或编码时必须更新版本及格式测试。新增枚举值只使用未分配编号，删除后不复用编号。文件不提供签名或真实性认证，结构合法不代表输入源码、语义或编译结果可信。

## 失败与预算

诊断定义按项目约定拆为两份：`src/include/ink/core/diagnostic_user.def` 仅包含用户源码错误；`src/include/ink/core/diagnostic_ice.def` 包含其他失败，包括资源超限、缺失输入、无效存档和不支持的版本。`diagnostic.def` 仅保留 Unknown 哨兵和两个定义表的聚合入口，已有诊断的编号和显示错误码保持不变。`SourceTooLarge` 和 `ParserLimitExceeded` 也归入 ICE。

存档诊断使用具体的 `DiagnosticKind` 和类型化参数，例如 `ASTArchiveSizeLimitExceeded` 携带 `Size`、`MaximumSize`，`ASTArchiveUnsupportedVersion` 携带 `ActualVersion`、`SupportedVersion`。存档错误没有可保证有效的源码位置，因此诊断不伪造 SourceId 或 SourceRange；文件路径由负责文件读写的调用方提供。现有 CLI 的诊断输出格式为 `internal compiler error[INK-P0013]: AST archive size 300 bytes exceeds limit 256 bytes`，ICE 对应退出码 3。

保存和恢复都通过传入的 `FrontendContext::diagnosticEngine()` 报告失败。每次操作只报告首个 ICE，后续级联检查不覆盖首个原因、不重复报告；成功操作不产生存档诊断，也不重放快照中的源码诊断。`ASTArchiveStatus` 继续供调用方判断失败类型，`Message` 是同一条结构化诊断的格式化文本，调用方不得再次报告该消息。不使用异常，也不因诊断分类为 ICE 而调用 abort。Reader 校验签名、版本、块和记录长度、整数溢出、枚举、字节值、数组长度、引用类型、源码范围、Token/payload、恢复元数据和 AST 结构。未完成校验的对象不发布给调用方。

`ASTArchiveLimits` 限制文件字节、源码字节、节点数、Token 数、数组/恢复记录元素数以及累计解码存储。字符串、Token/节点表、AST 节点与数组、恢复表、行索引和相应临时副本均在分配前检查。`MaxAllocationBytes` 是解码存储预算，并作为 Writer 单条记录临时字段缓冲的上限；它不是进程 RSS 上限，不精确包含分配器、Arena 块、容器管理和最终结构验证的辅助内存。宿主内存耗尽仍遵循仓库的进程级致命故障约定。

## 测试

`src/testcase/parser/ast_serialization_test.cpp` 接入统一的 `ink_tests`，覆盖：

- 现有 2,939 个 BNF 样例的往返恢复，比较 AST dump、全部 Token/payload、源码、解析状态、恢复记录及再次序列化的字节。
- 所有已注册节点种类，包括独立构造的 10 种 missing/error 节点及其类型正确的父节点。
- 原对象、原输入字节和两个编译上下文均释放后的生命周期。
- Unicode 名称、各进制、字符、四种字符串模式、解码后的 NUL、换行索引。
- 解析错误、词法失败、取消、资源耗尽和 10,000 层左结合表达式。
- LLVM 通用 BitstreamReader 对输出容器的独立读取。
- ICE 分类、首次失败只报告一次、诊断参数与 Message 一致、成功及恢复树不重放诊断、失败后再次成功调用。
- 逐字节截断、版本和记录顺序、未知种类/枚举、字段缺失、非法引用、范围/Token/payload、UTF-8、恢复信息、VBR 溢出、资源上限及固定种子的 2,000 次字节变异。

```sh
cmake --build cmake-build-debug --target ink_tests
cmake-build-debug/src/testcase/ink_tests --gtest_filter="*ASTSerialization*"
cmake-build-debug/src/testcase/ink_tests --gtest_brief=1
ctest --test-dir cmake-build-debug -R "ProcessTest$" --output-on-failure
```

上述有限测试用于持续回归，不代表穷举所有输入，也不证明后续模块语义加载或跨模块泛型实例化已经实现。
