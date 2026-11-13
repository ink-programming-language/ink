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

VM 镜像固定使用小端序，操作码、偏移、立即数及两类地址重定位字段均按小端序编码，不保存字节序配置。模型保存全局数据的原始字节；地址字段的加载绑定由重定位记录描述，指令字节编码已实现，解码尚未实现。

镜像字节可由调用方显式构造，或通过发射器当前提供的全局布局与操作数字段接口追加；镜像模型本身不执行布局。`ExecutableEmitter::getExecutable()` 借用当前构建中的未绑定镜像，只提供 const 访问；后续发射仍可能修改其内容。镜像模型不携带代码范围和数据权限信息；后续加载及执行阶段如何建立指令边界、区分代码与数据和实施权限检查仍待确定。结构校验只要求入口偏移指向镜像中的一个字节。

`ExecutableEmitter` 拥有 `ExecutableEmissionContext`，上下文保存镜像、模块符号、统一的 `NativeSymbols` 和待解析符号重定位，并提供字节追加及回填接口。扫描时先登记模块中的函数与全局定义，再从 `SymbolDeclarations` 排除这些定义，得到 `NativeSymbols = SymbolDeclarations - (Functions + Globals)`；符号只有一个名称，同时作为逻辑引用名和宿主查找名。`FunctionEmitter` 拥有保存基本块偏移状态的 `FunctionEmissionContext`；`InstructionEmitter` 借用两个上下文，负责指令和操作数字段编码。函数发射器重置和扫描基本块状态后，按函数内顺序调用 `padBasicBlock`；该接口在 `FunctionEmissionContext` 中登记当前镜像偏移并移除对应的未解析记录，再按块内顺序调用 `InstructionEmitter::padInstruction`。基本块 `padOperand` 已实现九字节 PC 相对目标字段，前向引用记录在 `FunctionEmissionContext::PendingStaticRelocationBasicBlock`，由函数发射结束后的 `FunctionEmitter::resolveBasicBlockStaticRelocations` 统一回填并清空；`reset` 也清空该队列。直接调用目标编码已实现，调用区生成、执行及 native adapter 仍待实现。

## 运行时地址与未绑定镜像

运行时地址统一使用 64 位宿主地址，BP、SP、PC、数据指针和 VM 函数入口采用同一地址坐标。Loader 参数 `ImageBase` 是镜像在代码段中的字节偏移，运行时镜像宿主基址则是 `CodeSegment.data() + ImageBase` 的地址，记为 `RuntimeImageBase`。VM 符号地址为 `RuntimeImageBase + SymbolOffset + Addend`，其中 `SymbolOffset` 是符号相对整个 `Image` 开头的字节偏移；Native 符号地址为绑定后的宿主符号地址加 Addend。PC 相对寻址仍可保留，`PC = RuntimeImageBase + InstructionOffset`，因此 `PC + (SymbolOffset + Addend - InstructionOffset)` 得到相同的运行时地址。ZERO 寄存器编码为 `04`、恒为零且不可写，绝对地址编码为 ZERO 基址加完整的八字节地址位模式。内存地址统一为 `RegisterId:u8 + OffsetBits:u64`，执行器可按 `Registers[RegisterId] + OffsetBits` 计算有效地址，无需按 PC 相对或绝对寻址分支；地址合法性检查仍须完成。

`ExecutableFile` 保存未绑定、与进程无关的镜像，地址字段不保存加载后的宿主指针。解释器使用 `malloc` 管理独立的运行时代码段及其有效字节数、容量；`getCodeSegment()` 返回有效字节的只读 span，`getCodeSegmentCapacity()` 返回容量。加载器在运行时代码段中绑定地址，原始镜像与元数据始终保持不变；入口执行仍待实现，其宿主地址应为 `RuntimeImageBase + EntryOffset`，导出地址为 `RuntimeImageBase + Export.Offset`。不能将已绑定指针缓存作为可执行文件内容保存。统一地址表示不改变 VM 调用与 Native 调用的 ABI 区别，native adapter 仍须处理调用协议。

代码段容量不足时优先 `realloc`，失败后 fallback 到 `malloc`、复制既有有效字节并释放旧存储。无论是哪条路径，只要地址发生变化，解释器都会更新统一符号表中的旧代码段地址，并修补既有库的全部动态重定位字段。修补先从旧绑定值去掉 Addend，依据原符号基地址判断是否属于旧代码段，再换算新地址并加回 Addend；Native 地址及旧引用已经选择的同名符号目标保持不变，不按当前名称重新绑定。运行时代码段搬迁不改写任何 `ExecutableFile`。外部持有的 span 或裸地址不在修补范围内，后续加载尝试即使最终失败，也可能已完成搬迁；调用方应重新取得 view 或查询地址。执行期间的其他地址稳定性仍须由后续执行器保证。

