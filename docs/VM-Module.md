# VM 内存模型与指令构建

`src/include/ink/backend/vm/module` 提供 lowering 使用的 VM 内存模型。它保存函数、基本块、逻辑指令和全局数据，以及函数的栈帧描述；这些对象不借用上层 IR 节点。只读数据由全局变量的常量属性表示。

```text
vm::MachineModule
 ├─ 全局数据（可写或只读）
 └─ vm::MachineFunction
     ├─ MachineFrameInfo
     └─ vm::MachineBasicBlock
         └─ vm::MachineInstruction
```

`MachineModule` 拥有函数，`MachineFunction` 拥有基本块，`MachineBasicBlock` 拥有指令。`MachineInstructionBuilder` 只借用插入位置，不拥有指令。Builder 销毁后，已创建的指令继续存活，直到所属基本块销毁。

## 创建与插入指令

```cpp
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"

namespace ink::vm
{
  void buildExample(MachineModule &ModuleValue)
  {
    MachineFunction &FunctionValue = ModuleValue.createFunction();
    MachineBasicBlock &Entry = FunctionValue.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry);
    MachineAdd32mmiInstruction &Add = Builder.create<Opcode::Add32mmi>(AddressOperand(RegisterId::BP, 20), AddressOperand(RegisterId::BP, 16), ImmediateOperand(5));
    static_cast<void>(Add);
  }
} // namespace ink::vm
```

示例中的 BP 偏移是调用方提供的示意布局，不规定参数、保存区或局部变量的固定 ABI。`create<Opcode>()` 按 `OpcodeTraits` 在编译期约束操作数种类和数量，按值保存操作数，并返回所属基本块持有的具体指令引用。

`setInsertPoint(Block)` 选择块尾；`setInsertPoint(Before)` 选择一条已归属指令之前的位置。后者返回 `bool`，未归属的指令会被拒绝，原有插入位置保持不变。连续在同一锚点前创建指令时，新指令按照创建顺序排列，锚点保持不变。

`getInsertBlock()` 返回所选基本块；`getInsertBefore()` 返回锚点，块尾或未设置插入点时返回 `nullptr`。`clearInsertPoint()` 清除所选位置。`tryCreate<Opcode>()` 在没有插入点，或跳转目标为空、未归属函数、属于其他函数时返回 `nullptr`；`create<Opcode>()` 要求已设置有效插入点，且跳转目标与插入块属于同一非空函数，违反此前提会报告致命错误。

当前接口不提供删除或转移指令、基本块和函数的操作。追加函数、追加基本块和插入指令保留现有对象地址。Builder、父指针和遍历范围均借用所属对象，使用期间相应容器必须继续存活。模块、函数和基本块禁止复制和移动，避免父指针和借用引用失效。

基类 `MachineInstruction` 没有虚析构函数；基本块通过记录具体类型的删除器释放指令。调用方使用 Builder 返回的引用或指针进行访问，由所属基本块负责销毁。`parent()` 从指令到基本块、从基本块到函数、从函数到模块提供归属关系；常量对象访问会传播为常量父对象指针。脱离所属容器单独构造的对象，其父指针为空。

## 比较、跳转与隐式状态

`Cmp8mm`、`Cmp8mi` 至 `Cmp64mm`、`Cmp64mi`，以及 `Cmp64rr`、`Cmp64rm`、`Cmp64ri`、`Cmp64mr` 持有两个来源，不产生显式结果；它们的 `getLHS()`、`getRHS()` 分别访问索引 0、1。`getDestination()` 仅用于 Load、Store、Add，零操作数 `Ret` 不提供目标或来源访问器。

跳转在内存中保存目标基本块指针。`BasicBlockOperand` 是持有 `const MachineBasicBlock *` 的操作数包装，`getTarget()` 直接返回该指针，`getTargetOperand()` 返回 `const BasicBlockOperand &` 以便统一遍历。Builder 可直接接收基本块指针，要求目标非空，且源块与目标块属于同一函数；允许前向、后向和自跳转。目标指针借用所属函数中的稳定块对象，函数必须覆盖跳转指令的使用期。

```cpp
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"

namespace ink::vm
{
  void buildBranchExample(MachineModule &ModuleValue)
  {
    MachineFunction &FunctionValue = ModuleValue.createFunction();
    MachineBasicBlock &Entry = FunctionValue.createBasicBlock();
    MachineBasicBlock &Failure = FunctionValue.createBasicBlock();
    MachineBasicBlock &Done = FunctionValue.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry);
    Builder.create<Opcode::Cmp32mi>(AddressOperand(RegisterId::BP, 20), ImmediateOperand(42));
    Builder.create<Opcode::Jne>(&Failure);
    Builder.create<Opcode::Jmp>(&Done);
    Builder.setInsertPoint(Failure);
    Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP, 20), ImmediateOperand(0));
    Builder.setInsertPoint(Done);
    Builder.create<Opcode::Ret>();
  }
} // namespace ink::vm
```

