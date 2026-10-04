# Ink 字节码文件格式

字节码文件保存已经降低的代码、常量和布局，以及供独立对象链接使用的符号身份。它不保存 AST、源 IR、运行中的调用帧、可变堆对象、编译期变量环境、原生地址或 libffi 调用计划。读回完整镜像后，执行器不需要原编译进程的 IRContext；原生导入在新的运行进程中重新查找。

本文描述 v7 文件格式和指令模式 v3。VM 指令、槽位、内存与执行预算见 [Ink 执行字节码](Ink-Execution-Bytecode.md)。源码模块发现、泛型实例化和语义 IR 存档分别属于前端，字节码链接器只接收已经确定的函数与符号元数据。

## 分层与入口

| 层 | 公共入口 | 责任 |
| --- | --- | --- |
| IR lowering | `ExecutionCompiler`、`SemanticValueBridge` | 将具体函数降低为 `ExecutableFunction`，把语义类型与函数身份映射为进程内 ID |
| 对象构建 | [`buildBytecodeObject()`](../src/include/ink/execution/artifact/bytecode_builder.h) | 根据 `BytecodeFunctionInput` 构建一个模块对象；编译全部定义，包括未执行的函数 |
| 文件读写 | [`serializeBytecodeArtifact()`、`deserializeBytecodeArtifact()`、`writeBytecodeFile()`、`readBytecodeFile()`](../src/include/ink/execution/artifact/bytecode_archive.h) | 编码与解码对象或完整镜像，检查版本、target、预算和结构 |
| 对象链接 | [`linkBytecodeArtifacts()`](../src/include/ink/execution/artifact/bytecode_linker.h) | 解析 Ink 导入、检查可见性和签名、合并类型并重映射 ID |
| VM 执行 | `ExecutionLinker(ExecutionImage)`、`ExecutionMachine::execute()` | 执行完整镜像，以新的调用帧和存储产生本次结果；按需准备原生符号和调用计划 |

`BytecodeArtifact::Kind` 区分两种产物：

- `Object` 保存一个 `ModuleName` 的定义以及引用的 Ink/原生导入，入口必须为 `InvalidFunction`。
- `Executable` 是已链接镜像，包含定义与原生导入，不含未解析的 Ink 导入。入口可以省略，供库镜像使用；提供入口时必须指向一个 `Public` 定义。

对象构建要求普通定义及被引用函数有显式元数据；类描述关联的方法和默认初始化函数会自动登记。只有类型的模块也可以形成对象。`Definition` 必须具有 Ink 函数体且属于当前对象的模块；`Import` 引用其他模块中的 Ink 函数，允许引用已分析的定义，但不把其函数体写入当前对象；`Native` 只对应 `FunctionBinding::Import` 原生导入，C ABI 的本地函数与原生导出仍为 `Definition`。空模块名默认使用对象模块，原生导入默认使用 `C`；空声明名默认使用 IR 函数名。函数签名由实际 IR 类型生成，显式提供的签名必须与其一致。泛型实参中引用的类型必须已加入该桥接层的类型表。

源码入口通过 `import math as library;` 后的 `library.answer()`，或 `from math import answer as localAnswer;` 读取并使用目标模块的真实函数签名，顶层函数默认 `public`，显式 `private` 拒绝跨文件访问。普通 Ink 函数必须有函数体，不能用无体声明代替导入；`import "C"` 必须只有声明，`export "C"` 和 `[abi("C")]` 本地函数必须有函数体。模块身份由 `--module-root` 下的相对源码路径确定，CMake 与编译参数不覆盖源声明的可见性或导入目标。生成对象或使用源码导入时，文件扩展必须是 `.ink`，移除扩展后的每个相对路径组件不能包含字面量点号：`pkg/value.ink` 对应 `pkg.value`，`pkg.value.ink` 会被拒绝，避免模块身份歧义。不含导入的单文件 `--interpret` 保留对 `points.windows.ink` 这类旧文件名的支持。文件协议中的 `Module` 可见性继续供底层构建 API 使用，当前语言只映射 `Public` 与 `Private`。