## 动态导入符号

`executable_file_type.h` 中的 `DynamicImportSymbol` 只保存 `core::Name Name`，表示待运行时解析的符号名称。当前统一符号表既可提供 Native 地址，也可提供已登记的 VM 导出地址。记录不区分函数与数据，也不保存解析来源、调用约定或 DLL、共享库名称。

导入表内的 `Name` 必须唯一。同一名称供调用和取地址共同引用，无需因用途不同重复导入。导入表本身不区分函数和数据；逻辑模块中的同名声明仍须通过种类和只读属性的一致性检查。Name 默认为无效的 `core::NAME_None`，必须通过进程共享的 `core::NamePool` 登记非空拼写后使用。导入记录保存待解析名称，不保存宿主指针或已绑定的运行时对象。动态导入项按指令的实际使用生成，未使用的未定义声明不会自动加入此表。

符号在 `DynamicImportSymbols` 中的位置就是其 u32 导入索引，供 `CallNativei` 及地址重定位引用。该逻辑指令复用 `ImmediateOperand`，接受 u32 数值索引或零 addend 的符号名；模块链接器在逻辑模块层检查该符号是当前输入声明且未被 VM 定义满足的函数。`InstructionEmitter` 通过 `ExecutableEmissionContext::getOrAddNativeSymbol` 按同一名称复用导入并写入 u32 索引，与普通地址取用共享导入；数值目标按原 32 位写入，u32 合法性由模块链接器检查。指令连同 opcode 共六字节，不生成动态地址重定位。直接调用按索引使用符号，不要求把调用目标地址填写到指令中；同一符号另有取地址用途时，可以生成地址重定位。

`CallNativem` 复用 `AddressOperand`，从其有效地址处读取 64 位 native 可调用指针，而不是读取 u32 导入索引。符号地址引用的是存放指针的内存位置，按普通地址符号规则链接。该指针的运行时构造、有效性验证及 adapter/宿主 ABI 协议尚未实现，不能仅凭现有导入表把任意 C 函数地址当作可调用值。

逻辑模块使用 `MachineSymbolDeclaration` 统一保存声明名称、函数或数据种类以及只读属性，不保存独立的 Native 函数或数据列表，也不提供不同的 VM 名与宿主名。未由 VM 定义满足的声明形成 Native 符号集合；镜像中的统一导入表只保存名称并按名称去重。完整调用签名、参数和返回值布局、调用区描述及宿主 ABI adapter 协议仍待设计，导入记录不提供这些信息。

## 动态导入符号重定位

`executable_file_type.h` 中的 `DynamicImportSymbolRelocation` 保存 `ImageOffset`、`SymbolIndex` 和 `Addend`，没有重定位种类字段；其静态常量 `ByteSize` 为 8。

`ImageOffset` 指向待填写字段的第一个字节，而不是所在指令的起点，该字段可以位于代码或数据中。`SymbolIndex` 是 `DynamicImportSymbols` 的 u32 下标，可以引用函数或数据符号。`Addend` 是解析符号地址后施加的有符号字节偏移，最终填写值为 `ResolvedSymbolAddress + Addend`；例如引用宿主 buffer 的第 8 字节可以使用 `Addend = 8`，只引用符号本身时 Addend 为零。记录存入 vector，不要求按地址排序。

本表记录加载时才能确定的动态导入符号地址，镜像内部 VM 地址使用下节的镜像基址地址重定位。`InstructionEmitter::padOperand` 已实现宿主内存地址及普通宿主符号地址立即数的字段和重定位生成，二者按宿主 `Name` 复用同一导入。内存地址操作数先追加 ZERO 寄存器编码 `04`，再追加八字节零占位；普通符号地址立即数只允许 Width 为 64，直接追加八字节零，不带寄存器编号。加载后的内存地址字段保存完整 `u64` 地址位模式，与恒零基址相加即可访问；`ff` 不再是合法基址。两者的 `ImageOffset` 都指向八字节字段起点，不能将 u32 字段作为地址重定位目标。函数地址的重定位只负责填写地址，其可调用表示及 adapter 协议仍须单独确定；直接 native 调用已写入 u32 导入索引，不生成地址重定位。

