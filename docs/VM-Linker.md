# VM 模块链接器

`src/include/ink/backend/vm/linker/module_linker.h` 提供 `ModuleLinker::link()`，输入多个只读 `MachineModule`，输出一个独立拥有全部节点的 `MachineModule`。第一版支持强定义、Local/External 链接属性、显式 Native 声明、严格符号解析和深克隆。

```cpp
std::array<const MachineModule *, 2> Inputs{&First, &Second};
auto Linked = ModuleLinker::link(Inputs, Diagnostics, OutputName);
```

返回值为 `std::unique_ptr<MachineModule>`，调用方必须传入 `const core::DiagnosticEngine &Diagnostics`。成功后可立即销毁所有输入；失败由链接器内部报告 `VMModuleLinkFailed`，返回 nullptr，不修改输入，也不返回部分模块。该诊断为普通错误，不使用异常或强制终止。输出名由调用方指定，默认匿名。空输入产生空模块。链接器保留输入顺序及函数、全局变量内部的创建顺序；只读与可写全局变量共用插入顺序，相同输入重复链接会产生相同的文本结果。

## 符号定义与声明

函数和全局变量共用一个 VM 链接名命名域，通过创建接口的 Linkage 参数指定 `MachineSymbolLinkage`：

- `Local` 是默认值，只能由原模块内的名称引用解析到。不同输入模块可以有同名 Local 定义；输出时为 Local 定义生成无冲突名称，同时重写其所属输入中的符号引用。匿名 Local 节点可保留，但无法通过名称引用。
- `External` 是跨模块可见的强定义，要求非空名称；全部输入中同名 External 定义最多一个，不按输入顺序选择胜者。
- `MachineModule::declareSymbol(Name, Kind, IsConstant)` 记录外部声明，Kind 为 `Function` 或 `GlobalVariable`。全局变量通过 IsConstant 属性区分只读和可写；全局声明与定义的常量属性必须精确一致，函数声明不能标记为常量。重复的兼容声明允许存在，但必须找到同名同种类、同属性的 External 定义。声明本身不占用函数体或数据存储，也不能用空函数或零大小数据代替。

模块内同名定义被拒绝，函数和全局之间也不能重名。Local 定义不能与本模块的普通声明或 Native 声明同名。普通声明可与同名同种类、同常量属性的 External 定义并存。

所有输入定义会先被收集，再处理声明和指令引用，因此解析支持前向引用和模块间循环引用。符号地址操作数不要求预先建立普通声明：若本模块没有同名 Local 定义或显式 Native 声明，可以直接找到全体输入中的 External 定义。普通声明用于额外约束目标种类。符号引用在解析阶段按原输入模块的作用域绑定；一个输入声明的 Native 名称不会成为其他输入可见的定义，也不会覆盖其他输入的同名 Local 绑定。

第一版为严格最终链接：所有普通声明和指令中的 VM 符号必须解析成功。即使声明没有被指令使用，也不能留下未定义的普通声明。链接成功后已经解析的普通声明不再作为未解析依赖保留。

直接调用的符号 `ImmediateOperand` 按原输入模块的作用域解析，addend 必须为零。`Calli` 的符号目标只能绑定到 VM 函数定义，支持同模块 Local 目标、跨模块 External 目标和递归引用；`CallNativei` 的符号目标只能绑定到该输入显式声明的 Native 函数。直接符号调用指向全局数据、Native 数据或另一类函数时会报错。

数值直接目标不参与符号绑定：`Calli` 保存 PC 相对 i64 位模式，`CallNativei` 保存统一动态导入表 `DynamicImportSymbols` 的 u32 索引。`Callm`、`CallNativem` 的 `AddressOperand` 定位存放 64 位可调用目标的内存；其中的符号按普通地址符号规则解析，不将指针槽误认成直接函数目标。链接器不检查内存中的实际指针值，也不校验参数和返回值的 ABI 兼容性。

## Native 声明

`declareNativeFunction(LinkName, Import)` 和 `declareNativeData(LinkName, Import)` 分别声明宿主函数和宿主数据。LinkName 是 VM 名称，`Import.Name` 是宿主查找名，两者可以不同；未定义的普通符号不会自动变成 Native。

`NativeFunctionImport` 和 `NativeDataImport` 都只保存宿主 `Name`。链接器分别按 `Import.Name` 合并函数声明和数据声明，保留两种逻辑类别以检查调用目标。宿主函数和宿主数据可以通过不同的 VM 链接名引用同一宿主名称；VM 链接名仍必须能无歧义解析。

各输入必须显式声明自己引用的 Native 符号。兼容的重复声明会合并；多个 VM 名称引用同种类的同一宿主名称时，输出选择统一的 VM 链接名并重写引用。同一 VM 名称指向不同宿主名称、同时声明为 Native 函数和数据、或与 External 定义冲突时会报错。普通外部声明不能同时充当 Native 声明。

