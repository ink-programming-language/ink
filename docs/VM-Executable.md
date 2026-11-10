# VM 可执行镜像模型

`ink/backend/vm/executable` 定义 `ExecutableFile` 的扁平内存模型。镜像将指令及只读、可写全局数据放在同一个 `std::vector<std::uint8_t>` 中。已实现的 [ModuleLinker](VM-Linker.md) 将多个 `MachineModule` 合并为一个模块，再由 [ExecutableEmitter](VM-Emitter.md) 统一布局并生成镜像、段信息及导入和重定位表。`EntryOffset` 相对 `Image` 开头，不是宿主地址或序列化文件偏移。

## 成员和接口

| 成员 | 访问器 | 含义 |
| --- | --- | --- |
| `Image` | `getImage()` | 镜像拥有的原始字节 |
| `EntryOffset` | `getEntryOffset()` | 入口在镜像中的字节偏移 |
| `Order` | `getByteOrder()` | 指令字段的 `vm::ByteOrder`，默认为 Little |
| `Sections` | `getSections()` | 代码、只读数据及可写数据的范围 |
| `NativeFunctions` | `getNativeFunctions()` | 按 u32 下标引用的宿主函数导入表 |
| `NativeData` | `getNativeData()` | 加载时需要解析的宿主数据导入表 |
| `NativeDataRelocations` | `getNativeDataRelocations()` | 加载时需要填写的宿主数据地址字段 |

访问器传播 const；容器允许复制和移动，复制后的镜像、导入记录和重定位记录相互独立。导入记录中的 `Name` 是 `core::Name` 句柄，复制后共享进程级 `core::NamePool` 中不可变的名称拼写；重新赋值某个记录的 Name 不会改变其他副本。`byteSize()` 返回镜像字节数。默认构造只创建空容器；空镜像没有有效入口，不能通过校验。

## 字节序和段范围

`bytecode/byte_order.h` 定义 `vm::ByteOrder::Little` 和 `Big`；发射器默认选择小端序，编码和解码所有多字节指令字段时显式使用这一选择。全局变量的原始初始化字节保持调用方提供的顺序，不由发射器根据字节序改写。

`executable_section.h` 定义 `ExecutableSection`，保存 `Kind`、`ImageOffset` 和 `ByteSize`。`ExecutableSectionKind` 包含 `Code`、`ReadOnlyData` 和 `WritableData`。存在段表时，所有段必须非空、按镜像顺序连续且互不重叠，完整覆盖 `Image`；入口须位于 Code 段。段内的数据对齐填充属于该段，段边界本身不要求满足某个固定对齐。后续加载器可据此只解码 Code，并区分只读及可写数据。

发射器始终生成非空段表：代码从偏移零开始，按函数和基本块原顺序连续编码，随后分别布局只读、可写全局数据；同一数据组内保持原有相对顺序，并按每个对象的 alignment 布局。对齐间隙和未初始化尾部填零。未产生字节的段不加入表。手动构造的旧镜像可以保留空段表，结构校验维持兼容；这种镜像不包含可供加载器推断代码和数据权限的完整布局信息。

## 宿主符号来源

`native_symbol_provider.h` 定义 `NativeSymbolProvider` 枚举：`Native` 表示宿主注册的 native 符号解析域，`SystemApi` 表示操作系统 API 解析域，默认值为 Native。只有函数导入保存 Provider；它只选择解析来源类别，不表示调用约定或 DLL、共享库名称。

函数和数据使用不同的导入表。函数表内的 `Provider + Name` 必须唯一，数据表内只按 Name 去重；不同函数 Provider 可以使用相同名称，函数表和数据表也允许使用相同名称。两种导入的 Name 类型均为 `core::Name`，默认值 `core::NAME_None` 无效，必须通过进程共享的 `core::NamePool` 登记非空拼写后使用。导入记录保存待解析身份，不保存宿主指针或已绑定的运行时对象。

## 函数导入

`native_function_import.h` 定义 `NativeFunctionImport`，保存 `Provider`、`Name` 和 `CallKind`。`NativeCallKind` 包含 `C` 和 `InkNative`，默认值为 C；Provider 与 CallKind 独立，前者选择符号解析来源，后者描述宿主函数入口的调用类别。同一 Provider、Name 的函数即使 CallKind 不同，也属于重复导入。

函数在 `NativeFunctions` 中的位置就是其 u32 导入索引，供 `CallNativei` 的最终编码引用。该逻辑指令复用 `ImmediateOperand`，接受 u32 数值索引或零 addend 的 VM 链接名；模块链接器检查符号形式指向显式 Native 函数并统一别名。发射器生成符号导入索引并直接编码数值索引；索引合法性由发射前的模块 validator 保证，后续加载器解析函数并按 CallKind 绑定到统一的运行时 adapter。按索引导入的宿主函数地址不回填到 `Image`，函数绑定也不使用数据重定位表。