`Interpreter` 持有统一的 `ExportSymbolMap`，以名称映射到 `uint64_t` 表示的真实宿主地址。构造时自动调用公开的 `initialize()`，通过 `try_emplace` 登记 `NativeSymbolAddresses`；重复初始化不会覆盖已有绑定。`Interpreter::resolveSymbolAddress(Name)` 仅查询此表，缺少名称时返回 nullptr。内置 `identity` 对应 `identityStub` 的地址，沿用 [VM 调用约定](VM-Calling-Convention.md) 中的 `Frame *` 桩接口。

`Loader::loadLibrary(CodeSegment, ImageBase, Executable)` 接收可写 span，其中 `ImageBase` 是镜像在 `CodeSegment` 中的字节偏移。加载器先遍历本库的 `ExportSymbols`，校验偏移后，将 `CodeSegment.data() + ImageBase + Export.Offset` 的真实宿主地址写入统一符号表，同名项覆盖既有绑定。随后遍历完整的 `DynamicImportSymbols`，每项通过 `Interpreter::resolveSymbolAddress(Name)` 查询一次，按导入索引保存结果；即使没有重定位引用，导入项也必须能解析，本库导出也可供本库导入使用。最后遍历重定位，仅按 `SymbolIndex` 读取已解析地址，计算 `ResolvedSymbolAddress + Addend`，通过目标字段上的 `uint64_t *` 直接写入，不建立修补数组。

直接写入要求小端宿主，Loader 使用平台字节序宏在编译时拒绝大端平台。实际目标地址 `CodeSegment.data() + ImageBase + ImageOffset` 必须满足 8 字节对齐，并通过 `check` 验证；关闭断言不放宽此前置条件。加载不自动对镜像或库间边界插入 padding，发射器仍采用当前紧排字段格式，因此结构校验合法的重定位仍可能不满足加载时的对齐要求。

`Interpreter` 持有 `DiagnosticEngine`，可通过 `getDiagnostics()` 注册诊断 consumer。找不到导入地址时报告 `VMLibraryLinkFailed`（`INK-V0004`）并返回 `false`。容量增长和既有地址修补发生在加载事务之前；事务恢复本次登记前的符号表、有效代码段长度及 handle，删除新增库记录，但已完成的容量增长和地址搬迁保留。既有有效字节中的非地址内容保持不变，既有地址绑定与当前存储一致，不保证失败后恢复原指针、容量或地址字段的原位模式。直接调用 Loader 的调用方自行负责回滚代码段逻辑状态及符号表；Loader 不保存修补快照。原始 `Image`、导入和重定位记录始终保持未绑定状态。

镜像窗口、源镜像与目标存储重叠、导出偏移、导入索引、完整修补字段和地址加减溢出属于内部不变量，失败通过 `REPORT_ICE_IF` 报告 `VMExecutableValidationFailed`（`INK-V0002`）并终止进程，不作为普通链接失败返回，也不依赖异常展开或事务析构恢复。重定位范围重叠由上游 `ExecutableFileValidator` 校验。镜像基址地址重定位和 `CallNativei` 索引的执行绑定仍待实现。执行器仍须处理宿主地址访问及权限；本步骤不执行宿主访存，也不验证任意 Addend 结果的可解引用性。

## 镜像基址地址重定位

`executable_file_type.h` 中的 `ImageBaseAddressRelocation` 保存 `ImageOffset`、`TargetOffset` 和 `Addend`，其静态常量 `ByteSize` 为 8。`ImageOffset` 是待填写八字节字段在镜像中的位置；`TargetOffset` 是目标符号相对镜像开头的偏移；`Addend` 是有符号字节偏移。后续绑定应填写 `RuntimeImageBase + TargetOffset + Addend`，使用镜像的宿主基址而非 Loader 参数中的代码段偏移；本表的加载绑定尚未实现。记录不保存符号名称或实际宿主指针，可用于代码或数据中的地址字段。

普通 VM 符号立即数统一要求 Width 为 64。已布局符号也追加八字节零并生成本表记录，不能直接把镜像偏移当作完整指针值写入。未布局符号先进入发射器的 `PendingStaticRelocationSymbol`，通过 `Register` 决定最终处理方式。普通符号取地址立即数的临时记录使用 `Register = ZERO`，地址字段本身不追加寄存器编号；寄存器相对访存引用则记录实际基址寄存器及当前指令的镜像偏移 `InstructionOffset`。最终解析时，PC 引用按 `InstructionOffset` 求差并回填位移，ZERO 引用生成 `ImageBaseAddressRelocation`，等待后续加载绑定填写 `RuntimeImageBase + TargetOffset + Addend`。当前普通 VM 符号访存选择 PC 基址。直接 `Calli` 的前向符号目标也复用 `PendingStaticRelocationSymbol`，记录 `Register = PC` 和当前 opcode 偏移，但指令只预留八字节位移字段，不追加寄存器字节、不生成加载地址重定位；最终回填 `FunctionOffset - InstructionOffset`。数值目标保留原 64 位位模式，整条指令十字节。