内存模型不保存 PC 相对位移，因此追加指令或调整块布局不会改变目标身份。后续镜像生成阶段需要完成函数和块布局，再根据目标块指针计算、编码 PC 相对位移；宿主指针不能写入可执行镜像。当前尚未实现镜像布局与发射。

`isBranch()`、`isConditionalBranch()`、`isReturn()`、`isTerminator()` 提供控制流分类；条件跳转只有一个显式目标块，未跳转时继续执行下一条指令。`readEFlagsMask()` 和 `writtenEFlagsMask()` 描述隐式标志依赖：所有 ADD、CMP 改写 CF、ZF、SF、OF，条件跳转读取各自所需的子集。EFLAGS 不作为它们的显式操作数，也不能用作普通寄存器值或地址基址。`eflags.h` 只提供标志定义、掩码及条件依赖元数据；标志生成和条件判断由后续执行器实现，当前模型不执行指令。

## 函数调用指令

`Calli`、`CallNativei` 使用持有单个 `ImmediateOperand` 的 `I` 形式；`Callm`、`CallNativem` 使用持有单个 `AddressOperand` 的 `M` 形式。四者均属于 `InstructionCategory::Call`，复用现有操作数类型。`getTarget()` 和 `getTargetOperand()` 都返回具体目标操作数的常量引用。

```cpp
Builder.create<Opcode::Calli>(ImmediateOperand(FunctionName));
Builder.create<Opcode::CallNativei>(ImmediateOperand(NativeFunctionName));
Builder.create<Opcode::Callm>(AddressOperand(RegisterId::BP, 16));
Builder.create<Opcode::CallNativem>(AddressOperand(PointerSlotName, 8));
```

`isCall()` 识别全部调用，`isNativeCall()` 识别两种 Native 调用。调用没有显式结果目标或参数操作数，也不是终结指令；返回后继续执行后继指令。四者的 `readEFlagsMask()` 为零，`writtenEFlagsMask()` 为 `ArithmeticEFlagsMask`，表示调用后不能依赖原先的算术标志。

`Calli` 的数值目标为相对当前 PC 的有符号 i64 位模式，符号目标绑定 VM 函数定义；`CallNativei` 的数值目标为统一动态导入表 `DynamicImportSymbols` 的 u32 索引，符号目标绑定显式 Native 函数声明。直接调用的符号 addend 必须为零。`Callm` 和 `CallNativem` 分别从有效地址读取 64 位 VM 函数地址和 native 可调用指针，后者不是导入索引。间接目标中的符号只定位存放指针的内存，按普通地址符号规则解析。

Builder 不解析名称或校验调用签名。逻辑指令、文本和链接接入均已实现；指令编码与解码尚未实现。参数与返回槽的布局约定见 [VM 调用约定](VM-Calling-Convention.md)，其自动代码生成和调用执行尚未实现。native 可调用指针的 adapter 与宿主 ABI 协议也仍待确定。

## 全局数据与栈帧

模块通过 `createGlobalVariable(ByteSize, Alignment, InitialBytes, Name, Linkage, IsConstant)` 创建 `MachineGlobalVariable`，类型定义在 `machine_global_variable.h`。最后两个参数默认分别为 `MachineSymbolLinkage::Local` 和 `false`，`IsConstant = true` 表示只读全局变量，通过 `isConstant()` 查询。创建接口返回 `std::expected<std::reference_wrapper<MachineGlobalVariable>, MachineModuleDataError>`，成功后的引用由模块持有。`globalVariables()` 按创建顺序统一遍历可写与只读全局变量，不按常量属性重新排列；数据节点提供初始化字节、大小、对齐、名称和父模块查询。

创建时复制调用方提供的数据，不保留输入 span 的借用。对齐必须是非零的二次幂，全局数据的初始化长度不能超过声明大小；无效对齐、初始化过长和不能表示的大小通过 `MachineModuleDataError` 报告。全局对象声明大小可以大于初始化字节数，模块不会为了表示未初始化尾部而分配相同大小的宿主内存。

`MachineFunction::frameInfo()` 返回函数拥有的 `MachineFrameInfo`。`createStackSlot(ByteSize, Alignment)` 创建抽象栈槽，返回 `std::expected<std::reference_wrapper<const MachineStackSlot>, MachineFrameLayoutError>`。栈槽记录大小和对齐，`stackSlots()` 提供只读遍历；尚未布局时，槽位的 `byteOffset()` 及帧的 `baseOffset()`、`byteSize()` 均为空。