`CallNativem` 复用 `AddressOperand`，从其有效地址处读取 64 位 native 可调用指针，而不是读取 u32 导入索引。符号地址引用的是存放指针的内存位置，按普通地址符号规则链接。该指针的运行时构造、有效性验证及 adapter/宿主 ABI 协议尚未实现，不能仅凭现有导入表把任意 C 函数地址当作可调用值。

CallKind 目前只区分 C 与 InkNative，不包含完整的 C ABI 签名、参数和返回值布局或调用区描述。`callNative` 的直接和间接形式已提供逻辑指令、文本读写、链接接入及字节编解码；加载器、adapter 和调用执行仍未实现。Native 函数符号除了用于直接 `callNative` 导入索引外，暂不能发射为地址立即数或内存地址；发射器报告 `INK-V0004`，不推测 adapter 指针协议。函数导入通过结构校验不代表已完成调用兼容性校验。

## 数据导入和重定位

`native_data_import.h` 定义 `NativeDataImport`，只保存 `core::Name Name`，不包含 Provider。`native_data_relocation.h` 定义 `NativeDataRelocation`，保存 `ImageOffset`、`SymbolIndex` 和 `Addend`，没有重定位种类字段；其静态常量 `ByteSize` 为 8。

`ImageOffset` 指向待填写字段的第一个字节，而不是所在指令的起点，该字段可以位于代码或数据中。`SymbolIndex` 是 `NativeData` 的 u32 下标，只引用数据导入表。`Addend` 是解析数据地址后施加的有符号字节偏移，最终填写值为 `ResolvedDataAddress + Addend`；例如引用宿主 buffer 的第 8 字节可以使用 `Addend = 8`，只引用符号本身时 Addend 为零。记录存入 vector，不要求按地址排序。

镜像内部的 VM 地址引用由发射器按静态布局完成；本表仅记录加载时才能确定的宿主数据地址。符号内存操作数引用宿主数据时，发射器写出 `ff` 地址基址标记和初始为零的八字节字段，并生成指向该字段的数据重定位。宿主数据地址立即数也生成八字节字段和重定位，要求指令立即数宽度为 64 位；不能将 u32 字段作为地址重定位目标。VM 符号内存地址仍使用 PC 相对形式，普通寄存器地址不会因某个数值看似宿主指针而自动切换地址空间。

加载器尚未实现，仍须验证目标平台的地址表示能够写入字段，并检查解析地址与 Addend 相加后的结果，再按镜像字节序填写。执行器也须明确处理宿主地址访问及权限；已有的发射和编解码并不执行宿主访存。

## 结构校验

`ExecutableFileValidator::validate(const ExecutableFile &, const core::DiagnosticEngine &) noexcept` 返回 `void`，调用方必须传入诊断引擎。校验成功时正常返回；失败时通过 `Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description)` 报告内部编译错误并终止进程，诊断代码为 `INK-V0002`，编号为 `0x04000002`。即使引擎没有 consumer，错误也会同步输出并终止；这一行为不依赖断言或 NDEBUG，在各构建配置中都生效。

错误上下文区分 `image`、`section table`、`section #Index`、`native function table`、`native data table`、`native function import #Index`、`native data import #Index` 和 `native data relocation #Index`，并附带具体原因。记录编号对应原始 vector 下标；重叠错误定位所发现冲突记录对中原始下标较大的记录。

校验检查入口位于非空镜像内、字节序枚举有效、可选段表的种类和完整覆盖及入口位于代码段、函数和数据表均可用 u32 下标引用、函数的 Provider 与 CallKind 枚举有效、两类 Name 均在 NamePool 中有效且非空、各导入表内符号身份唯一，以及每条数据重定位引用有效、完整的 8 字节字段位于镜像内、所有修补范围互不重叠。相邻重定位字段有效，字段无需对齐。

校验不改写或重排镜像和记录，不解析宿主符号，不解码指令，不校验入口的指令边界或字节码中的函数导入索引，也不根据未知运行时绑定检查最终地址与 Addend 的运算结果。发射器假定输入模块已通过独立 validator，不检查静态目标的合法性，也不自动调用本镜像校验器；需要时由调用方单独校验输出。对外部字节码的验证及运行时访存、栈和权限检查仍由后续加载器、执行器负责。

镜像内存模型、结构校验、逻辑模块链接、可执行镜像发射和单条指令编解码均已实现。可执行文件的磁盘字节格式、Reader/Writer、Loader、运行时地址映射、native adapter 和执行器仍待实现；`Image` 不是自动生成的自描述磁盘文件，不能直接写出 C++ 对象内存布局。
