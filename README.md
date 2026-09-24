# ink

当前按 [`docs/Ink-Lexical-Rules.md`](docs/Ink-Lexical-Rules.md)、[`docs/Ink-grammar-Rules.bnf`](docs/Ink-grammar-Rules.bnf) 和 [`docs/Ink-Parser-Design.md`](docs/Ink-Parser-Design.md) 构建前端，包含 Core、CLI 支持库、tokenizer、parser、semantic 对象模型、`ink-tokenize`、`ink-parse` 及其测试。parser 已替换为 ASTKind 单继承与 Visitor 架构，使用 C++20。semantic 已接入构建，当前提供名称池、类型、常量和变量声明的基础存储，尚不执行名称解析或语义检查。IR、后端和执行器尚未接入新的前端接口，不参与构建。词法、语法和语义模型测试分别位于 `src/testcase/tokenizer`、`src/testcase/parser` 与 `src/testcase/semantic`。

## 构建

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 生成、编译 LLVM/Clang 依赖和 Ink，并运行全部测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ink_tests
ctest --test-dir build -C Release --output-on-failure
```

`ink_tests` 是统一的 GoogleTest 入口，包含 tokenizer、parser 和 AST 的全部单元测试；在 CLion 中运行同名配置即可执行这些用例。构建该目标同时构建 `ink-tokenize` 和 `ink-parse`，CTest 再运行额外的 CLI 进程测试。

LLVM 的构建树包会生成到 `build/third_party/llvm/lib/cmake/llvm`，主工程通过 `LLVM_DIR` 和 `find_package(LLVM CONFIG)` 导入。`utf8proc 2.9.0` 固定使用 Unicode 15.1，与当前词法规则的 Unicode 版本一致。命令行工具统一使用仓库自有的 `ink::cli::Application` 解析参数，并通过 `ParseResult` 显式返回解析和定义错误，不使用 C++ 异常作为控制流。spdlog 1.17.0 负责 Ink 的统一文本输出和运行时日志。

Ink 所有工具共用的参数拼写、输入输出、诊断和退出码规则见 [`docs/command-line.md`](docs/command-line.md)。

基于 [`docs/grammar.bnf`](docs/grammar.bnf) 交互确认的语义分析、泛型、comptime 和新 IR 设计规则见 [`docs/IR.md`](docs/IR.md)。该文档记录设计约定，不表示相关编译管线已经实现。

Ink 跨 module 函数、成员函数、闭合实例、全局变量、Imported 符号和 `extern "C"` 的链接名称规则见 [`docs/name-mangling.md`](docs/name-mangling.md)。

## Tokenizer 接口

- `tokenize(FrontendContext &, std::string)` 或 `tokenizeSource(FrontendContext &, SourceId)` 返回持有源码和解码值的 `TokenizedBuffer`。先检查 `succeeded()`；成功结果以唯一的 `END_OF_FILE` 结束。全局 UTF-8／NUL／BOM 校验失败不输出 token，其他词法错误保留此前完成的 token，不追加 EOF。
- `TokenKind` 为扁平枚举，C++ 枚举项按仓库约定使用 UpperCamelCase，`tokenKindName()` 返回文档中的大写名称。空白、注释和错误不占 token 种类。标识符的 NFC 名称、字符串的解码 UTF-8 值和字符的 Unicode 标量值分别保存在 payload 中。
- Core 的 `SourceLocation` 使用 32 位不透明编码，默认无效，`fromByteOffset(0)` 表示有效的文件起点。`SourceRange` 始终为原始字节的半开区间，可通过 `getBegin()`／`getEnd()` 获取位置。文件身份由源码 buffer 或诊断的 `Source` 保存；不同文件的位置不可直接比较。超过 `0xFFFFFFFE` 字节的输入显式报 `SourceTooLarge`。
- Unicode 正式名称复用 LLVM 的公开名称查询接口；完整别名由官方 Unicode 15.1 `NameAliases.txt` 生成。更新方式见 `cmake/generate_unicode_aliases.py`，生成表记录原始数据的 SHA-256，XID 和 NFC 数据不复制到本项目。

## Parser 接口

- `ink::parser::parse(FrontendContext &, TokenizedBuffer, ParseLimits)` 返回 `ParseResult`。`Unit` 持有不可变 TokenBuffer、ASTContext、ModuleAST 和恢复记录；节点和源码引用在 Unit 销毁前有效。`succeeded()` 同时检查词法结果、当前调用的语法错误和解析状态。
- `Completed` 表示扫描结束，错误输入仍能返回恢复后的 AST。`LimitExceeded` 和 `Cancelled` 明确标识部分结果。ParseLimits 控制嵌套、诊断、工作量与 AST 分配；分配预算触发停止后仍允许构造父节点和最终列表，以返回结构完整的部分树。
- `ASTVisitor`／`ConstASTVisitor` 只分派当前节点，`StrictExprVisitor` 要求覆盖全部表达式。`ASTWalker` 使用显式栈按源码顺序遍历，支持跳过子节点和提前停止。`verifyAST` 校验结构契约；`dumpAST` 返回字符串，不直接产生进程输出。
- `ink-parse INPUT` 或通过标准输入运行 `ink-parse -` 可查看结构与恢复记录。正常退出为 0，词法或语法错误为 1，输入读取失败为 2。
- 测试包含文法家族、恢复边界、Arena 析构与回滚、Visitor、源码生命周期、长链、资源预算、Token 边界扰动及确定性随机输入。可运行 `cmake --build build --config Release --target run_all_tests` 执行完整回归。

## Semantic 对象模型

- 对象模型头文件位于 `src/include/ink/semantic/model`，实现位于 `src/lib/semantic/model`；`Type` 和 `Decl` 的派生类各自使用独立头文件，按需包含对应的 `*_type.h` 或 `*_decl.h`。
- `src/include/ink/semantic/model/Types.def` 用 `INK_SEMANTIC_TYPE(Name, Base)` 集中登记类型种类，`Base` 配置为 `BuiltinType` 或 `UserDefinedType`；`TypeKind` 与两个基类的 `classof()` 分类判断均由该表生成。
- `SemanticContext` 借用 `core::CompilationContext`，拥有 `NamePool`、规范类型与常量、独立的表达式值和声明；对象地址在上下文存活期间保持稳定。后续语义 Session 可以组合这份存储。
- `Name` 只有一个 32 位索引，默认无效。`namePool().intern(Text)` 为相同字节串复用索引，`find(Text)` 不插入，`text(Name)` 返回池拥有的稳定视图。名称只在所属池内比较；调用方负责携带池或上下文，不能将一个池的索引交给另一个池解释。池不执行词法验证或 Unicode 规范化，前端名称应来自已经验证的 token。
- `Type`、`Constant` 和 `ExprValue` 都继承 `Value`，`Decl` 独立于该层次。`Type` 分为 `BuiltinType` 与 `UserDefinedType`：前者包括元类型、void、bool、整数、IEEE 16/32/64 位浮点、定长数组、切片、指针和引用；后者包括由声明身份区分的 `ClassType`、`EnumType`、`InterfaceType`。类型值的类型为元类型，元类型的类型为自身。当前常量仍为 bool 和整数，整数 payload 使用 LLVM `APInt`，宽度不匹配或类型来自其他上下文时返回空指针，不隐式截断。
- `createExprValue(ValueType, Expression, Source)` 为已检查表达式记录结果类型、只读 AST 来源和文件身份，`context()` 返回所属的模型上下文。工厂要求结果类型属于当前上下文，由调用方负责表达式检查；创建不会执行表达式，也不会按 AST 节点合并表达式值。表达式 AST 必须保持存活，后续分析器可按独立的 `ExprValue` 身份关联绑定、转换和实例结果。
- `getArrayType(ElementType, ElementCount)` 按元素类型和 64 位定长长度复用数组类型；多维数组通过嵌套构造。`getSliceType()`、`getPointerType()`、`getReferenceType()` 按目标类型与 `AccessKind::ReadOnly/ReadWrite` 复用类型。访问权限表示能否通过该值修改目标，与变量绑定本身的可变性分开；引用类型也与表达式的值/位置类别分开。这些类型构造器本身属于 builtin，即使目标是用户类。
- `createTypeDecl(Name, Kind, Source)` 登记类、枚举或接口声明，`getUserDefinedType(Declaration)` 返回对应的规范名义类型；同名而身份不同的声明得到不同类型。目前仅保存名义身份，成员、继承、枚举底层类型、布局和泛型实例仍由后续分析实现。组合类型工厂仅检查结构约束；元素合法性、数组布局大小和引用使用规则尚未由分析器检查。
- `createVarDecl(Name, Mutability, Source, Initializer)` 用 `BindingMutability::Mutable/Immutable` 明确绑定初始化后的可变性，分别对应 `var/const`；`mutability()` 返回枚举，`isMutable()` 提供便捷判断。`Initializer` 是可空的 `const Value *`，可指向当前上下文的类型值、常量或表达式值，也可先登记声明，再通过 `setInitializer()` 发布初始化值。`setType()` 和 `setInitializer()` 均拒绝冲突更新及其他上下文的对象；两者都有值时，初始化值在完成所需转换后的类型必须与声明类型一致。每个声明都有独立身份，即使名称相同也不会合并；声明不存放执行时的变量内容。声明来源和表达式值关联的 AST 为借用引用，所属 `ParsedUnit` 必须比语义上下文活得更久。
- 空名称或名称索引空间耗尽返回无效 `Name`；无效或越界索引查询返回空视图。整数工厂拒绝零位宽，浮点工厂拒绝 16/32/64 以外的位宽；组合类型工厂拒绝外来类型和无效访问权限。声明工厂拒绝无效或越界名称，变量声明工厂另检查绑定可变性，类型声明工厂另检查声明种类。上述检查返回显式状态，不依赖异常。
