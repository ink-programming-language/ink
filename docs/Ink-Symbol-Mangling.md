# Ink 符号身份与名称编码 v2

状态：已接入编译器。LLVM、字节码链接键、名义类身份、反射 getter 和调用 thunk 共用 `_INK2`；旧 `_INK1` 实现和 `__ink_fn_<编号>` 已移除。IR 文本、IR 二进制及字节码容器均为 v7，旧版本明确拒绝读取；字节码指令模式仍为 v3。

本文定义可独立编译、可逆解析的符号身份。包管理、泛型、enum/interface 执行语义等尚未实现的能力必须先补齐其语义元数据，不能因为编码中有对应位置就视为已经支持。

## 1. 设计原则

- 同一个闭合声明由不同编译进程、VM/AOT、不同源码根位置构建时，得到相同的逻辑身份。
- 包、模块、声明和泛型实例都是结构化数据，不通过拼接带点的字符串猜测归属。
- 导入别名和重导出保留原声明身份；确实生成新包装函数时才建立新声明身份。
- 公开函数不依赖遍历编号、指针、NamePool ID、源码绝对路径、时间戳或 LLVM 分配的后缀。
- 所有组成部分可逆，不截断、不默认使用哈希替代身份。超出资源限制时显式失败。
- 源码访问权限、语言链接、调用约定、目标文件链接属性分别处理。
- 编码只消费已经完成语义分析的身份；不在 mangler 中做名称查找、类型推导、常量求值或泛型参数绑定。

## 2. 三种不同的键

| 键 | 内容 | 用途 |
| --- | --- | --- |
| DeclarationKey | 包 + 模块 + 所属声明链 + 声明种类 + 名称 + 泛型形参模式 + 未替换的重载签名 | 确认来自哪一个定义 |
| InstanceKey | DeclarationKey + 所有必要的外层实例 + 按形参顺序排列的规范泛型实参 | 确认哪一个闭合实例 |
| LinkSymbol | InstanceKey + 闭合调用合同或全局对象类型 | 生成原生符号、字节码符号键及反射身份 |

函数返回类型进入 LinkSymbol，但不进入重载选择键。不能仅按返回类型、参数名、默认值、可见性或调用约定声明重载。参数名及命名参数绑定规则保存在导出声明元数据中，不用于区分同类型参数列表的链接符号。

优化等级、调试信息、函数体实现、指令编号不进入这些键。编译缓存还需要源码和依赖指纹，这些缓存信息不能全部塞进符号名称。

## 3. Package 与 module

PackageIdentity 包含四项：

| 字段 | 规则 |
| --- | --- |
| Authority | 包来源的规范命名域，例如 `org.example`；由解析器确定，不能使用本机目录 |
| Name | 包名分段列表，例如 `["graphics", "image"]` |
| Revision | 解析后的精确发布版本或不可变源码修订；不能使用 `^1.0`、`latest` 等选择表达式 |
| Variant | 影响语言定义和 ABI 的配置键值列表，按键的 NFC UTF-8 字节排序，不允许重复键 |

因此 `org.example/demo@1.0.0` 和 `org.example/demo@2.0.0` 可以同时出现在进程中；不同来源的同名同版本包也不会共用身份。Variant 包含启用的语义 feature、影响声明的 cfg 和依赖解析选择；不包含 O0/O2、构建时间或工作目录。依赖选择记录包解析器提供的稳定解析身份，不递归展开整个依赖图，也不使用锁文件中的临时数组下标。

工作区包使用清单中稳定的 Authority/Name 和显式的工作区修订标识；普通源码编辑不自动改变此标识。不同内容冒用同一包身份需要由产物一致性检查拒绝，mangling 不替代版本管理。

ModuleIdentity 是包内相对路径的分段列表。例如 `net/http/client.ink` 对应 `["net", "http", "client"]`。目录分隔符、盘符、源码根位置和 `.ink` 后缀不进入身份。`["a", "b"]` 与 `["a.b"]` 的编码不同；是否允许后一种源码路径仍由模块解析规则决定。大小写敏感，导入别名不改变身份。