输出保留合并后的 Native 声明及逻辑符号引用。`NativeFunctionImport` 和 `NativeDataImport` 位于 `module/native_function_import.h`、`module/native_data_import.h`，声明类别用于逻辑链接的种类检查。可执行镜像使用统一的 `DynamicImportSymbols`，每项只保存宿主 `Name`，函数和数据的同名引用共用导入记录。链接器不解析宿主地址，不生成动态导入索引、动态导入符号地址重定位或导出表，不为普通 VM 内存访问自动增加宿主访存能力；宿主 ABI 调用描述和 adapter 协议仍待实现。

## 深克隆与执行镜像边界

链接分为 `resolveLinkSymbols → cloneLinkedModule` 两个阶段。符号解析器先检查输入、收集定义和 Native 导入、绑定普通声明，再按原模块作用域绑定指令引用，最后分配 Local 输出名称，避免生成名称捕获原本未定义的引用。`LinkBindings` 只保存输入作用域到 SymbolId 的绑定及唯一符号记录，不保存输入模块列表，也不承担节点复制；其内部存储仅由解析器修改，消费者只读查询输出名称和唯一符号记录。定义目标借用输入节点，仅在本次 link 调用内部使用。只有绑定完整有效后才创建输出模块，克隆阶段不再执行符号解析。

克隆阶段创建新函数和全部新基本块，再通过旧基本块对象到新基本块对象的映射重建指令。前向、后向和自跳转均保留目标身份；匿名或重名的基本块不依靠名称匹配。输出的 parent 指针和跳转目标只指向输出对象。

全局变量保留声明大小、初始化字节、对齐、常量属性和插入顺序，未初始化尾部不展开为宿主缓冲区。各函数保留栈槽顺序、大小、对齐以及已布局或未布局状态。符号内存地址和符号立即数都会改写名称，包括直接调用目标与间接调用的指针槽地址。普通地址和立即数的 `i64` addend 完整保留，不因指令位宽截断；直接调用符号的 addend 必须为零。

函数结构校验由内部 `visitFunctionReferences` 遍历负责，检查基本块和指令归属、寄存器和分支目标；遇到符号操作数时，在该操作数的语义位置调用符号解析器。结构校验与名字绑定在同一遍历中按语义顺序处理。指令遍历和重建通过统一的指令 visitor 与 `tryCloneInstruction` 操作数改写接口分发，避免链接器再次维护一套 opcode switch。操作数按目标、来源等语义顺序处理；一旦某个操作数失败，就停止处理该指令剩余操作数，链接调用只报告首个错误。

局部名称改写先在 LinkBindings 中记录原模块的符号绑定，再为 Local 符号生成无冲突输出名称。克隆阶段的操作数改写重载从绑定结果查询目标输出名称，构造新的 AddressOperand 或符号 ImmediateOperand；基本块操作数通过块映射重建，最后由 MachineInstructionBuilder 插入输出指令。调用复用同一套操作数改写接口，数值目标保持原始位模式。源节点及源操作数保持原值，没有通过原位修改输入实现链接。

链接器不决定最终代码或数据地址，不计算基本块跳转的 PC 相对位移，也不选择镜像入口。镜像布局与发射尚未实现，当前链接流水线为：

```text
MachineModule × N → ModuleLinker → MachineModule
```

当前已支持直接调用的函数种类检查及间接调用指针槽的普通符号绑定。普通 VM 符号地址计算、动态导入符号索引分配、地址重定位记录生成、导出选择及偏移计算，以及指令编码与解码尚未实现。机器级函数签名、完整 ABI 校验、调用执行及全局初始化字节内部的符号地址记录仍待补充。全局指针和函数指针表不能仅凭原始初始化字节自动建立符号引用；链接器不会推测某段原始字节是否表示地址。所有输入由调用方保证采用兼容的 VM 与 ABI 配置。weak、COMDAT、部分链接、归档提取和不可达代码剔除也不在第一版范围内。

## 文本格式

[VM Module 文本序列化](VM-Module.md#符号操作数与文本序列化) 保留链接属性和全部声明，仍兼容省略 Local 属性的原有文本。例如一个输入声明并引用另一个输入定义的数据：

```text
.module "consumer"
.declare global @counter
.section .text
.function update external
entry:
    add64mmi [@counter], [@counter], 1
    ret
.endfunction
```

```text
.module "provider"
.section .data
.global counter, 8, 8 external
.endglobal
```

Native 声明使用独立的 VM 名称和宿主名称：

```text
.native_function @vm_log, @host_log
.native_function @vm_api, @host_api
.native_data @vm_buffer, @host_buffer
```

文本读写只保留结构和元数据，不代替链接器的冲突、依赖与引用检查。
