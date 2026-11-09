# VM 内存模型与指令构建

`src/include/ink/backend/vm/module` 提供 lowering 使用的 VM 内存模型。它保存函数、基本块、逻辑指令、常量与全局数据，以及函数的栈帧描述；这些对象不借用上层 IR 节点。

```text
vm::MachineModule
 ├─ 常量与全局数据
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

内存模型不保存 PC 相对位移，因此追加指令或调整块布局不会改变目标身份。后续 `ObjectEmitter` 完成函数和块布局后，根据目标块指针查找代码位置，再计算、编码 PC 相对位移；宿主指针不会写入对象文件。当前尚未实现该发射器。

`isBranch()`、`isConditionalBranch()`、`isReturn()`、`isTerminator()` 提供控制流分类；条件跳转只有一个显式目标块，未跳转时继续执行下一条指令。`readEFlagsMask()` 和 `writtenEFlagsMask()` 描述隐式标志依赖：所有 ADD、CMP 改写 CF、ZF、SF、OF，条件跳转读取各自所需的子集。EFLAGS 不作为它们的显式操作数，也不能用作普通寄存器值或地址基址。`eflags.h` 只提供标志定义、掩码及条件依赖元数据；标志生成和条件判断由后续执行器实现，当前模型不执行指令。

## 常量、全局数据与栈帧

模块通过 `createConstant(Bytes, Alignment, Name)` 创建 `MachineConstant`，通过 `createGlobalVariable(ByteSize, Alignment, InitialBytes, Name)` 创建 `MachineGlobalVariable`。两个数据类型分别定义在 `machine_constant.h` 和 `machine_global_variable.h`，创建接口使用的 `MachineModuleDataError` 定义在 `machine_module.h`。两者返回 `std::expected<std::reference_wrapper<具体类型>, MachineModuleDataError>`，成功后的引用由模块持有。`constants()`、`globalVariables()` 提供保留插入顺序的遍历，数据节点提供只读字节、大小、对齐、名称和父模块查询。

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

## 对象文件边界

`serialization` 目录提供 `writeMachineModuleText()` 和 `readMachineModuleText()`，将完整模块转换为采用 Intel 风格指令、标签和数据伪指令的文本，并从文本恢复独立拥有的模块。函数直接写为 `.function sum`，基本块使用函数内的 `entry:` 等标签；跳转使用 `jne failure`、`jmp done`。读取器支持同一函数内的前向、后向和自引用，拒绝未定义或其他函数中的目标；写出器按目标块对象身份选择标签，匿名或重名节点使用的自动标签及原始名称覆盖不会改变跳转目标。目前为临时实现，后续需要迭代为正式版，见 [TODO](todo.md)。

`vm::MachineModule` 是可编辑逻辑代码模型。现有 `vm::ObjectFile` 保存已经编码的 Code 字节、常量区、全局存储、符号、基址引用和调试记录，`ObjectFileReader`、`ObjectFileWriter` 继续负责文件容器的读取与写出。

`executable` 目录提供链接后可执行镜像的内存模型 `ExecutableFile`，统一保存镜像字节、入口偏移、按索引引用的宿主函数导入、宿主数据导入和数据地址重定位记录，并提供结构校验；接口和约束见 [VM 可执行镜像模型](VM-Executable.md)。该模型尚未实现链接、磁盘读写或宿主绑定。

后续的 `ObjectEmitter` 应当完成模块布局、符号偏移、基本块目标到 PC 相对位移的转换、指令编码与对象内容生成。当前尚未实现该发射器、指令编解码器、lowering pass 或 VM 执行器；创建一个 `vm::MachineModule` 不会自动产生可执行字节。指令字节布局见 [VM 指令编码格式](VM-Instruction-Format.md)，执行状态与调用约定的讨论见 [VM 运行时设计](VM-Runtime.md)。
