# VM 内存模型与指令构建

`src/include/ink/backend/vm/module` 提供 lowering 使用的 VM 内存模型。它保存统一符号声明、函数、基本块、逻辑指令和全局数据，以及函数的栈帧描述；这些对象不借用上层 IR 节点。只读数据由全局变量的常量属性表示。

```text
vm::MachineModule
 ├─ SymbolDeclarations
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

## 无条件跳转

`Jmp` 持有同一函数的 `BasicBlockOperand`。Builder 可直接接收非空目标块指针；`getTarget()` 返回该指针，`getTargetOperand()` 返回块操作数引用。块布局变化不改变目标身份，镜像发射阶段再计算 PC 相对位移。

新增的三种跳转按目标来源使用独立操作数形式：

```cpp
Builder.create<Opcode::Jmpi>(ImmediateOperand(FunctionName));
Builder.create<Opcode::Jmpr>(RegisterOperand(RegisterId::PC, 16));
Builder.create<Opcode::Jmpm>(AddressOperand(RegisterId::BP, 8));
```

`Jmpi` 使用 `I` 形式，立即数是相对当前指令 PC 的有符号 64 位位移；函数符号由链接器绑定为 VM 函数，发射器计算相对位移。`Jmpr` 使用单个 `RegisterOperand` 的 `R` 形式，目标为寄存器值加偏移。`Jmpm` 使用 `M` 形式，从有效地址处读取八字节目标宿主地址；操作数中的符号定位指针槽，按普通地址符号解析。三者的 `getTarget()` 和 `getTargetOperand()` 返回具体操作数的常量引用。

`isBranch()`、`isReturn()`、`isTerminator()` 提供控制流分类；四种跳转均为无条件终结指令。`Load`、`Store`、`Move`、`Arithmetic`、`Bitwise`、`Compare` 和 `Convert` 提供 `getDestination()`，三操作数目标指令的 `getLHS()`、`getRHS()` 分别访问索引 1、2；`Ret` 没有目标或来源操作数。

## 显式比较条件跳转

`ConditionalBranch` 类别同时描述比较来源和跳转目标，不依赖隐式寄存器。`Mmb` 的布局为 `AddressOperand, AddressOperand, BasicBlockOperand`，`Mib` 为 `AddressOperand, ImmediateOperand, BasicBlockOperand`，`Rrb` 为 `RegisterOperand, RegisterOperand, BasicBlockOperand`，`Rib` 为 `RegisterOperand, ImmediateOperand, BasicBlockOperand`。比较关系与有符号属性分别记录在 `compare_branch.def` 与 `register_compare_branch.def`，操作码经 `opcode.def` 接入同一套强类型模型（`InstructionDomain::Integer`）。

```cpp
Builder.create<Opcode::Je64mib>(AddressOperand(RegisterId::BP, 16), ImmediateOperand(42), BasicBlockOperand(&Done));
Builder.create<Opcode::Jlt32mmb>(AddressOperand(RegisterId::BP, 16), AddressOperand(RegisterId::BP, 24), BasicBlockOperand(&Less));
```

支持相等、不等、有符号及无符号的四种大小关系，每种提供 8/16/32/64 位和两种来源形式。`getLHS()`、`getRHS()` 访问索引 0、1；`getTarget()` 返回最后一个操作数中的块指针，`getTargetOperand()` 返回该块操作数引用。单块指针构造便捷接口仅用于原有 `Jmp`，比较分支显式传入三个操作数。Builder 检查目标非空并且与插入点属于同一函数，克隆与文本读写也保留最后一个目标块的身份。

`isBranch()` 包含无条件和条件跳转，`isConditionalBranch()` 仅描述后者的分类。条件成立时跳目标块，不成立时执行下一条指令；它们不写来源值，不产生比较结果槽或隐式状态。需要表达两个显式去向时，可在其后追加普通 `jmp`。

## 函数调用指令

`Calli` 使用单个 `ImmediateOperand`，`Callm` 使用单个 `AddressOperand`，`Callr` 使用单个 `RegisterOperand`，均属于 `InstructionCategory::Call` 与 `InstructionDomain::None`，文本统一为 `call`。`getTarget()` 和 `getTargetOperand()` 返回具体目标操作数的常量引用。

```cpp
Builder.create<Opcode::Calli>(ImmediateOperand(FunctionName));
Builder.create<Opcode::Callm>(AddressOperand(RegisterId::BP, 16));
```

`isCall()` 识别调用。调用没有显式结果目标或参数操作数，不是终结指令。`Calli` 数值目标是完整 u64 地址，符号目标可绑定 VM 函数定义或尚待加载解析的函数声明，符号 addend 必须为零。`Callm` 从内存读取完整函数地址，其符号只定位指针槽。

发射时本镜像符号地址生成镜像基址重定位，外部符号地址生成动态导入重定位，调用目标也使用这些地址记录。解释器已实现 `Calli`、`Callm` 共用的目标地址分派及普通 CALL/RET；调用区自动生成和完整签名校验仍待实现。参数与返回槽布局见 [VM 调用约定](VM-Calling-Convention.md)。

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

`AddressOperand(core::Name Symbol, std::int64_t ByteOffset)` 表示符号相对的内存地址，ADD、Load、Store 的 `m` 来源读取该地址处的值。`ImmediateOperand(core::Name Symbol, std::int64_t ByteOffset)` 则表示符号地址本身加上偏移，作为 `i` 值参与指令；它与 `ImmediateOperand(std::uint64_t Bits)` 的数值形式由 `ImmediateValueKind::Symbol` 和 `ImmediateValueKind::Numeric` 区分。`getValueKind()` 返回该分类，数值形式的 `bits()` 返回 `std::optional<std::uint64_t>`，符号形式的 `getSymbol()` 和 `getByteOffset()` 分别返回名称与有符号偏移；不匹配的访问器返回空值。

符号名称通过 `core::Name` 保存链接名，不借用输入文本，也不是宿主指针。函数和全局变量分别保存 `MachineSymbolLinkage::Local` 或 `External`，创建接口的 Linkage 参数指定该属性，默认 Local。`MachineModule::declareSymbol(Name, Kind, IsConstant)` 统一保存符号声明，通过 `symbolDeclarations()` 只读遍历。Kind 为 Function 或 GlobalVariable，IsConstant 默认 false，只能在全局变量声明上设为 true；链接时声明与定义的种类和常量属性必须精确一致。模块内存模型本身不执行链接校验。

导入候选由 `ImportSymbols = SymbolDeclarations - (Functions + Globals)` 推导。声明先尝试绑定本模块或本次链接中可见的 VM 定义；未满足的声明留待加载时解析，可以由其他 Ink 库或 Native 提供，不能提前认定为 Native。函数和数据共用符号声明和动态导入表，每个名称同时用于逻辑引用和加载查找。

`serialization` 目录提供 `writeMachineModuleText()` 和 `readMachineModuleText()`，将完整模块转换为采用 Intel 风格指令、标签和数据伪指令的文本，并从文本恢复独立拥有的模块。函数直接写为 `.function sum`，基本块使用函数内的 `entry:` 等标签；当节点为匿名、重名或包含非标识符字节时，写出器分配不冲突的文本标签并在其后紧跟引号包裹的原始名称覆盖（如 `.global .G0 "", 0, 1`、`.function sum.1 "sum"`、`entry.1: "entry"`）。基本块跳转使用 `jmp done`；其他跳转显式写为 `jmpi`、`jmpr`、`jmpm`。读取器支持同一函数内的前向、后向和自引用，拒绝未定义或其他函数中的目标；写出器按目标块对象身份选择标签，匿名或重名节点使用的自动标签及原始名称覆盖不会改变跳转目标。目前为临时实现，后续需要迭代为正式版，见 [TODO](todo.md)。

`RegisterOperand(RegisterId Register, std::int64_t ByteOffset = 0)` 保存寄存器编号和有符号字节偏移，通过 `getRegister()` 和 `getByteOffset()` 查询。来源文本 `pc + 2` 表示寄存器值加 2，`[pc + 2]` 则表示该地址处的内存；文本读写和指令克隆保留完整的 `i64` 偏移。寄存器来源和地址基址接受 BP、SP、PC、ZERO 及 R0–R9；ZERO 编码为 `03`、文本拼写为 `zero`，恒为零且不可写。R0–R9 编码为 `04`–`0d`，保存任意 64 位位模式。寄存器目标接受 BP、SP 和 R0–R9，且偏移必须为零，PC 与 ZERO 均不能作为普通写入目标。新增寄存器沿用现有八位编号字段，不改变 opcode、头部或操作数槽的编码格式。

符号操作数使用 `@name` 或带转义名称的 `@"name"`，以区分寄存器、数值和函数内块标签。`[@counter + 8]` 表示读取或写入符号相对地址的内存，`@counter + 8` 表示地址值。例如 `add64mmi [bp + 16], [@counter], 1` 使用 counter 内存中的值，`add64mii [bp + 16], @counter, 1` 使用 counter 的地址值。符号偏移完整保留为 `i64`，文本读写不会根据指令位宽截断符号偏移；名称必须在 NamePool 中有效且非空。

调用文本统一使用 `call`，由操作数决定直接或间接形式，例如 `call @function_name`、`call [bp + 16]` 和 `call [@pointer_slot + 8]`。名称支持转义，直接符号调用拒绝非零 addend。`callNative` 不再是合法助记符；函数来源由实际目标地址在执行时确定。

定义行末尾的 `external` 保留外部链接属性，例如 `.function sum external` 和 `.global counter, 8, 8 external`。全局变量的格式为 `.global name, size, alignment [readonly] [local|external]`，`readonly` 表示 IsConstant，例如 `.global message, 3, 1 readonly external`。全部全局变量统一写在 `.section .data` 中，文本不再提供 `.rodata`、`.constant` 或 `.endconstant`。省略链接属性或使用 `local` 表示 Local；写出器省略 Local 属性。External 定义必须具有非空原始名称，自动显示标签不替代链接名。顶层声明语法如下，名称同样支持带转义的 `@"name"`：

```text
.declare function @other_function
.declare global @other_global
.declare global @other_readonly_global readonly
.declare function @host_function
.declare function @host_bridge
.declare global @host_buffer
```

全部声明都使用 `.declare`，不再提供独立的 Native 声明语法。声明中的种类和只读属性保留给逻辑链接检查。文本读取只恢复字段、名称和结构，不执行链接，也不决定哪些声明最终由 VM 定义满足。发射器从声明与定义的差集建立统一外部导入符号集合，被指令使用的符号按名称加入镜像的 `DynamicImportSymbols`；未使用的声明不会自动生成动态导入项。完整宿主 ABI 调用描述及 adapter 协议仍待实现。

## 链接与可执行镜像边界

已实现的模块链接流水线为 `MachineModule × N → ModuleLinker → MachineModule`。`linker` 目录的 `ModuleLinker` 支持强符号、Local/External、统一声明、外部导入符号推导、种类检查和深克隆，具体接口与冲突规则见 [VM 模块链接器](VM-Linker.md)。发射器已提供全局及函数布局、基本块目标到 PC 相对位移的转换、指令编码和重定位处理。符号内存地址统一选择恒零的 ZERO 基址；本镜像符号地址通过镜像基址重定位填写，外部符号地址通过动态导入重定位填写，普通符号立即数和直接调用目标使用相同规则。进程内的 Name 句柄不能进入字节流。

`executable` 目录提供可执行镜像的内存模型 `ExecutableFile`，保存小端镜像字节、入口偏移、统一动态导入符号表 `DynamicImportSymbols`、动态导入符号地址重定位 `DynamicImportSymbolRelocations`、镜像基址地址重定位 `ImageBaseAddressRelocations`，以及记录导出名称和镜像内偏移的 `ExportSymbols`，并提供结构校验；模型不保存段表或字节序配置，接口和约束见 [VM 可执行镜像模型](VM-Executable.md)。`ModuleLinker` 输出仍是逻辑模块，不生成上述镜像内容。

当前已实现镜像发射、加载及当前指令集的执行。lowering pass、公共解码器、镜像磁盘读写和通用 native adapter 仍待实现。创建和链接逻辑模块后须显式调用发射器生成指令字节。编码方案见 [VM 指令编码格式讨论稿](VM-Instruction-Format.md)，运行时规划见 [VM 运行时设计](VM-Runtime.md)，参数、返回槽和可变参数调用预留区布局约定见 [VM 调用约定](VM-Calling-Convention.md)。
