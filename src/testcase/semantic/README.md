# Semantic 测试说明

本目录的 9 个 `*_test.cpp` 文件均已加入 `ink_tests`。15 个引用旧 API 的历史测试已原样迁到 [docs/reference/semantic-tests](../../../docs/reference/semantic-tests/README.md)，仅供重建行为时参考，不参与构建，也不计入当前测试数量；归档说明列出了文件和重新接入要求。

当前 semantic 保留公开的 `Analyzer::analyze` 入口、模块及块遍历和基于 `ASTNodes.def` 的语句及声明分派，已重建类型、名称解析、部分表达式分析、函数声明及函数体、局部变量声明、返回语句和调用；尚未支持的节点仍报告 `SemanticUnsupported` ICE。

分析器诊断通过 `State.at(Node)` 或 `State.at(Range)` 创建的 `AnalysisSite` 固定源码文件和范围。可恢复错误由 `RETURN_REPORT_IF`、`RETURN_REPORT_IF_FALSE`、`EXPECTED_TYPE`、`CHECK_TYPE_MATCH` 和 `CHECK_HAS_LAYOUT` 报告并保留原失败返回值；ICE 条件检查统一使用 `CHECK_ICE`，失败时终止，不再附带不可达的失败返回。`requireType` 和 `requireRuntimeValue` 同样接收 Site。不同子节点和名称仍使用各自的范围，Context 后续切换源码不会改变已有 Site 的诊断位置。

`analyzer_entry_test.cpp` 覆盖公开入口的输入校验、源码归属、嵌套块遍历、具体语句及声明分派、块深度限制和未实现的 comptime 路径。它不依赖已移除的 IR 或 execution。`analyzer_function_decl_test.cpp` 和 `analyzer_call_test.cpp` 覆盖已重建的函数声明、函数体、返回语句和调用路径，包括原生函数导入导出以及实参和返回值的目标类型。

`analyzer_variable_test.cpp` 覆盖函数内名称绑定形式的 `var` 和 `const`、初始化器的目标类型与类型推导、分配及初始化存储、运行时读取、无初始化诊断、同域重名、嵌套遮蔽及失败声明的绑定清理。局部可调用值和指针值均保留声明的类型。变量赋值、声明属性、解构、顶层变量、unit 存储和 comptime 声明仍明确报告未实现。

`analyzer_type_test.cpp`、`name_resolver_test.cpp`、`scope_store_test.cpp` 和 `analyzer_expr_test.cpp` 也已加入目标。表达式提供 `analyzeExpr` 和 `emitValue` 两个入口，均返回借用的 `const ir::Value *`，失败返回 `nullptr`。`analyzeExpr` 是共用分析入口，返回绑定原有的 IR 对象，不读取最终变量；普通值、类型描述符和模块描述符也保留原有身份。`emitValue` 在共用分析后读取变量，普通值和描述符直接返回，由消费者验证是否适用。局部变量直接对应 `AllocaInstruction`，其 `type()` 是对象声明类型 `T`、`isStorage()` 为真；读取后的 `LoadInstruction` 类型同为 `T`、`isStorage()` 为假。`analyzeType` 只接受真正的 IR 类型描述符。`analyzeExpr` 保留局部分配和 `GlobalVariable` 存储对象，由消费者根据使用位置检查 `isStorage()` 等属性；通过 `emitValue` 读取全局存储仍报告 `SemanticUnsupported`。括号保留共用分析得到的原对象，嵌套括号不会重复加载变量。

变量的对象类型、可写属性和声明初始化标记由存储 `Value` 自身提供，`Binding` 只负责名称映射；表达式分析和消费直接传递 `Value`，不另带绑定元数据或包装结果。`var` 的存储 `isMutable()` 为真，`const` 为假；初始化通过 `createInitialize` 写入一次并设置声明初始化标记，普通 `createStore` 拒绝不可写存储且不改变该标记。`AnalyzerState` 不另存初始化状态，重新进入已有作用域或新建分析状态仍保留存储对象的属性。`analyzeExpr` 不读取最终变量，因此未初始化变量也能得到原存储对象且不产生读取提示；`emitValue` 对合法局部存储产生一次 `load`，缺少声明初始化时另行报告 `SemanticUninitializedRead` warning（`INK-S0006`），仍返回加载结果。读取及提示均不修改初始化标记。同名遮蔽使用独立存储对象的初始化标记；同一对象的多个名称共享其类型、可写属性和初始化标记。当前标记只记录声明初始化，尚未实现赋值和 CFG 初始化分析。IR 的 `createAddressOf` 从存储对象产生独立的 `*T` 指针值，后者不是存储位置，名称绑定不会改变这一区分。对象类型携带各层可写限定，取地址保留目标的 `const`，加载只去除被读对象的顶层限定。模块和类型成员使用同样的 `Value` 属性规则。

