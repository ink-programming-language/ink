# VM 可执行镜像模型

`ink/backend/vm/executable/executable_file.h` 定义 `ExecutableFile`，动态导入、地址重定位和导出记录集中定义在同目录的 `executable_file_type.h` 中。镜像将指令及全局数据放在同一个 `std::vector<std::uint8_t>` 中，不保存代码和数据的段表。已实现的 [ModuleLinker](VM-Linker.md) 将多个 `MachineModule` 合并为一个逻辑模块；`ExecutableEmitter` 负责全局布局和模块发射流程，`InstructionEmitter` 已实现 opcode、操作数字段及直接调用目标编码，并在布局过程中记录待解析重定位。`EntryOffset` 和导出符号的 `Offset` 均相对 `Image` 开头，不是宿主地址或序列化文件偏移。

## 成员和接口

| 成员 | 访问器 | 含义 |
| --- | --- | --- |
| `Image` | `getImage()` | 镜像拥有的原始字节 |
| `EntryOffset` | `getEntryOffset()` | 入口在镜像中的字节偏移 |
| `DynamicImportSymbols` | `getDynamicImportSymbols()` | 按 u32 下标引用的动态导入符号表，统一表示函数和数据 |
| `ImageBaseAddressRelocations` | `getImageBaseAddressRelocations()` | 加载时需要加上镜像基址的八字节地址字段 |
| `DynamicImportSymbolRelocations` | `getDynamicImportSymbolRelocations()` | 加载时需要填写的动态导入符号地址字段 |
| `ExportSymbols` | `getExportSymbols()` | 镜像导出的符号名称及其镜像内字节偏移 |

访问器传播 const；容器允许复制和移动，复制后的镜像、导入、重定位和导出记录相互独立。符号记录中的 `Name` 是 `core::Name` 句柄，复制后共享进程级 `core::NamePool` 中不可变的名称拼写；重新赋值某个记录的 Name 不会改变其他副本。`byteSize()` 返回镜像字节数。默认构造只创建空容器；空镜像没有有效入口，不能通过校验。

## 显式入口与导出选择

模块发射使用以下接口，入口名称和导出列表都必须显式提供：

```cpp
ExecutableFile emit(const MachineModule &Module, core::Name EntryFunctionName, std::span<const core::Name> ExportNames);
```

`EntryFunctionName` 必须是输入模块中有效、具名且包含至少一条指令的 VM 函数，不能指定全局变量、Native 声明或空函数。发射器先完成全局及函数布局和静态重定位，再将该函数的镜像偏移写入 `EntryOffset`。接口不默认查找 `main`。

`ExportNames` 仅选择显式列出的 VM 函数或全局变量，按列表顺序生成 `ExportSymbols`；名称必须有效、唯一且能解析到输入模块中的目标，最终偏移须小于镜像字节数。列表中的名称同时作为目标名和导出名，没有额外别名。Local 定义也可以显式导出，未列出的 External 定义不会自动导出；传入空列表表示没有导出。

例如，已构造的 `Module` 包含带指令的 `start` 函数和 `counter` 全局变量时：

```cpp
core::NamePool &Names = core::NamePool::instance();
const auto EntryName = Names.intern("start");
const std::array<core::Name, 2> ExportNames{EntryName, Names.intern("counter")};
ExecutableEmitter Emitter(Diagnostics);
ExecutableFile Executable = Emitter.emit(Module, EntryName, ExportNames);
```

每次 `emit` 先重置 `ExecutableEmissionContext` 中的镜像，旧的入口偏移和导出记录随之清空，随后按本次参数重新生成。

## 字节序和镜像范围

VM 镜像固定使用小端序，操作码、偏移、立即数及两类地址重定位字段均按小端序编码，不保存字节序配置。模型保存全局数据的原始字节；地址字段的加载绑定由重定位记录描述，指令使用八字节对齐编码：八字节头部保存 opcode 和最多三个操作数的寄存器编号，每个语义操作数另占八字节槽。解释器直接读取有类型的运行时整数槽，已实现的指令执行范围见 [VM 运行时](VM-Runtime.md)。