稳定身份中的 Authority、Revision、声明名和所有包/模块路径分段必须非空，包名和模块路径至少各有一段；Variant 可以为空，某个配置项的值是否允许空串由其 schema 决定。包来源的别名和版本选择表达式由包解析器先归一化，编码器不能自己猜测两个来源是否等价。

没有包清单的单文件程序可以使用当前链接单元独占的匿名包身份，但不得把它作为稳定的跨包导出。要独立编译并链接多个文件，驱动必须显式获得一致的包身份和模块根；不能永久把所有输入都命名为 `main`。

## 4. 外层编码：可读的长度前缀记录

最终名称仅包含 ASCII 字母、数字和下划线：

```text
Symbol = "_INK2" Record
Record = Tag DecimalLength "_" Payload
Tag = 一个大小写敏感的 ASCII 字母
```

DecimalLength 是 Payload 的 ASCII 字节数，不包含 Tag、长度自身和分隔符。十进制数不得有前导零，零只写 `0`。复合记录的 Payload 是其子记录的连续编码，按本规范的固定字段顺序排列；列表通过记录边界结束，不另存一份冗余数量。

`N` 是名称/文本记录。先验证输入是合法 NFC UTF-8，再逐字节编码：ASCII 字母和数字原样保留，其他字节编码为 `_HH`，HH 必须为大写十六进制；下划线自身编码为 `_5F`。不得把可原样保留的字母数字写成转义形式。长度统计转义后的 Payload。

```text
add     → N3_add
a_b     → N5_a_5Fb
中      → N9__E4_B8_AD
[]      → L0_                 # 空列表
i32     → i2_32               # 有符号整数类型
```

下面用 `Tag(子项...)` 表示对应记录的结构，实际输出由编码器计算长度，不包含括号、逗号、空格和注释。Tag 在不同语法位置可以复用，但解码器必须按该位置的固定 schema 校验，不能只接受任意 TLV 树。

```text
Package     = P(N(Authority), L(N(NameSegment)...), N(Revision), L(E(N(Key), N(Value))...))
Module      = M(N(ModuleSegment)...)
DeclRef     = R(Package, Module, O(Owner...), K(Kind), N(Name), G(GenericParameter...), H(OverloadPattern))
Instance    = X(GenericArgument...)
Function    = F(DeclRef, Instance, Signature)
Global      = V(DeclRef, Instance, W(Mutability), Type)
TypeInfo    = T(Type)
Reflection  = J(Package, Module)
```

Kind 的 Payload 是一个字符：`f` 函数、`v` 全局对象、`c` class、`e` enum、`i` interface。Global 的 Mutability 是 `r` 或 `w`；非函数声明的 H 为空。空实例为 `X0_`。保留未实现类别不表示立即产生对应运行时代码。

默认使用完整编码，不做类型替换表压缩。这样相同实体只存在一种合法拼写，解码不依赖外部符号表。过长名称可以在未来另设明确版本的缩写协议；本版不得静默截断或输出无法独立解码的哈希名。

## 5. 类型编码

类型别名在编码前展开为其规范类型。名义新类型必须保留自身声明身份，不能作为透明别名处理。