产物拥有独立类型表与代码。调用对象构建器后继续向原桥接层增加类型，不会改变已经返回的对象。

## target 与兼容性

v7 只接受构建与运行端相同的本机 target，且只支持 `CallingConvention::C`。这项限制同样适用于 Ink 函数元数据，不表示 Ink 定义由宿主 C 函数实现。

`nativeBytecodeTarget()` 编码操作系统、架构、本机字节序和指针位宽，例如 `windows-x86_64-le-p64`；ARM32 和 RISC-V 还编码浮点调用 ABI，ARM64EC 使用独立架构名。无法识别的 target 被拒绝。文件中的布局大小和对齐来自该 target；target 不匹配时返回 `IncompatibleTarget`，不自动转换布局或重新编译。文件本身的整数编码固定为小端，与运行端的 target 字节序字段分别校验。

原生导入记录符号名和类型/ABI 描述。链接时先按原生名称查找所有输入对象中的 `Exported` 定义，并检查规范签名及 C ABI；匹配后将导入引用重映射到该字节码函数，源码 private 可见性不阻止原生边界绑定。同名导出的签名不匹配时报告 `SignatureMismatch`，不会退回宿主符号查找。未匹配到导出的导入继续保存为 `Native`，运行时通过宿主 FFI 查找。文件不会固定原生地址，也不保证外部库已经安装；找不到符号或 ABI 不受支持时由执行路径报告对应错误。

`RuntimeFunctionDescriptor::External` 只表示原生导入，`CAbi` 独立记录 C ABI，`Exported` 表示当前定义发布原生符号。`NativeAbi` 表示目标与宿主 ABI 兼容，`Supported` 表示所选 C 调用约定与定参形式受支持；验证器还单独检查实际参数与返回类型。C ABI 函数接受 bool、8/16/32/64 位整数、f32/f64 和指针，返回类型另外允许 void。`[abi("C")]` 与 `export "C"` 的函数体保存在代码表中，VM 直接执行其字节码，不按名字查找宿主实现。原生导出名固定为源码声明名，不随 Ink 模块名、导入别名或私有可见性改变。

当前产物仍是 Ink 字节码对象和镜像，`export "C"` 的 ABI、原生名称和导出方向会被完整验证与保存；当前后端不生成 DLL/共享库，也不把 VM 函数值转换为可传给原生函数的回调地址。库镜像不是操作系统可加载的动态库。`[link(...)]` 不属于已支持语法的语义能力。

## 符号身份与可见性

`BytecodeSymbolIdentity::LinkName` 是唯一的链接键，使用共享的 `_INK2` 规则；原生导入使用原始 C 符号。其他字段用于诊断和 ABI 验证：

| 字段 | 意义 |
| --- | --- |
| `Module` | 原声明所属模块，导入别名不改变该字段 |
| `Name` | 源声明显示名，所属作用域由 LinkName 保存 |
| `Signature` | 与表内数字 ID 无关的运行时 ABI 类型合同，包含参数、返回类型与布局 |
| `GenericArguments` | 按声明顺序排列的有类型泛型实参，包含实参种类、类型身份和精确常量编码 |

`bytecodeSymbolKey()` 直接返回 `LinkName`。包来源、包版本、构建变体、模块段、所属声明、泛型模式与闭合签名均由 [共享符号规则](Ink-Symbol-Mangling.md) 编码。读取和链接会核对该名字与运行时函数签名、泛型实参一致；旧的临时编号或拼接键不能进入 v7 文件。

语义分析记录局部函数的词法块路径，IR 统一符号适配器编码父函数声明和实例；LLVM 与字节码不再各自构造作用域名字。这些定义始终文件私有，不能被其他模块导入，显式标记局部函数为 `public` 也会被语义分析拒绝。