镜像字节可由调用方显式构造，或通过发射器当前提供的全局布局与操作数字段接口追加；镜像模型本身不执行布局。`ExecutableEmitter::getExecutable()` 借用当前构建中的未绑定镜像，只提供 const 访问；后续发射仍可能修改其内容。镜像模型不携带代码范围和数据权限信息；后续加载及执行阶段如何建立指令边界、区分代码与数据和实施权限检查仍待确定。结构校验要求入口偏移位于镜像内且满足八字节对齐，但不识别代码、数据或完整指令起点。

`ExecutableEmitter` 拥有 `ExecutableEmissionContext`，上下文保存镜像、模块符号、`ImportSymbols` 和待解析符号重定位，并提供字节追加及回填接口。`ImportSymbols = SymbolDeclarations - (Functions + Globals)` 表示当前镜像之外待绑定的声明，目标可以由其他 Ink 库或 Native stub 提供。函数发射器按函数、基本块和指令顺序发射；基本块目标仍使用八字节 PC 相对位移，前向引用由函数发射结束后的 `resolveBasicBlockStaticRelocations` 回填。普通符号地址和直接调用目标统一使用加载地址重定位。

## 运行时地址与未绑定镜像

运行时地址统一使用 64 位宿主地址，BP、SP、PC、数据指针和函数入口采用同一地址坐标。Loader 参数 `ImageBase` 是镜像在代码段中的字节偏移，运行时基址为 `CodeSegment.data() + ImageBase`。本镜像符号地址通过镜像基址重定位填写，外部符号通过动态导入重定位填写。符号内存地址使用 ZERO 基址加八字节地址字段；寄存器相对内存和基本块、直接跳转仍保留各自的寄存器及 PC 相对表示。

`ExecutableFile` 保存未绑定、与进程无关的镜像，地址字段不保存加载后的宿主指针。解释器使用 `std::vector<std::uint64_t>` 中具有有效生命周期的整数槽管理独立的运行时代码段及其有效字节数、容量；代码与数据仍共享同一地址坐标，字节视图用于镜像复制和任意数据访问；`getCodeSegment()` 返回有效字节的只读 span，`getCodeSegmentCapacity()` 返回容量。加载器在运行时代码段中绑定地址，原始镜像与元数据始终保持不变；入口执行的宿主地址为 `RuntimeImageBase + EntryOffset`，导出地址为 `RuntimeImageBase + Export.Offset`。不能将已绑定指针缓存作为可执行文件内容保存。统一地址表示不改变 VM 调用与 Native 调用的 ABI 区别，native adapter 仍须处理调用协议。

代码段容量不足时分配更大的整数槽数组，复制覆盖既有有效字节的完整整数槽并释放旧存储；容量按八字节向上取整，逻辑有效字节数不变。地址发生变化后，解释器会更新统一符号表中的旧代码段地址，并修补既有库的两类地址重定位字段。动态重定位先从旧绑定值去掉 Addend，依据原符号基地址判断是否属于旧代码段，再换算新地址并加回 Addend；Native 地址及旧引用已经选择的同名符号目标保持不变，不按当前名称重新绑定。镜像基址重定位则根据各库的新宿主基址、`TargetOffset` 和 Addend 重新计算，不依赖字段中的旧值或统一符号表。运行时代码段搬迁不改写任何 `ExecutableFile`。外部持有的 span 或裸地址不在修补范围内，后续加载尝试即使最终失败，也可能已完成搬迁；调用方应重新取得 view 或查询地址。执行期间的其他地址稳定性仍须由后续执行器保证。

## 动态导入符号

`executable_file_type.h` 中的 `DynamicImportSymbol` 只保存 `core::Name Name`，表示待加载时解析的符号名称。统一符号表可以提供其他 Ink 镜像的导出地址或 Native stub、数据地址；镜像中的导入记录不预先指定来源。

导入表内的 `Name` 必须唯一。同一名称供调用和取地址共同引用，无需因用途不同重复导入。导入表本身不区分函数和数据；逻辑模块中的同名声明仍须通过种类和只读属性的一致性检查。Name 默认为无效的 `core::NAME_None`，必须通过进程共享的 `core::NamePool` 登记非空拼写后使用。导入记录保存待解析名称，不保存宿主指针或已绑定的运行时对象。动态导入项按指令的实际使用生成，未使用的未定义声明不会自动加入此表。