| 类型 | 编码结构 | 规则 |
| --- | --- | --- |
| void | `v()` | 不携带字段 |
| bool | `b()` | 与任意整数类型不同 |
| 有符号整数 | `i(十进制位宽)` | 支持当前 IR 的任意合法位宽，例如 i1、i32、i128 |
| 无符号整数 | `u(十进制位宽)` | 与相同位宽的有符号整数不同 |
| 浮点 | `f(十进制位宽)` | v2 定义 IEEE binary16/32/64；其他格式需新增明确编码 |
| 指针 | `p(Access + Type)` | Access 为一个原始字符 `r` 或 `w` |
| 引用 | `r(Access + Type)` | 与指针保持不同类型身份 |
| 切片 | `s(Access + Type)` | 保留只读/可写性，长度是运行时值 |
| 定长数组 | `a(D(元素数), Type)` | 元素数为非负规范十进制整数；闭合类型必须是确定常量 |
| 函数类型 | `q(L(ParameterType...), Q(ReturnType))` | 与现有 IR FunctionType 一致，保存规范参数序列和返回类型 |
| class | `c(DeclRef, Instance)` | 名义身份，不递归编码字段布局 |
| enum | `e(DeclRef, Instance)` | 名义身份，不与底层整数类型合并 |
| interface | `j(DeclRef, Instance)` | 名义身份；不预设其运行时表示 |
| 元类型 type | `m()` | 只用于编译期身份及泛型形参合同，不能进入运行时存储签名 |
| 泛型变量 | `g(D(外层深度), D(形参索引))` | 只允许出现在未闭合声明模式中 |
| tuple，预留 | `t(Type...)` | 有序结构类型；语言支持后才能接受，空 tuple 不等于 void |

其中 `D` 的 Payload 是规范十进制非负整数。整数位宽必须大于零并符合编译器资源限制。当前 string 是只读 u8 切片，其规范类型为 `s(r + u(8))`，不重复设置 string 类型码。字符字面量按语义分析后的实际类型编码。

`const` 局部绑定不自动成为一种 `const<T>` 类型；只有实际存在于类型中的 `AccessKind` 才进入指针/引用/切片编码。当前 class 是名义值类型，不能因为两个 class 的字段相同就合并。

`Node` 中出现 `*Node` 时，指针仅引用 Node 的 DeclRef/Instance，不展开 Node 的字段，因此不会发生递归编码。字段布局、枚举底层表示、interface 布局另存 ABI 元数据。

IR 的 Label、Module、未解析类型、错误类型不能出现在运行时符号签名中。未来的独立 isize/usize、地址空间、闭包、协程等必须明确其类型等价规则后扩展协议；不能把未知类型降级成 void 或普通指针。

## 6. 函数、重载和方法

```text
Signature       = S(B(LanguageLinkage), C(CallingConvention), Receiver, L(ExplicitParameterType...), Q(ReturnType))
Receiver        = A("n") | A("r" + NominalType) | A("w" + NominalType) | A("v" + NominalType)
OverloadPattern = Receiver + L(UnsubstitutedParameterType...)
```

LanguageLinkage 的字节为 `i`（Ink）或 `c`（C）。CallingConvention 为 `c`（C）、`f`（Fast）、`k`（Cold），使用显式文件标签，不依赖 C++ 枚举序号。这些定义只代表可表达的合同，后端不支持的组合必须显式拒绝。

Receiver 的 `n` 表示普通/静态函数，`r` 为只读实例接收者，`w` 为可写实例接收者，`v` 为按值接收者。当前实例方法使用 `w`。闭合方法签名的 ExplicitParameterType 不再重复加入隐式 this；lowering 根据 Receiver 恢复它。函数值类型 q 中则使用 IR 的全部实际参数类型，包括仍显式存在的接收者参数。

当前 IR FunctionType 不包含调用约定。因此 v2 初版只允许 C calling convention 的函数作为一等函数值跨越接口；Fast/Cold 可以编码直接函数符号，但不能在没有额外类型合同的情况下当作普通函数值传递。若未来支持这类函数值，必须扩展语义模型与编码，不能从指针猜调用约定。

显式 `comptime` 形参在实例化时进入 Instance，闭合运行时参数列表不再保留它。位置/命名实参在调用分析阶段归一化；可变参数包若闭合为固定参数序列，编码展开后的类型序列。C varargs 当前不支持，v2 初版不得伪装成固定参数函数。

`Box::get()` 的 Owner 包含 Box 的名义类型及其实例；`Box<i32>::get()` 与 `Box<u32>::get()` 自然不同。`__add__` 等现有特殊方法仍按其真实方法声明名编码。构造/析构协议尚未实现，不通过匹配普通方法名字擅自改变声明种类。