`bytecodeTypeIdentity()` 按类型结构编码种类、位宽、符号、大小、对齐、原生表示、权限和子类型。函数包含返回类型及有序参数；指针包含指向类型；数组包含元素类型和元素数，零长度和嵌套数组同样参与身份编码。Class 使用前端提供的稳定名义身份 `NominalIdentity`，由定义模块与声明身份决定，不按字段结构判等。同名但来自不同声明的 class 保持不同类型，独立 Context 中同一声明的数字 ID 可以不同。

Class 身份作为类型图的叶节点，使 `Node` 包含 `*Node` 等递归指针可以有限编码。读取和链接另行验证定义：同一名义身份的大小、对齐、原生表示、有序字段类型和偏移必须一致；冲突不能被身份合并掩盖。类型表先保留所有 ID，再补全布局，只沿 class/array 的按值边检查循环和深度，拒绝按值自包含、不完整字段、非法偏移及缺失子类型。非 class 的结构身份仍拒绝无法有限编码的类型环。

| 可见性 | 解析范围 |
| --- | --- |
| `Private` | 仅定义所在对象；同模块的另一个对象也不能导入。不同对象中的同名私有定义保持独立 |
| `Module` | `ModuleName` 相同的对象 |
| `Public` | 所有输入对象 |

链接保留定义的实际可见性，不由导入声明扩大权限。不同对象中身份相同的非私有定义报告 `DuplicateSymbol`。精确身份存在但不可见时报告 `InvisibleSymbol`；只有同一包身份下相同模块、所属声明、声明名和泛型实参而签名不符时报告 `SignatureMismatch`；无候选时报告 `MissingSymbol`。私有函数可以被同一对象内的代码正常调用，不要求先导出。

原生导出使用独立的名称空间：任何两个定义的 `Exported` 为真且原生符号名相同，均报告 `DuplicateSymbol`，包括不同模块中的 private 定义。普通 C ABI 本地函数不参与这一重名检查。模块导入仍检查源码可见性与目标 ABI；导入者的 `CAbi` 必须与定义一致，否则报告 `SignatureMismatch`。消费对象保留目标 C ABI，但不会再次设置 `Exported`，最终镜像只保留定义者的导出标记。

## 泛型实参编码

字节码构建要求调用方已经完成泛型实例化，文件记录的是闭合实例身份；当前源码前端的泛型实例化仍另行实现。运行时类型编号不代替泛型类型参数。底层构建 API 的 `OverloadPattern` 保存依赖泛型的未替换形参模式；省略表示参数类型固定，不能拿闭合签名冒充模板模式。

| 实参 | `Kind` | `Type` | `Value` |
| --- | --- | --- | --- |
| 类型实参 | `Type` | 表中该类型的规范身份 | 空字符串 |
| bool 常量 | `Value` | bool 的规范身份 | `0` 或 `1` |
| 整数或浮点常量 | `Value` | 含位宽和符号等信息的规范身份 | 无 `0x` 前缀的大写十六进制位模式；固定长度为 `ceil(bitWidth / 4)`，最高十六进制位中的无效 bits 必须为零 |
| 字符串常量 | `Value` | 字符串的规范身份 | 原始字节的大写十六进制编码，长度为偶数；允许空串 |

例如 i32 常量 1 与 2 分别使用 `00000001`、`00000002`。类型参数 i32、常量参数 i32(1) 和常量参数 u32(1) 是三个不同身份。指针、函数及其他未列出的运行时类型不作为 v7 常量泛型实参。

## 链接与 ID 重映射

每个对象有自己的 `RuntimeTypeId` 和 `FunctionId` 空间，来自不同文件的同一个数字没有共同含义。链接器先验证全部对象并建立原生导出名索引，再按规范类型身份建立最终类型表，分配全部定义和未被本地导出满足的原生导入的最终函数号，之后解析导入并复制代码。被原生导出满足的导入不再保留单独的符号/描述记录，其直接调用和函数值统一重映射到定义。

重映射覆盖：