符号在 `DynamicImportSymbols` 中的位置是重定位元数据引用的 u32 索引，不作为调用操作数写入指令。`Calli` 的操作数是八字节地址字段；外部函数调用与取地址按同一名称复用导入项，各自生成 `DynamicImportSymbolRelocation`。Loader 将实际绑定地址写入每个字段，调用执行时不读取导入索引。

`Callm` 复用 `AddressOperand`，从寄存器基址加编码偏移指向的内存槽读取 64 位函数地址，允许未对齐槽位，调用方须保证内存可读。取得目标地址后与 `Calli` 共用调用逻辑：代码段内对齐目标直接按 Ink 调用协议执行，其他目标必须命中 Native stub 地址集合才按 `void(Frame*)` 调用。

逻辑模块使用 `MachineSymbolDeclaration` 保存名称、函数或数据种类及只读属性。未由当前镜像定义满足的声明形成 `ImportSymbols`；不能据此认定目标是 Native。实际使用的外部符号按名称进入统一导入表。现有 Native 调用仅支持已注册的 `Frame*` stub，完整调用签名及通用宿主 ABI adapter 仍待扩展。

## 动态导入符号重定位

`executable_file_type.h` 中的 `DynamicImportSymbolRelocation` 保存 `ImageOffset`、`SymbolIndex` 和 `Addend`，没有重定位种类字段；其静态常量 `ByteSize` 为 8。

`ImageOffset` 指向待填写字段的第一个字节，而不是所在指令的起点，该字段可以位于代码或数据中。`SymbolIndex` 是 `DynamicImportSymbols` 的 u32 下标，可以引用函数或数据符号。`Addend` 是解析符号地址后施加的有符号字节偏移，最终填写值为 `ResolvedSymbolAddress + Addend`；例如引用宿主 buffer 的第 8 字节可以使用 `Addend = 8`，只引用符号本身时 Addend 为零。记录存入 vector，不要求按地址排序。

本表记录当前镜像之外的符号地址，包含符号内存地址、普通取地址及直接调用目标；本镜像内符号使用镜像基址地址重定位。地址字段先追加八字节零，外部符号按名称复用导入，`ImageOffset` 指向字段本身。内存寻址在头中使用 ZERO 基址，普通立即数和 `Calli` 不额外追加寄存器字节。函数与数据地址共享本表，地址重定位不改变调用协议，也不赋予任意地址可调用性。

`Interpreter` 持有统一的 `ExportSymbolMap`，以名称映射到 `uint64_t` 表示的真实宿主地址。构造时自动调用公开的 `initialize()`，通过 `try_emplace` 登记 `NativeSymbolAddresses`；重复初始化不会覆盖已有绑定。`Interpreter::resolveSymbolAddress(Name)` 仅查询此表，缺少名称时返回 nullptr。内置 `identity` 对应 `identityStub` 的地址，沿用 [VM 调用约定](VM-Calling-Convention.md) 中的 `Frame *` 桩接口。

`Loader::loadLibrary(CodeSegment, ImageBase, Executable)` 接收可写 span，其中 `ImageBase` 是镜像在 `CodeSegment` 中的字节偏移。加载器先遍历本库的 `ExportSymbols`，校验偏移后，将 `CodeSegment.data() + ImageBase + Export.Offset` 的真实宿主地址写入统一符号表，同名项覆盖既有绑定。随后遍历完整的 `DynamicImportSymbols`，每项通过 `Interpreter::resolveSymbolAddress(Name)` 查询一次，按导入索引保存结果；即使没有重定位引用，导入项也必须能解析，本库导出也可供本库导入使用。动态重定位仅按 `SymbolIndex` 读取已解析地址，计算 `ResolvedSymbolAddress + Addend`，通过目标字段上的 `uint64_t *` 直接写入，不建立修补数组；同次加载也完成下节的镜像基址地址重定位。