`padGlobals` 只负责全局布局及符号偏移登记，不触发重定位解析。`ExecutableEmitter::emit` 依次执行 `padGlobals`、`padFunction` 后调用 `resolveStaticRelocations`，由 `ExecutableEmissionContext` 统一处理待解析符号记录；此时每个待解析 VM 符号都必须已有镜像偏移，缺少目标时报告内部编译错误，成功后清空临时队列。`padFunction` 在每个函数发射前调用 `Context.resolveSymbol(Function.name(), Context.byteSize())`，登记函数名的入口镜像偏移并移除其未解析符号记录，再按函数、基本块和指令顺序发射字节。布局与静态重定位完成后，`emit` 按显式入口名称及导出列表填写 `EntryOffset` 和 `ExportSymbols`。

结构校验允许 `TargetOffset` 等于镜像字节数，以表达镜像末尾位置；这不表示该地址可解引用。最终地址与 Addend 的运算、对象边界和访问权限由后续加载及执行阶段检查。

## 导出符号

`executable_file_type.h` 中的 `ExportSymbol` 保存 `core::Name Name` 和 `std::uint64_t Offset`。Name 是对外可查找的导出名称，Offset 是符号相对 `Image` 开头的字节偏移，不区分函数与数据。每个名称必须在 NamePool 中有效且非空，并在 `ExportSymbols` 内唯一；不同名称可以导出同一个偏移。导入与导出使用独立的身份规则，允许使用相同名称。

Offset 必须小于镜像字节数，不能指向镜像末尾之后的位置。结构校验不检查导出偏移是否为指令入口或对象边界。导出记录由 `emit` 按显式 `ExportNames` 列表生成，选择规则见 [显式入口与导出选择](#显式入口与导出选择)。Loader 在解析本库导入前，将这些名称及真实宿主地址登记到解释器的统一符号表，通过 `Interpreter::resolveSymbolAddress(Name)` 查询；后登记的同名项覆盖表中当前地址，已有库已绑定的引用仍保留原目标。

## 结构校验

`ExecutableFileValidator::validate(const ExecutableFile &, const core::DiagnosticEngine &) noexcept` 返回 `void`，调用方必须传入诊断引擎。校验成功时正常返回；失败时通过 `Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description)` 报告内部编译错误并终止进程，诊断代码为 `INK-V0002`，编号为 `0x04000002`。即使引擎没有 consumer，错误也会同步输出并终止；这一行为不依赖断言或 NDEBUG，在各构建配置中都生效。

错误上下文区分镜像、动态导入、镜像基址地址重定位、动态导入符号重定位和导出符号，并附带记录编号及具体原因。记录编号对应所属 vector 的原始下标。

校验检查入口位于非空镜像内、动态导入表可用 u32 下标引用、导入与导出的 Name 均在 NamePool 中有效且非空、导入表内 Name 唯一、导出表内 Name 唯一且 Offset 位于镜像内，以及每条动态导入重定位引用有效、镜像基址重定位的 TargetOffset 不超过镜像字节数、完整的 8 字节字段位于镜像内。两类重定位的所有修补范围共同检查，不能在同一字段上重复绑定。相邻重定位字段有效，结构校验不要求字段对齐；当前 Loader 的直接八字节写入另有实际目标地址 8 字节对齐的前置条件，结构校验成功不保证该条件成立。

校验不改写或重排镜像和记录，不解析宿主符号，不解码指令，不校验入口或导出偏移的指令边界、调用兼容性或字节码中的动态导入索引，也不根据未知运行时绑定检查最终地址与 Addend 的运算结果。调用方显式调用本镜像校验器；对外部字节码的验证及运行时访存、栈和权限检查仍需由后续加载器、执行器完成。

镜像内存模型、结构校验、逻辑模块链接、全局及函数布局、指令字节发射、显式入口选择和导出生成均已实现。Loader 已提供动态导入地址重定位，共享 runtime 的 `identity` 已提供 Native 桩样例。解码、可执行文件的磁盘字节格式、Reader/Writer、镜像基址地址重定位、完整 Native 调用绑定和执行器仍待实现；`Image` 不是自动生成的自描述磁盘文件，不能直接写出 C++ 对象内存布局。
