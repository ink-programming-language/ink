# Semantic 测试保留说明

本目录的 17 个原有 `*_test.cpp` 文件保持精简前的完整内容，作为重建 semantic 的行为参考。

当前 semantic 保留公开的 `Analyzer::analyze` 入口、模块及块遍历和基于 `ASTNodes.def` 的语句及声明分派，已重建类型、名称解析和部分表达式分析；尚未支持的节点仍报告 `SemanticUnsupported` ICE。原有测试引用已移除的 IR、execution 等 API，因此暂不加入 CMake 测试目标，也不计入当前通过的测试数量。恢复实现并迁移相关 API 后，再将对应测试加入构建。

新增的 `analyzer_entry_test.cpp` 已加入 `ink_tests`，覆盖公开入口的输入校验、源码归属、嵌套块遍历、具体语句及声明分派、块深度限制和未实现的 comptime 路径。它不依赖已移除的 IR 或 execution。

`analyzer_type_test.cpp`、`name_resolver_test.cpp`、`scope_store_test.cpp` 和 `analyzer_expr_test.cpp` 也已加入目标。类型位置和普通表达式共用 `analyzeExpr`，`analyzeType` 只在结果上要求实际类型。统一入口覆盖名称、字面量、括号、模块/类型成员、指针/引用类型、数组类型和函数类型，直接返回借用的 `const ir::Value *`，失败返回 `nullptr`。具体 IR 类型保留类型/模块描述值、普通值和存储对象的区别，查询存储对象不会隐式加载其内容或产生地址。运行时使用位置另行调用 `requireRuntimeValue`，类型/模块不能作为运行时操作数。

内建类型和布尔常量统一由 `lookupBuiltinValue` 的本地表查询，并从当前上下文取得规范对象；作用域绑定优先于内建回退。一元 `*`、`&` 以操作数本身是否为 `Type` 区分类型构造和运行时操作，指针值的 `type()` 是指针类型并不会使其进入类型构造分支。测试覆盖跨 32/64 位上下文的内建查询和指针参数的运行时分流。

普通表达式的整数默认 `i32`、浮点默认 `f64`、字符默认 `u32`，字符串保留 tokenizer 解码后的 UTF-8 字节并使用 `[u8; 字节数]` 类型。`analyzeExpr` 接收 `ExprHints`：`ExpectedType` 参与最终结果的类型与范围检查，`DefaultIntegerLiteralType` 只改变没有显式目标类型的整数字面量；显式目标类型优先，已有值不因字面量默认类型而转换。两个提示都必须属于当前 IR 上下文，括号透明传递提示，一元操作数、成员接收对象和数组元素等独立位置重新选择提示。类型要求由 `analyzeType` 的 `requireType` 检查，未知名称及成员在所有位置统一报告 `SemanticUnknownName`；多候选重载统一报告 `SemanticAmbiguousName`，单候选函数返回原值后由消费方检查是否适用。

数组长度统一经过 `analyzeExpr`，数组消费方要求非负的 `IntegerConstant`，支持括号、常量名称及模块/类型常量成员。长度分析只通过字面量默认提示选择 `u64`，保留完整 `uint64_t` 范围；已有 `i32/u32` 等整数常量保留原类型，按其位宽检查符号后提取长度。布尔、浮点、字符串及存储对象不能作为长度，数组布局仍单独检查溢出。超过 `u64` 的字面量报告 `SemanticIntegerOutOfRange`。尚未实现的运算和调用在普通表达式、类型及长度位置统一遵循 `SemanticUnsupported` ICE 策略；统一入口检查递归限制并明确拒绝尚未实现的 `comptime` 求值。`ExprItem` 语句已接入表达式入口，函数/变量声明、调用、运行时取地址/解引用和数组构造等后续实现仍未恢复。

目前非空 `ExprHints::ExpectedType` 仅由表达式单元测试传入，生产入口由数组长度消费方设置 `DefaultIntegerLiteralType`；变量初始化、实参和返回值的上下文类型将在对应处理器恢复时接入。类型文本统一由 `ir::Type::toString()` 生成，表达式分析不再分别拼接数值类型名称。

可在 `saved-execution` 分支的 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31` 提交查看原有完整实现和测试构建配置。