## 7. 泛型定义与实例

GenericParameter 使用 `T()` 表示类型参数、`V(TypePattern)` 表示值参数、`B(ParameterSchema)` 表示参数包；按定义中的形参顺序保存。形参名字和默认实参表达式不进入身份。

GenericArgument 使用 `T(Type)`、`V(Type, ConstantData)`、`B(GenericArgument...)`。绑定命名实参、展开包、补默认值和求值发生在编码之前；不能把调用处的实参文本直接编码。实例必须完全闭合，不能残留 g 或依赖表达式。

Instance 的顶层项数和种类必须与 G 一一匹配；参数包始终保留独立 B 容器，空包与没有这个形参不同。泛型变量的深度 0 表示当前声明，深度 1 表示最近的泛型 Owner，非泛型 Owner 不占层；索引是该层全部泛型形参的零基索引。类型位置只能引用类型参数，数组长度位置只能引用整数值参数，所有引用都必须在对应声明环境中验证。

DeclarationKey 保留未替换的参数模式，所以 `f[T](x: T)` 与 `f[T](x: i32)` 即使都实例化为 `f[i32](i32)` 也不会合并。泛型变量按外层深度和索引引用，重命名 T 不改变身份。所有 Owner 实例也参与身份，不能只编码最内层函数自己的实参。

v2 初版的依赖类型模式支持上述结构类型、类型变量，以及数组长度为直接整数泛型形参的形式（此时 a 的长度位置使用 g，闭合后必须替换为 D）。任意 comptime 类型运算、复杂依赖表达式，以及仅靠约束区分的泛型重载，尚需语义规范；在其规范模式未定义前不得导出 v2 身份，也不得以声明序号或源码文本哈希兜底。这是明确的初版边界。

ConstantData 的规则：

- bool 使用 `B("0")` 或 `B("1")`。
- 整数使用 `B(大写十六进制位模式)`，位数固定为 ceil(BitWidth / 4)；有符号值按其位宽的二进制补码编码，超出位宽的高位必须为零。
- 浮点使用 `B(IEEE原始位模式)`。提案定义泛型浮点身份按位区分，保留负零和 NaN payload；这是泛型键的规则，不改变浮点表达式的比较规则。实现泛型之前须将此约定同步到语义层，不能只由 mangler 单方面实施。
- enum 值带名义 enum 类型和规范底层值；枚举运行时语义尚未完成时拒绝生成。
- 字符串/字节序列使用 `B(大写十六进制内容字节)`，保留长度和嵌入 NUL，不对字符串内容做 NFC 归一化；类型记录决定是数组还是切片。
- 数组、tuple、class 常量使用 `A(V(Type, ConstantData)...)`，按元素或声明字段顺序递归编码；不包含宿主 padding、地址、句柄或初始化位图。结构和元素类型必须符合外层 Type。
- 空指针使用 `Z()`；v2 初版拒绝非空指针、宿主函数地址、运行时资源和其他无法持久化的值。以后支持符号地址时应编码目标 InstanceKey 与偏移，不编码机器地址。

相同 InstanceKey 的多处实例化可以使用 COMDAT/linkonce_odr 等平台机制合并，但必须先验证相同 ABI 合同和定义一致性；符号名相同本身不是任意合并函数体的授权。

## 8. 局部声明与编译器生成实体

Owner 链中的命名类型保存完整的名义 Type；外层函数保存 `F(DeclRef, Instance)`。词法块使用 `B(D(BlockIndex))`，BlockIndex 在直接父块内按语法块顺序编号，忽略空白和注释，不使用字节偏移、行号或全局遍历号。仅匿名/局部实体允许使用这种结构位置，不能用它区分模块级公开重载。

局部函数因此包含父函数的重载身份、外层实例、词法块路径和自身名称。重排父函数中的块可能改变其局部符号，这是可接受的；无关模块的编译顺序不会改变它。局部类型不得泄漏到初版稳定的公开签名中，以免制造对函数局部结构的外部 ABI 依赖。

