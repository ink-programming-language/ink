# VM 可执行镜像模型

`ink/backend/vm/executable` 定义 `ExecutableFile` 的扁平内存模型。镜像将指令及全局数据放在同一个 `std::vector<std::uint8_t>` 中，不保存代码和数据的段表。已实现的 [ModuleLinker](VM-Linker.md) 仅将多个 `MachineModule` 合并为一个逻辑模块；从逻辑模块布局并生成镜像、动态导入及重定位和导出表尚未实现。`EntryOffset` 和导出符号的 `Offset` 均相对 `Image` 开头，不是宿主地址或序列化文件偏移。

## 成员和接口

| 成员 | 访问器 | 含义 |
| --- | --- | --- |
| `Image` | `getImage()` | 镜像拥有的原始字节 |
| `EntryOffset` | `getEntryOffset()` | 入口在镜像中的字节偏移 |
| `DynamicImportSymbols` | `getDynamicImportSymbols()` | 按 u32 下标引用的动态导入符号表，统一表示函数和数据 |
| `DynamicImportSymbolRelocations` | `getDynamicImportSymbolRelocations()` | 加载时需要填写的动态导入符号地址字段 |
| `ExportSymbols` | `getExportSymbols()` | 镜像导出的符号名称及其镜像内字节偏移 |

访问器传播 const；容器允许复制和移动，复制后的镜像、导入、重定位和导出记录相互独立。符号记录中的 `Name` 是 `core::Name` 句柄，复制后共享进程级 `core::NamePool` 中不可变的名称拼写；重新赋值某个记录的 Name 不会改变其他副本。`byteSize()` 返回镜像字节数。默认构造只创建空容器；空镜像没有有效入口，不能通过校验。

## 字节序和镜像范围

VM 镜像固定使用小端序，操作码、偏移、立即数及动态导入地址重定位字段均按小端序编码，不保存字节序配置。模型只保存全局数据的原始字节，不改写这些字节；完整的指令编码与解码尚未实现。

镜像字节当前由调用方显式构造，模型不会自动布局或发射代码及数据。镜像模型不携带代码范围和数据权限信息；后续加载及执行阶段如何建立指令边界、区分代码与数据和实施权限检查仍待确定。结构校验只要求入口偏移指向镜像中的一个字节。

## 动态导入符号

`dynamic_import_symbol.h` 定义 `DynamicImportSymbol`，只保存 `core::Name Name`，表示待解析的宿主符号名称。记录不区分函数与数据，也不保存解析来源、调用约定或 DLL、共享库名称。

导入表内的 `Name` 必须唯一。同一宿主名称供调用和取地址共同引用，也供函数和数据引用共用，无需因用途不同重复导入。Name 默认为无效的 `core::NAME_None`，必须通过进程共享的 `core::NamePool` 登记非空拼写后使用。导入记录保存待解析名称，不保存宿主指针或已绑定的运行时对象。

符号在 `DynamicImportSymbols` 中的位置就是其 u32 导入索引，供 `CallNativei` 的最终编码及地址重定位引用。该逻辑指令复用 `ImmediateOperand`，接受 u32 数值索引或零 addend 的 VM 链接名；模块链接器仍在逻辑模块层检查符号形式指向显式 Native 函数并统一别名。VM 链接名到镜像导入索引的分配尚未实现。直接调用按索引使用符号，不要求把调用目标地址填写到指令中；同一符号另有取地址用途时，可以生成地址重定位。

`CallNativem` 复用 `AddressOperand`，从其有效地址处读取 64 位 native 可调用指针，而不是读取 u32 导入索引。符号地址引用的是存放指针的内存位置，按普通地址符号规则链接。该指针的运行时构造、有效性验证及 adapter/宿主 ABI 协议尚未实现，不能仅凭现有导入表把任意 C 函数地址当作可调用值。

逻辑模块通过 `module/native_function_import.h` 中的 `NativeFunctionImport` 保存宿主函数名称，通过 `module/native_data_import.h` 中的 `NativeDataImport` 保存宿主数据名称，两个导入类型都只包含 `Name`。模块声明仍区分函数和数据，供逻辑链接检查调用目标种类；镜像中的统一导入表只按宿主名称去重。完整调用签名、参数和返回值布局、调用区描述及宿主 ABI adapter 协议仍待设计，导入记录不提供这些信息。

## 动态导入符号重定位

`dynamic_import_symbol_relocation.h` 定义 `DynamicImportSymbolRelocation`，保存 `ImageOffset`、`SymbolIndex` 和 `Addend`，没有重定位种类字段；其静态常量 `ByteSize` 为 8。