`layout(BaseOffset)` 根据调用方提供的起始偏移显式计算槽位偏移和帧大小，检查对齐与无符号地址计算溢出。成功后，`byteSize()` 表示最终末端减去 `BaseOffset`，包含中间的对齐填充，不包含起始偏移之前的区域，也不自动补齐末尾。计算失败保留之前成功的布局；添加新栈槽会使现有布局失效。`MachineFrameLayoutError` 包含 `InvalidAlignment` 和 `SizeOverflow`。

`MaximumAlignment` 缓存所有栈槽对齐要求的最大值，通过 `alignment()` 导出；空帧的值为 `1`。例如一个槽要求 4 字节对齐，另一个要求 8 字节对齐，帧的最大对齐需求就是 `8`。当前布局算法逐个使用各槽的对齐要求，不直接使用这个最大值给整个帧末尾取整，也不会据此调整运行时 BP 或 SP。后续栈帧代码生成需要结合 ABI 和这个需求，保证参考基址及调用栈满足所需对齐；只有槽偏移对齐，不能保证参考基址未对齐时最终地址也对齐。

`BaseOffset` 记录最后一次成功布局的起始偏移，通过 `baseOffset()` 查询；它不是运行时 BP 的寄存器值。例如调用方选择 BP 作为参考基点，并把前 16 字节留给保存区时，可以调用 `layout(16)`，让当前局部存储区域从偏移 16 开始安排。申请下面两个槽后，结果为：

| 栈槽 | 大小 | 对齐 | 布局后偏移 |
| --- | --- | --- | --- |
| A | 4 字节 | 4 字节 | 16 |
| B | 8 字节 | 8 字节 | 24 |

此时 `MaximumAlignment = 8`、`BaseOffset = 16`，布局末端为 `32`，`byteSize() = 32 - 16 = 16`。A 占 `[16, 20)`，中间有 4 字节填充，B 占 `[24, 32)`；前缀 `[0, 16)` 不计入这个局部区域的大小。这些都是编译期布局记录，调用 `layout()` 不会分配 VM 栈，也不会自动生成调整 SP 的指令。

本模型不固定调用保存区大小，也不隐式预留 BP/PC 保存项。布局得到的是无符号字节偏移；将其用于指令的有符号 `ByteOffset` 时，上层仍须检查能否表示为 `i64`，并明确该布局基点与 BP 的关系。

## 符号操作数与文本序列化

`AddressOperand(core::Name Symbol, std::int64_t ByteOffset)` 表示符号相对的内存地址，ADD、CMP、Load、Store 的 `m` 来源读取该地址处的值。`ImmediateOperand(core::Name Symbol, std::int64_t ByteOffset)` 则表示符号地址本身加上偏移，作为 `i` 值参与指令；它与 `ImmediateOperand(std::uint64_t Bits)` 的数值形式由 `ImmediateValueKind::Symbol` 和 `ImmediateValueKind::Numeric` 区分。`getValueKind()` 返回该分类，数值形式的 `bits()` 返回 `std::optional<std::uint64_t>`，符号形式的 `getSymbol()` 和 `getByteOffset()` 分别返回名称与有符号偏移；不匹配的访问器返回空值。

符号名称通过 `core::Name` 保存链接名，不借用输入文本，也不是宿主指针。函数和全局变量分别保存 `MachineSymbolLinkage::Local` 或 `External`，创建接口的 Linkage 参数指定该属性，默认 Local。`MachineModule::declareSymbol(Name, Kind, IsConstant)` 保存带种类的外部声明，Kind 为 Function 或 GlobalVariable，IsConstant 默认 false，只能在全局变量声明上设为 true。链接时全局声明与定义的常量属性必须精确一致。`declareNativeFunction(Name, Import)` 和 `declareNativeData(Name, Import)` 保存显式 Native 声明，其中 Name 是 VM 链接名，Import.Name 是宿主查找名。上述声明可以通过 `symbolDeclarations()`、`nativeFunctions()`、`nativeData()` 的只读视图遍历，模块内存模型本身不执行链接校验。

`serialization` 目录提供 `writeMachineModuleText()` 和 `readMachineModuleText()`，将完整模块转换为采用 Intel 风格指令、标签和数据伪指令的文本，并从文本恢复独立拥有的模块。函数直接写为 `.function sum`，基本块使用函数内的 `entry:` 等标签；跳转使用 `jne failure`、`jmp done`。读取器支持同一函数内的前向、后向和自引用，拒绝未定义或其他函数中的目标；写出器按目标块对象身份选择标签，匿名或重名节点使用的自动标签及原始名称覆盖不会改变跳转目标。目前为临时实现，后续需要迭代为正式版，见 [TODO](todo.md)。