- 类型布局中的指向类型、数组元素类型、class 字段类型、返回类型和参数类型。
- 函数描述、函数签名、槽位类型和初始值类型，包括递归数组元素和 class 字段中的类型 ID。
- 直接调用目标与所有调用点的签名。
- 初始函数值载荷中的函数 ID，包括数组元素、class 字段、返回或传递给间接调用的函数值。
- 指令元数据标记为 `Layout` 的操作数。

本函数内的槽号、调用点索引、跳转 PC 和常量字节偏移保持相对位置。所有最终函数号先于引用修正分配，因此前向引用、自递归和相互调用不需要递归展开函数体。链接结束再次验证完整镜像；失败不返回部分可执行产物。

## v7 二进制编码

编码使用显式字段，不直接写入 C++ 对象内存。`u32`、`u64` 使用小端编码，布尔字段使用单字节 `0`/`1`；字符串与字节区为 `u32` 长度加原始字节，表为 `u32` 表项数加连续表项。长度不含终止 NUL。索引的无效值为 `0xffffffff`。

| 顺序 | 字段 |
| --- | --- |
| 1 | 8 字节魔数 `INKBC\0\r\n` |
| 2 | `u32` 容器版本，当前为 7，偏移 8 |
| 3 | `u32` 指令模式版本，当前为 3，偏移 12 |
| 4 | `u32` 产物种类：Object=1、Executable=2 |
| 5 | target 字符串、模块名字符串、`u32` 入口函数号 |
| 6 | 类型布局表 |
| 7 | 函数描述表 |
| 8 | 函数代码表 |
| 9 | 符号表，之后不得有额外字节 |

类型表的公共头依次为种类 `u32`、大小 `u64`、对齐 `u64`、原生表示 `bool`、类型名字 `string`。类型 ID 为表项位置，读入后分配新的类型域身份；类型专用信息按种类编码，不再为所有类型写入所有字段。

| 类型 | 公共头之后的载荷 |
| --- | --- |
| Integer | 位宽 `u32`、有符号 `bool` |
| Float | 位宽 `u32` |
| Pointer | 指向类型 `u32`、可写 `bool` |
| Function | 返回类型 `u32`、参数类型表 `u32[]` |
| Array | 元素类型 `u32`、元素数量 `u64` |
| Class | 名义身份 `string`、字段表、方法表 |
| Void / Boolean / String | 无专用载荷 |

Class 字段记录名字 `string`、类型 `u32`、偏移 `u64`、私有标志 `bool`、默认初始化函数 ID `u32`。方法记录名字 `string`、签名 ID `u32`、函数 ID `u32`、私有标志 `bool`、接收者可写标志 `bool`。无默认初始化器使用 `0xffffffff`；导入类的私有方法允许只保留描述而没有调用 ID。方法签名首项必须是对应类的指针，初始化器必须无参数且返回字段类型。链接同时重映射这些类型及函数引用。

数组和 class 仍遵守 `computeTypeLayout()` 的目标布局：元素数量允许为零，class 字段按声明顺序对齐并含尾部填充，空 class 大小为 1。读入和链接只沿按值边检查递归，并补全共享的 `ArrayDesc::ElementLayout` 与 `FieldDesc::Layout`。文件不保存宿主地址、AST 或 IR 指针。

函数描述记录 ID、签名 ID、原生符号名、External/NativeAbi/Supported/CAbi/Exported 五个独立布尔标记。

每个函数记录 ID、签名 ID、局部单元数量、槽位类型表、初始槽表、调用点表、指令数组及常量字节区。每条指令使用一个 `u32` 操作码和四个 `u32` 操作数；调用点使用目标函数号、间接被调用者槽、签名号和实参槽表。