直接写入要求小端宿主，Loader 使用平台字节序宏在编译时拒绝大端平台。实际目标地址 `CodeSegment.data() + ImageBase + ImageOffset` 必须满足 8 字节对齐。发射器将指令及操作数槽按八字节布局，解释器加载每个非空镜像前在库间补零，使 `ImageBase` 对齐到八字节；空镜像加载不改变有效代码段长度。重定位字段也必须位于八字节边界。库间补零只属于运行时代码段，不改变 `ExecutableFile::Image` 或其中的相对偏移。

`Interpreter` 持有 `DiagnosticEngine`，可通过 `getDiagnostics()` 注册诊断 consumer。找不到导入地址时报告 `VMLibraryLinkFailed`（`INK-V0004`）并返回 `false`。容量增长和既有地址修补发生在加载事务之前；事务恢复本次登记前的符号表、有效代码段长度及 handle，删除新增库记录，但已完成的容量增长和地址搬迁保留。既有有效字节中的非地址内容保持不变，既有地址绑定与当前存储一致，不保证失败后恢复原指针、容量或地址字段的原位模式。直接调用 Loader 的调用方自行负责回滚代码段逻辑状态及符号表；Loader 不保存修补快照。原始 `Image`、导入和重定位记录始终保持未绑定状态。

镜像窗口、源镜像与目标存储重叠、导出偏移、导入索引、完整修补字段、镜像基址目标偏移和地址加减溢出属于内部不变量，失败报告 `VMExecutableValidationFailed`（`INK-V0002`）并终止进程。重定位范围重叠由 `ExecutableFileValidator` 校验。Loader 只填写地址，不改写调用 opcode；`Calli` 和 `Callm` 在执行时按最终地址区分 stub 与 Ink。

## 镜像基址地址重定位

`executable_file_type.h` 中的 `ImageBaseAddressRelocation` 保存 `ImageOffset`、`TargetOffset` 和 `Addend`，其静态常量 `ByteSize` 为 8。`ImageOffset` 是待填写八字节字段在镜像中的位置；`TargetOffset` 是目标符号相对镜像开头的偏移；`Addend` 是有符号字节偏移。Loader 已实现首次加载绑定，将 `RuntimeImageBase + TargetOffset + Addend` 写入字段，使用 `CodeSegment.data() + ImageBase` 的宿主地址而非 Loader 参数中的代码段偏移。记录不保存符号名称或实际宿主指针，可用于代码或数据中的地址字段。

本表绑定本镜像内的固定目标，不查询 `ExportSymbolMap`，也不受后加载同名导出的符号覆盖影响。代码段每次搬迁后，解释器都按该库的新宿主基址及原始 `TargetOffset`、`Addend` 重算全部本表字段，而不尝试从旧地址或名称反推目标。首次加载与搬迁修补均检查库镜像范围、完整八字节目标字段、`TargetOffset` 范围和地址加减溢出，并遵守实际字段八字节对齐及小端平台的直接写入前置条件。所有写入只发生在运行时代码段，`ExecutableFile::Image` 及原始重定位记录保持未绑定。

普通符号立即数和 `Calli` 统一使用 64 位地址字段。已布局的本镜像符号生成镜像基址地址重定位；前向地址引用先记录在 `PendingStaticRelocationSymbol`，使用 `Register = ZERO`，最终解析生成同类重定位。符号内存地址也使用 ZERO 基址和地址重定位。`Jmpi` 的函数符号目标仍以 `Register = PC` 记录并回填 PC 相对位移，基本块目标保留函数内静态解析。数值 `Calli` 保留完整 u64 地址位模式，不自动生成重定位。

`padGlobals` 只负责全局布局及符号偏移登记，不触发重定位解析。`ExecutableEmitter::emit` 依次执行 `padGlobals`、`padFunction` 后调用 `resolveStaticRelocations`，由 `ExecutableEmissionContext` 统一处理待解析符号记录；此时每个待解析 VM 符号都必须已有镜像偏移，缺少目标时报告内部编译错误，成功后清空临时队列。`padFunction` 在每个函数发射前先补齐到八字节边界，再调用 `Context.resolveSymbol(Function.name(), Context.byteSize())`，登记函数名的入口镜像偏移并移除其未解析符号记录，再按函数、基本块和指令顺序发射字节。布局与静态重定位完成后，`emit` 按显式入口名称及导出列表填写 `EntryOffset` 和 `ExportSymbols`。