`analyzer_address_test.cpp` 覆盖源级取地址和解引用：`&` 接受局部变量和解引用存储，不读取目标；不可写局部得到 `*const T`。未初始化局部仍生成地址，单独报告 `SemanticUninitializedAddress` warning（`INK-S0062`），非存储操作数仍报告 `SemanticInvalidAddressOperand` error。`*` 先取得指针值，再返回 `DereferenceInstruction` 存储，只有 `emitValue` 消费结果时才加载目标；未初始化指针的读取 warning 不阻止这些步骤。解引用存储不记录声明初始化状态，不用其 `isInitialized()` 判断目标是否可读或产生初始化提示。`&*Pointer` 和嵌套运算只求值一次各自操作数；非指针、无类型 `nullptr` 和 `*void` 的解引用仍报告 `SemanticInvalidDereference` error。测试同时验证 warning 后继续检查操作是否合法，例如未初始化整数的解引用产生读取 warning 和解引用 error，最终分析失败。这些操作不提供目标生命周期、空指针或悬空指针的运行时检查。

初始化 warning 使用独立的 `Site.report`，不通过 `RETURN_REPORT_IF` 返回失败。诊断测试检查两种 warning 的等级、编号及格式；CLI 进程测试要求只有 warning 时保留源码位置并退出 0，warning 与真正 error 共存时退出 1。没有类型也没有初始化器、不可取址操作数和类型不匹配等错误不因 warning 规则而放宽。

内建类型、布尔常量和 `nullptr` 统一由 `lookupBuiltinValue` 的本地表查询，并从当前上下文取得规范对象；作用域绑定优先于内建回退。一元 `*`、`&` 以操作数是否为真正的 IR 类型描述符区分类型构造和运行时操作，指针值的 `type()` 是指针类型并不会使其进入类型构造分支。测试覆盖跨 32/64 位上下文的内建查询和指针参数的运行时分流。

普通表达式的整数默认 `i32`、浮点默认 `f64`、字符默认 `u32`，字符串保留 tokenizer 解码后的 UTF-8 字节并使用 `[u8; 字节数]` 类型。公开的表达式入口通过参数接收当前表达式的 `ExprHints`：`ExpectedType` 参与最终结果的类型与范围检查，`DefaultIntegerLiteralType` 只改变没有显式目标类型的整数字面量；显式目标类型优先，已有值不因字面量默认类型而转换。两个提示都必须属于当前 IR 上下文，违反该前置条件报告 `SemanticConstructionFailed` ICE。括号和数值取负继承提示，运行时取地址/解引用的操作数、成员接收对象和数组元素等独立位置重新选择提示。入口不因提示类型缺少运行时布局而提前报告错误；数组元素、函数签名的返回类型及参数类型、运行时值等具体消费位置检查布局要求。类型要求由 `analyzeType` 的 `requireType` 检查，类型描述本身不要求运行时布局，未知名称及成员在所有位置统一报告 `SemanticUnknownName`；普通名称位置的多候选重载报告 `SemanticAmbiguousName`，调用位置尚未支持重载选择，单候选函数返回原值后由消费方检查是否适用。

数组长度使用共用表达式解析路径，先排除存储位置，再要求非负的 `IntegerConstant`，支持括号、常量名称及模块/类型常量成员。长度分析只通过字面量默认提示选择 `u64`，保留完整 `uint64_t` 范围；已有 `i32/u32` 等整数常量保留原类型，按其位宽检查符号后提取长度。布尔、浮点、字符串及存储对象不能作为长度，数组布局仍单独检查溢出。超过 `u64` 的字面量报告 `SemanticIntegerOutOfRange`。尚未实现的表达式形式在普通表达式、类型及长度位置统一遵循 `SemanticUnsupported` ICE 策略；表达式入口检查递归限制并明确拒绝尚未实现的 `comptime` 求值。`ExprItem` 语句已接入表达式入口，运行时数组构造等后续实现仍未恢复。

生产路径已通过 `ExprHints::ExpectedType` 为实参、返回值和显式类型的局部变量初始化器提供声明的目标类型，数组长度消费方设置 `DefaultIntegerLiteralType`。调用目标、实参和数值取负等运行时消费位置统一通过 `emitValue` 读取存储，再由 `requireRuntimeValue` 验证结果；类型构造调用等未实现形式仍保留各自诊断，全局存储读取统一报告 `SemanticUnsupported` 的 `variable storage reads`。类型文本统一由 `ir::Type::toString()` 生成，表达式分析不再分别拼接数值类型名称。

可在 `saved-execution` 分支的 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31` 提交查看原有完整实现和测试构建配置。