初始槽记录类型 ID、初始化位、载荷种类与 64 位标量 bits。载荷 0 表示无附加载荷，1 保存宽整数的位宽和按低字在前排列的 64 位字数组，2 保存字符串字节，3 表示空指针，4 保存数组，5 保存 class。数组附加载荷为 `u32` 元素数及逐项递归编码的完整初始值记录，class 附加载荷为 `u32` 字段数及声明顺序的完整初始值记录；二者的标量 bits 必须为零，数量、每项类型和初始化状态必须匹配对应布局，空聚合使用零元素数或字段数。函数值保存的是可重映射函数 ID。非空裸指针及活动存储身份不能编码，包括数组元素或 class 字段内的此类指针，返回 `UnsupportedConstant`；`CString` 保存的是执行前的字节与分配指令，每次调用仍重新创建可写缓冲。

符号表记录函数号、符号种类、可见性、完整符号身份及泛型实参。符号种类 Definition/Import/Native 的标签为 1/2/3，可见性 Private/Module/Public 为 1/2/3，泛型实参 Type/Constant 为 1/2。

操作码、运行时种类、比较谓词和失败状态使用显式文件标签，不能直接依赖 C++ 枚举声明顺序。固定映射定义于 [`bytecode_archive_internal.h`](../src/lib/execution/artifact/bytecode_archive_internal.h)；增加或变更指令契约时必须同步维护版本及映射。未知容器版本或指令版本返回 `UnsupportedVersion`。

v7 在符号表中新增规范 `LinkName` 字符串，位于 `Signature` 字符串之后、泛型实参数量之前。读取时同时验证逻辑身份和 ABI 合同，因此拒绝旧容器。指令模式仍为版本 3：指针操作采用裸地址，聚合内存统一使用目标布局，旧的对象身份与字节偏移句柄不能按新契约执行，因此拒绝旧版本。以下聚合操作码保留已有标签：

| 文件标签 | 操作码 | `Operands[0..3]` |
| --- | --- | --- |
| 42 | `Array` | 结果槽、`ConstantData` 偏移、字节长度、0 |
| 43 | `ArrayRepeat` | 结果槽、被重复的元素槽、0、0 |
| 44 | `ArrayElementPointer` | 结果指针槽、数组地址槽、整数索引槽、0 |
| 45 | `ArrayExtract` | 结果元素槽、数组值槽、整数索引槽、0 |
| 46 | `Class` | 结果槽、`ConstantData` 偏移、字节长度、0 |
| 47 | `FieldExtract` | 结果字段槽、class 值槽、字段索引、0 |
| 48 | `FieldPointer` | 结果指针槽、class 地址槽、字段索引、0 |

`Array` 的字节区按元素顺序存放小端 `u32` 源槽号；长度必须等于数组元素数乘 4，所有源槽的类型必须与元素类型一致。这些槽号属于当前函数，链接时不重映射。重复次数取自结果数组布局，重复元素只求值一次。元素地址保留源指针权限；提取返回值快照，两种索引操作执行时都检查负值和越界。

`Class` 的字节区按字段声明顺序存放小端 `u32` 源槽号；长度必须等于字段数乘 4，源槽类型逐项匹配字段类型。`FieldExtract` 读取不可变值快照。`FieldPointer` 的索引是经验证的立即数，派生地址保留原分配身份、生命周期和权限，并增加目标布局字段偏移；整个 class 必须已初始化。整体赋值保留对象的分配身份，因此现有字段地址可以观察后续写入。

容器 v2 曾在函数描述的原有三个布尔标记后增加 `CAbi` 和 `Exported`，其含义在 v7 中保持不变。v7 明确拒绝读取旧容器 v1–v6 或旧指令模式，不推测旧布局与元数据的含义。非定义符号不得设置 `Exported`，原生导入与导出都要求合法的非空原生符号名及 C ABI，重复导出在对象和链接镜像验证中均被拒绝。

写入按类型 ID 保存布局，并按函数 ID 排序函数描述、代码和符号；哈希桶布局不影响同一产物的文件字节。读写往返保持该确定性编码。不同输入对象顺序可能产生不同最终 ID，因此不承诺任意链接顺序生成逐字节相同的文件。

## 读取、验证与预算

`BytecodeLimits` 与 VM 的 `ExecutionLimits` 分开：前者约束构建、链接和归档操作，后者约束实际执行。