结构校验与加载绑定都允许 `TargetOffset` 等于镜像字节数，以表达镜像末尾的 one-past 位置；此时修补字段本身仍须完整位于镜像内。这不表示结果地址可解引用。Loader 检查宿主目标地址与 Addend 的运算不溢出，但不要求加上 Addend 后的结果仍位于镜像内；对象边界和访问权限由后续执行阶段检查。

## 导出符号

`executable_file_type.h` 中的 `ExportSymbol` 保存 `core::Name Name` 和 `std::uint64_t Offset`。Name 是对外可查找的导出名称，Offset 是符号相对 `Image` 开头的字节偏移，不区分函数与数据。每个名称必须在 NamePool 中有效且非空，并在 `ExportSymbols` 内唯一；不同名称可以导出同一个偏移。导入与导出使用独立的身份规则，允许使用相同名称。

Offset 必须小于镜像字节数，不能指向镜像末尾之后的位置。结构校验不检查导出偏移是否为指令入口或对象边界。导出记录由 `emit` 按显式 `ExportNames` 列表生成，选择规则见 [显式入口与导出选择](#显式入口与导出选择)。Loader 在解析本库导入前，将这些名称及真实宿主地址登记到解释器的统一符号表，通过 `Interpreter::resolveSymbolAddress(Name)` 查询；后登记的同名项覆盖表中当前地址，已有库已绑定的引用仍保留原目标。

## 结构校验

`ExecutableFileValidator::validate(const ExecutableFile &, const core::DiagnosticEngine &) noexcept` 返回 `void`，调用方必须传入诊断引擎。校验成功时正常返回；失败时通过 `Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description)` 报告内部编译错误并终止进程，诊断代码为 `INK-V0002`，编号为 `0x04000002`。即使引擎没有 consumer，错误也会同步输出并终止；这一行为不依赖断言或 NDEBUG，在各构建配置中都生效。

错误上下文区分镜像、动态导入、镜像基址地址重定位、动态导入符号重定位和导出符号，并附带记录编号及具体原因。记录编号对应所属 vector 的原始下标。

校验检查入口位于非空镜像内且按八字节对齐、动态导入表可用 u32 下标引用、导入与导出的 Name 均在 NamePool 中有效且非空、导入表内 Name 唯一、导出表内 Name 唯一且 Offset 位于镜像内，以及每条动态导入重定位引用有效、镜像基址重定位的 TargetOffset 不超过镜像字节数、完整的 8 字节字段位于镜像内。两类重定位的所有修补范围共同检查，不能在同一字段上重复绑定。相邻重定位字段有效，两类重定位的 `ImageOffset` 都必须满足八字节对齐；运行时存储及非空镜像基址同样按八字节对齐，保证 Loader 的直接整数槽写入满足要求。

校验不改写或重排镜像和记录，不解析宿主符号，不解码指令，不校验入口或导出偏移的指令边界、调用兼容性或字节码中的动态导入索引，也不根据未知运行时绑定检查最终地址与 Addend 的运算结果。调用方显式调用本镜像校验器；对外部字节码的验证及运行时访存、栈和权限检查仍需由后续加载器、执行器完成。

镜像内存模型、结构校验、逻辑模块链接、全局及函数布局、指令字节发射、显式入口选择和导出生成均已实现。Loader 已提供动态导入和镜像基址地址重定位，解释器在代码段搬迁后修补两类地址字段，共享 runtime 的 `identity` 已提供 Native 桩样例。完整指令执行、可执行文件的磁盘字节格式、Reader/Writer 和通用宿主 ABI adapter 仍待实现；`Image` 不是自动生成的自描述磁盘文件，不能直接写出 C++ 对象内存布局。

当前对齐编码替换了此前的紧排指令格式，旧 raw image 必须重新生成。镜像目前没有编码版本字段；若需要不同编码共存，必须另行增加版本标识或显式转换。指令编码对齐不约束 VM 数据的有效地址，未对齐的数据访问仍由字节复制完成。