生成实体使用专属顶层记录，不与用户声明名竞争：

| 实体 | 记录 |
| --- | --- |
| 类型反射描述 | `T(Type)` |
| 模块反射入口 | `J(Package, Module)` |
| 字段默认初始化函数 | `I(NominalType, N(FieldName), Signature)` |
| 编译器 thunk | `H(N(Role), Target, Signature)`，Target 为普通函数 `F(...)` 或字段初始化器 `I(...)`；当前 Role 为 `reflection` |

字段默认初始化器使用字段名和所属类型，不使用字段数组下标。闭包入口、析构胶水、interface witness/vtable 等先预留设计空间；具体身份必须由对应语义功能定义，再加入新的 schema。不得继续追加 LLVM 临时后缀作为可跨产物引用的身份。

## 9. FFI、可见性与 ABI 检查

- `import "C"` 与 `export "C"` 的外部名称遵循显式 C 符号名合同，不加 `_INK2`。平台对象格式要求的装饰由目标后端处理。
- C 导出的 Ink 实现体可以使用 F 记录，另有原名 wrapper；仅 `[abi("C")]` 的本地函数仍保留 Ink 声明身份并编码为 F。
- 公共 Ink 函数按需要输出 external/hidden/default 等链接属性。`public/private` 不进入名称，不等于目标文件的 external/internal。
- 私有函数如果被其他模块实例化的泛型引用，可能仍需可链接的 hidden 符号；源码访问检查仍由语义层执行。
- 原生进程入口 `main` 仍是 wrapper，源码入口函数有正常的 F 身份。其他运行时固定 C 名称维持明确的保留命名域。
- `_INK2` 只说明名称编码版本，不说明完整机器 ABI。产物还必须携带 target triple、数据布局、Ink ABI 版本、使用到的名义类型布局合同和函数传参/返回 lowering 合同。
- Ink 链接驱动在交给 VM 或原生链接器之前检查这些合同。相同包/名义身份出现不一致布局或定义必须报错；不能指望普通系统链接器从名称发现所有不兼容。直接绕过检查的系统链接不属于保证范围。

目标 triple、sret、寄存器分配和结构体字段偏移不直接进入逻辑名称，保持逻辑身份与后端 lowering 分离。名义布局变更需要包版本/ABI 合同更新，即使名称未变也不承诺二进制兼容。

## 10. 完整示例

包 `org.example/demo@1.0.0`，无 Variant，模块 `math`，声明 `func add(A: i32, B: i32): i32`，Ink language linkage、C calling convention、无接收者、无泛型。其完整符号为：

```text
_INK2F139_R94_P42_N13_org_2EexampleL7_N4_demoN9_1_2E0_2E0L0_M7_N4_mathO0_K1_fN3_addG0_H18_A1_nL10_i2_32i2_32X0_S34_B1_iC1_cA1_nL10_i2_32i2_32Q5_i2_32
```

为 `class_bounds.ink` 显式赋予同一个包身份，并采用模块名 `class_bounds` 时，两个普通函数的符号为：

```text
index(): i32
_INK2F130_R96_P42_N13_org_2EexampleL7_N4_demoN9_1_2E0_2E0L0_M18_N14_class_5FboundsO0_K1_fN5_indexG0_H7_A1_nL0_X0_S23_B1_iC1_cA1_nL0_Q5_i2_32

main(): i32
_INK2F129_R95_P42_N13_org_2EexampleL7_N4_demoN9_1_2E0_2E0L0_M18_N14_class_5FboundsO0_K1_fN4_mainG0_H7_A1_nL0_X0_S23_B1_iC1_cA1_nL0_Q5_i2_32
```

这些编码同时用作编解码回归测试的固定样例。源诊断继续使用源声明名字；`abi::demangle()` 返回可逆的结构化记录，供链接工具和后续调试器展示使用。

## 11. 实现与使用