| 限制 | 默认值 |
| --- | --- |
| `MaxBytes` | 64 MiB 文件字节 |
| `MaxRecords` | 1,048,576 个累计记录 |
| `MaxStringBytes` | 16 MiB 累计字符串/字节区 |
| `MaxAllocationBytes` | 256 MiB 受控分配预算 |
| `MaxTypeDepth` | 256 层类型结构、按值聚合布局或聚合常量嵌套 |

Reader 在分配前检查计数、长度、剩余输入及预算，再检查类型图、符号/描述/代码关系、指令操作数、初始载荷、调用签名、分支范围和入口。截断、未知标签、重复记录、非法 target、越界 ID、额外尾随字节及不可序列化指针均失败；失败结果不携带部分产物。文件 I/O 使用 `IoError`，预算使用 `LimitExceeded`，格式与镜像错误通过 `InvalidFormat`/`InvalidImage` 等显式状态返回。

静态验证不替代运行中的初始化、权限、指针生命周期、步骤和资源限制。加载有效文件也不会恢复任何过去的执行结果或副作用；调用函数仍从新的帧、实参和初始槽开始。

## 测试覆盖

[`bytecode_builder_test.cpp`](../src/testcase/execution/artifact/bytecode_builder_test.cpp) 检查完整定义输出、元数据边界和类型表冻结。[`bytecode_linker_test.cpp`](../src/testcase/execution/artifact/bytecode_linker_test.cpp) 使用独立 IRContext 和实际对象文件覆盖三个模块、局部 ID 碰撞、不同类型编号、直接/间接/函数值调用、模块与私有可见性、同名私有隔离、重载与泛型身份及链接失败。[`bytecode_archive_test.cpp`](../src/testcase/execution/artifact/bytecode_archive_test.cpp) 覆盖确定性、逐字节截断、两个版本号、target、资源限制、宿主指针拒绝、宽整数与 CString/原生导入往返。[`bytecode_validation_test.cpp`](../src/testcase/execution/artifact/bytecode_validation_test.cpp) 验证未被使用的恶意布局、缺失或循环类型引用、类型深度、伪造函数常量、未被指令引用的非法调用点及非规范泛型参数仍会在进入 VM 前被拒绝。

[`bytecode_process_test.cmake`](../src/testcase/execution/cli/bytecode_process_test.cmake) 为每次编译、链接和执行启动新的 `inkc` 进程，并在移走源码和对象文件后重复运行已链接镜像；同时检查原生符号重新绑定、私有入口拒绝、重载父函数中的同名局部函数以及旧导入和可见性覆盖参数被拒绝。

[`array_artifact_test.cpp`](../src/testcase/execution/artifact/array_artifact_test.cpp) 覆盖嵌套和空数组常量往返、独立类型表之间的数组签名与元素 ID 重映射、数组长度不匹配、损坏的元素载荷及逐字节截断。

[`execution/multifile`](../src/testcase/execution/multifile) 按场景保留多个 `.ink` 文件和对应编译链接清单，每个目录独立注册为 `ExecutionMultiFileTest`。这些主要的多模块源码集成测试通过真实的 `from ... import ...` 与模块成员调用覆盖跨文件调用和相互递归、共享指针读写、重载、限定模块名、默认 `public` 与显式 `private` 边界、跨模块编译期函数依赖，以及导入和符号解析失败，并验证正反两种对象输入顺序和移走源文件后的加载执行。目录结构和运行方式见[命令行测试说明](command-line.md#cli-与源码执行测试)。

[`class_artifact_test.cpp`](../src/testcase/execution/artifact/class_artifact_test.cpp) 覆盖 class 常量和递归指针往返、名义身份冲突、损坏字段偏移、独立类型表重映射及截断。[`class_programs`](../src/testcase/execution/class_programs) 覆盖源码方法与默认值、dunder 值复制、comptime 冻结、访问控制和跨模块类定义；多模块用正反对象顺序链接，并移走原始文件后执行镜像。