`RegisterOperand(RegisterId Register, std::int64_t ByteOffset = 0)` 保存寄存器编号和有符号字节偏移，通过 `getRegister()` 和 `getByteOffset()` 查询。来源文本 `pc + 2` 表示寄存器值加 2，`[pc + 2]` 则表示该地址处的内存；文本读写和指令克隆保留完整的 `i64` 偏移。寄存器来源接受 BP、SP、PC，寄存器目标仅接受 BP、SP 且偏移必须为零。

符号操作数使用 `@name` 或带转义名称的 `@"name"`，以区分寄存器、数值和函数内块标签。`[@counter + 8]` 表示读取或写入符号相对地址的内存，`@counter + 8` 表示地址值。例如 `add64mmi [bp + 16], [@counter], 1` 使用 counter 内存中的值，`add64mii [bp + 16], @counter, 1` 使用 counter 的地址值。符号偏移完整保留为 `i64`，文本读写不会根据指令位宽截断符号偏移；名称必须在 NamePool 中有效且非空。

调用文本共用 `call` 和 `callNative` 名称，由操作数决定具体形式。例如 `call @function_name` 和 `callNative 0` 分别是符号直接调用和按 native 导入索引调用；`call [bp + 16]` 和 `callNative [@pointer_slot + 8]` 则从内存读取可调用目标。名称支持转义，直接调用拒绝 `call @function_name + 8` 等非零符号偏移；符号是否存在、直接调用是否属于正确类别由链接器检查。

定义行末尾的 `external` 保留外部链接属性，例如 `.function sum external` 和 `.global counter, 8, 8 external`。全局变量的格式为 `.global name, size, alignment [readonly] [local|external]`，`readonly` 表示 IsConstant，例如 `.global message, 3, 1 readonly external`。全部全局变量统一写在 `.section .data` 中，文本不再提供 `.rodata`、`.constant` 或 `.endconstant`。省略链接属性或使用 `local` 表示 Local；写出器省略 Local 属性。External 定义必须具有非空原始名称，自动显示标签不替代链接名。顶层声明语法如下，名称同样支持带转义的 `@"name"`：

```text
.declare function @other_function
.declare global @other_global
.declare global @other_readonly_global readonly
.native_function @vm_function, @host_function
.native_function @system_function, @host_bridge
.native_data @vm_buffer, @host_buffer
```

Native 函数和数据声明都只指定 VM 链接名与宿主名称。对应的 `NativeFunctionImport` 和 `NativeDataImport` 定义在 `module/native_function_import.h`、`module/native_data_import.h`，各自只保存宿主 `Name`；声明类别保留给逻辑链接的种类检查。镜像中的 `DynamicImportSymbols` 统一表示函数和数据，每项只保存宿主 `Name` 并按名称去重；完整宿主 ABI 调用描述及 adapter 协议仍待实现。文本读取只检查字段、名称和结构，不把未解析符号自动归类为 Native，也不执行模块链接。

## 链接与可执行镜像边界

已实现的模块链接流水线为 `MachineModule × N → ModuleLinker → MachineModule`。`linker` 目录的 `ModuleLinker` 支持强符号、Local/External、显式 Native、严格解析和深克隆，具体接口与冲突规则见 [VM 模块链接器](VM-Linker.md)。代码和数据的最终布局、符号地址计算、基本块目标到 PC 相对位移的转换及镜像发射尚未实现。按既有编码约定，VM 符号内存地址使用 PC 相对地址，普通 VM 符号立即数使用相对镜像开头的字节偏移；进程内的 Name 句柄不能进入字节流。

`executable` 目录提供可执行镜像的内存模型 `ExecutableFile`，保存镜像字节、入口偏移、字节序、统一动态导入符号表 `DynamicImportSymbols`、动态导入符号地址重定位 `DynamicImportSymbolRelocations`，以及记录导出名称和镜像内偏移的 `ExportSymbols`，并提供结构校验；模型不保存段表，接口和约束见 [VM 可执行镜像模型](VM-Executable.md)。`ModuleLinker` 输出仍是逻辑模块，不生成上述镜像内容。

当前尚未实现 lowering pass、指令编码与解码、镜像布局与发射、镜像磁盘读写、加载器、native adapter 或 VM 执行器；创建和链接逻辑模块不会生成指令字节。编码方案见 [VM 指令编码格式讨论稿](VM-Instruction-Format.md)，运行时规划见 [VM 运行时设计](VM-Runtime.md)，参数、返回槽和可变参数调用预留区布局约定见 [VM 调用约定](VM-Calling-Convention.md)。