- `src/include/ink/abi/linkage_identity.h`：共享 `PackageIdentity`、`ModuleIdentity` 和 wire `Record`。没有另一套与 IR 重复的类型枚举。
- `src/lib/abi/codec.cpp`：规范编码、严格解码、NFC 检查、泛型模式替换与资源限制。默认单符号 1 MiB、深度 128、记录预算 65536；失败通过结果对象报告。
- `src/lib/ir/linkage.cpp`：唯一的 IR 到符号适配入口，直接读取现有 Type、Function 与所属声明，保留参数类型的访问权限。`ir::functionSymbol()` 和 `ir::reflectionSymbol()` 返回完整机器名字。
- Module 保存结构化包/模块身份，Function 保存语义分析生成的词法块路径，Class 保存规范 `T(c(...))` 名义身份。IR v7 存档保留这些信息。
- LLVM 导入保留 C 名称，导出生成 C wrapper 与独立 Ink 实现；普通 Ink 定义使用 external linkage，private 定义使用 hidden visibility。类 SSA 类型也采用规范名义名字。
- 字节码 v7 保存 `LinkName`，`bytecodeSymbolKey()` 直接使用它。原有 `Signature` 与运行时类型结构继续作为 ABI 合同，读取时检查逻辑签名、泛型实参与描述一致，链接时检查目标、布局与调用合同。
- AOT 检查同一次多模块 lowering 中的名义布局冲突，LLVM module flags 标记 Ink ABI 1 / mangling 2；`.inkabi` 节保存 target、data layout、类布局及函数 lowering 合同，优化后保留。当前 CLI 只负责生成原生对象，没有独立原生对象的 Ink 链接驱动；直接调用系统链接器不会自动校验 `.inkabi`，不在跨产物 ABI 保证范围内。

独立构建时传入一致的已解析包身份和模块根目录，例如：

```powershell
inkc --emit-llvm artifacts/class_bounds.ll --package-authority org.example --package-name demo --package-revision 1.0.0 --module-root src/testcase/backend/programs -i src/testcase/backend/programs/class_bounds.ink
```

`--package-name` 使用 `/` 分隔名称段；`--package-variant key=value` 可以重复，编码前按键排序并拒绝重复键。authority、name、revision 三项必须同时提供。包管理器尚未实现，调用方负责传入已解析版本和语义构建变体。未指定时使用 `local/anonymous@0`，仅用于同一链接单元，不能当作多个独立发布包的共同身份。源码文件相对 `--module-root` 的路径决定 Module，导入别名不影响身份。

源码泛型实例化、泛型类、enum/interface 运行语义、tuple、约束重载和复杂依赖类型表达式仍不受支持。编解码器支持已定义的泛型结构记录、直接类型参数与数组长度参数；类/enum 常量尚无完整语义字段合同，明确拒绝。字节码底层 API 的闭合泛型参数支持原有 scalar/string 集合，位模式统一使用大写十六进制；依赖形参的函数必须显式提供未替换的 `OverloadPattern`，省略意味着参数类型固定。没有使用遍历编号或源码哈希补齐缺失身份。

当前回归覆盖包版本/来源/variant 区分、模块分段歧义、别名透明性、Unicode NFC 与转义唯一性、任意整数位宽、ro/rw、名义递归类型、不同父函数重载与词法块中的同名局部函数、模板模式不同而闭合签名相同、泛型常量位模式、编译顺序及源码根迁移、优化等级、C 导入导出、跨后端名称一致、存档身份与失败回滚、布局不兼容、解码长度溢出、非法转义、未知标签、预算和截断输入。源码泛型语义实现时，还需补充默认值/命名实参规范化与重复泛型定义一致性的语义回归。

编解码器必须验证固定字段顺序、字段数量、允许的 tag、完整消费输入以及资源预算，使用显式结果/诊断返回失败，遵守项目无异常规则。可接受输入必须满足 `encode(decode(Symbol)) == Symbol`；拒绝同一身份的非规范拼写。
