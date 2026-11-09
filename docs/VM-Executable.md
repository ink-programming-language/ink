# VM 可执行镜像模型

`ink/backend/vm/executable` 定义 `ExecutableFile` 的扁平内存模型。镜像将指令、常量及全局数据放在同一个 `std::vector<std::uint8_t>` 中，不再分别持有 ObjectFile 的常量节和全局存储对象。镜像布局由后续链接器确定，`EntryOffset` 相对 `Image` 开头，不是宿主地址或序列化文件偏移。

## 成员和接口

| 成员 | 访问器 | 含义 |
| --- | --- | --- |
| `Image` | `getImage()` | 镜像拥有的原始字节 |
| `EntryOffset` | `getEntryOffset()` | 入口在镜像中的字节偏移 |
| `NativeFunctions` | `getNativeFunctions()` | 按 u32 下标引用的宿主函数导入表 |
| `NativeData` | `getNativeData()` | 加载时需要解析的宿主数据导入表 |
| `NativeDataRelocations` | `getNativeDataRelocations()` | 加载时需要填写的宿主数据地址字段 |

访问器传播 const；容器允许复制和移动，复制后的镜像、导入记录和重定位记录相互独立。导入记录中的 `Name` 是 `core::Name` 句柄，复制后共享进程级 `core::NamePool` 中不可变的名称拼写；重新赋值某个记录的 Name 不会改变其他副本。`byteSize()` 返回镜像字节数。默认构造只创建空容器；空镜像没有有效入口，不能通过校验。

## 宿主符号来源

`native_symbol_provider.h` 定义 `NativeSymbolProvider` 枚举：`Native` 表示宿主注册的 native 符号解析域，`SystemApi` 表示操作系统 API 解析域，默认值为 Native。只有函数导入保存 Provider；它只选择解析来源类别，不表示调用约定或 DLL、共享库名称。

函数和数据使用不同的导入表。函数表内的 `Provider + Name` 必须唯一，数据表内只按 Name 去重；不同函数 Provider 可以使用相同名称，函数表和数据表也允许使用相同名称。两种导入的 Name 类型均为 `core::Name`，默认值 `core::NAME_None` 无效，必须通过进程共享的 `core::NamePool` 登记非空拼写后使用。导入记录保存待解析身份，不保存宿主指针或已绑定的运行时对象。

## 函数导入

`native_function_import.h` 定义 `NativeFunctionImport`，保存 `Provider`、`Name` 和 `CallKind`。`NativeCallKind` 包含 `C` 和 `InkNative`，默认值为 C；Provider 与 CallKind 独立，前者选择符号解析来源，后者描述宿主函数入口的调用类别。同一 Provider、Name 的函数即使 CallKind 不同，也属于重复导入。

函数在 `NativeFunctions` 中的位置就是其 u32 导入索引，供未来的 `callNative` 指令引用。后续加载器解析函数并按 CallKind 绑定到统一的运行时 adapter；宿主函数地址不回填到 `Image`，函数调用也不使用数据重定位表。

CallKind 目前只区分 C 与 InkNative，不包含完整的 C ABI 签名、参数和返回值布局或调用区描述。函数导入通过结构校验不代表已完成调用兼容性校验，也不表示加载器、adapter 或 `callNative` 指令已经实现。

## 数据导入和重定位

`native_data_import.h` 定义 `NativeDataImport`，只保存 `core::Name Name`，不包含 Provider。`native_data_relocation.h` 定义 `NativeDataRelocation`，保存 `ImageOffset`、`SymbolIndex` 和 `Addend`，没有重定位种类字段；其静态常量 `ByteSize` 为 8。

`ImageOffset` 指向待填写字段的第一个字节，而不是所在指令的起点，该字段可以位于代码或数据中。`SymbolIndex` 是 `NativeData` 的 u32 下标，只引用数据导入表。`Addend` 是解析数据地址后施加的有符号字节偏移，最终填写值为 `ResolvedDataAddress + Addend`；例如引用宿主 buffer 的第 8 字节可以使用 `Addend = 8`，只引用符号本身时 Addend 为零。记录存入 vector，不要求按地址排序。

镜像内部的 VM 地址引用由静态链接完成；本表仅记录加载时才能确定的宿主数据地址。消费这些 64 位字段的指令必须明确使用宿主地址空间，现有普通 VM load/store 不会自动获得宿主访存能力。后续编码器和加载器还须约定相应操作数形式，验证目标平台的地址表示能够写入该字段，并检查解析地址与 Addend 相加后的结果；不能将原有 u32 引用字段直接覆盖为 u64 地址。

## 结构校验

`ExecutableFileValidator::validate(const ExecutableFile &, const core::DiagnosticEngine &) noexcept` 返回 `void`，调用方必须传入诊断引擎。校验成功时正常返回；失败时通过 `Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description)` 报告内部编译错误并终止进程，诊断代码为 `INK-V0002`，编号为 `0x04000002`。即使引擎没有 consumer，错误也会同步输出并终止；这一行为不依赖断言或 NDEBUG，在各构建配置中都生效。

错误上下文区分 `image`、`native function table`、`native data table`、`native function import #Index`、`native data import #Index` 和 `native data relocation #Index`，并附带具体原因。记录编号对应原始 vector 下标；重叠错误定位所发现冲突记录对中原始下标较大的记录。

校验检查入口位于非空镜像内、函数和数据表均可用 u32 下标引用、函数的 Provider 与 CallKind 枚举有效、两类 Name 均在 NamePool 中有效且非空、各导入表内符号身份唯一，以及每条数据重定位引用有效、完整的 8 字节字段位于镜像内、所有修补范围互不重叠。相邻字段有效，字段无需对齐。

校验不改写或重排镜像和记录，不解析宿主符号，不解码指令，不校验入口的指令边界或字节码中的函数导入索引，也不根据未知运行时绑定检查最终地址与 Addend 的运算结果。调用方仍须保留并验证镜像布局及权限等执行信息。

本次仅实现内存模型和结构校验。可执行文件的磁盘字节格式、Reader/Writer、Linker、Loader、地址映射、native adapter 和执行器均未由这些类型实现；`Image` 不是自动生成的自描述磁盘文件，不能直接写出 C++ 对象内存布局。