`ImageOffset` 指向待填写字段的第一个字节，而不是所在指令的起点，该字段可以位于代码或数据中。`SymbolIndex` 是 `DynamicImportSymbols` 的 u32 下标，可以引用函数或数据符号。`Addend` 是解析符号地址后施加的有符号字节偏移，最终填写值为 `ResolvedSymbolAddress + Addend`；例如引用宿主 buffer 的第 8 字节可以使用 `Addend = 8`，只引用符号本身时 Addend 为零。记录存入 vector，不要求按地址排序。

本表记录加载时才能确定的动态导入符号地址，不记录镜像内部 VM 地址。`ExecutableEmitter::padOperand` 已实现宿主内存地址及普通宿主符号地址立即数的字段和重定位生成，二者按宿主 `Name` 复用同一导入。内存地址操作数先追加 `ff` 地址基址标记，再追加八字节零占位；普通符号地址立即数只允许 Width 为 64，直接追加八字节零，不带 `ff` 标记。两者的 `ImageOffset` 都指向八字节字段起点，不能将 u32 字段作为地址重定位目标。VM 符号内存地址采用 PC 相对形式，普通 VM 符号立即数采用相对镜像开头的偏移；尚未布局的 VM 符号记录在发射器的 `PendingStaticRelocationSymbol` 中，保留 `PCRelative` 或 `ImageRelative` 种类及字段位宽，静态回填尚未实现。普通寄存器地址不会因某个数值看似宿主指针而自动切换地址空间。函数地址的重定位只负责填写地址，其可调用表示及 adapter 协议仍须单独确定；直接 native 调用的 u32 导入索引编码仍待指令发射层实现。

加载器尚未实现，仍须验证目标平台的地址表示能够写入字段，并检查解析地址与 Addend 相加后的结果，再按小端序填写。执行器也须明确处理宿主地址访问及权限；现有模型不执行宿主访存。

## 导出符号

`export_symbol.h` 定义 `ExportSymbol`，保存 `core::Name Name` 和 `std::uint64_t Offset`。Name 是对外可查找的导出名称，Offset 是符号相对 `Image` 开头的字节偏移，不区分函数与数据。每个名称必须在 NamePool 中有效且非空，并在 `ExportSymbols` 内唯一；不同名称可以导出同一个偏移。导入与导出使用独立的身份规则，允许使用相同名称。

Offset 必须小于镜像字节数，不能指向镜像末尾之后的位置。结构校验不检查导出偏移是否为指令入口或对象边界。导出表由调用方显式构造；逻辑模块的导出选择、布局后的偏移计算及运行时查找尚未实现。

## 结构校验

`ExecutableFileValidator::validate(const ExecutableFile &, const core::DiagnosticEngine &) noexcept` 返回 `void`，调用方必须传入诊断引擎。校验成功时正常返回；失败时通过 `Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description)` 报告内部编译错误并终止进程，诊断代码为 `INK-V0002`，编号为 `0x04000002`。即使引擎没有 consumer，错误也会同步输出并终止；这一行为不依赖断言或 NDEBUG，在各构建配置中都生效。

错误上下文区分 `image`、`dynamic import symbol table`、`dynamic import symbol #Index`、`dynamic import symbol relocation #Index` 和 `export symbol #Index`，并附带具体原因。记录编号对应原始 vector 下标；重叠错误定位所发现冲突记录对中原始下标较大的记录。

校验检查入口位于非空镜像内、动态导入表可用 u32 下标引用、导入与导出的 Name 均在 NamePool 中有效且非空、导入表内 Name 唯一、导出表内 Name 唯一且 Offset 位于镜像内，以及每条重定位引用有效、完整的 8 字节字段位于镜像内、所有修补范围互不重叠。相邻重定位字段有效，字段无需对齐。

校验不改写或重排镜像和记录，不解析宿主符号，不解码指令，不校验入口或导出偏移的指令边界、调用兼容性或字节码中的动态导入索引，也不根据未知运行时绑定检查最终地址与 Addend 的运算结果。调用方显式调用本镜像校验器；对外部字节码的验证及运行时访存、栈和权限检查仍需由后续加载器、执行器完成。

镜像内存模型、结构校验和逻辑模块链接均已实现。指令编码与解码、镜像布局与发射、可执行文件的磁盘字节格式、Reader/Writer、Loader、运行时地址映射、native adapter 和执行器仍待实现；`Image` 不是自动生成的自描述磁盘文件，不能直接写出 C++ 对象内存布局。
