# VM 指令编码格式讨论稿

本文保留 Ink VM 指令及操作数的编码方案讨论，不表示已有编码器或解码器。当前实现只包含逻辑指令、模块链接、可执行镜像模型与结构校验；指令编码与解码均尚未实现。操作码来自 `src/include/ink/backend/vm/instruction/opcode.def`，寄存器编号来自同目录的 `register.def`。`VMInstructionSetVersion` 的默认值为 `1.2.0`，可通过 `INK_VM_INSTRUCTION_SET_VERSION` 配置；这一版本新增 `call` 和 `callNative` 逻辑操作码。符号地址操作数在逻辑模型中保留链接名和偏移，按镜像布局将其转换为最终数值字段的阶段尚未实现。

当前实现提供 `Operand` 基类以及 `RegisterOperand`、`AddressOperand`、`ImmediateOperand`、`BasicBlockOperand` 四种逻辑操作数，并通过强类型指令布局将操作数接入 `MachineInstruction`。基本块操作数在内存中持有目标块指针；调用复用立即数和地址操作数，分别表达直接目标和存放间接目标的内存位置。[VM 内存模型与指令构建](VM-Module.md) 提供模块、函数、基本块的所有权结构和 `MachineInstructionBuilder`，供 lowering 构建逻辑指令。[模块链接器](VM-Linker.md) 已实现逻辑模块合并、符号解析和深克隆，`executable` 目录提供镜像模型与结构校验。镜像布局与发射、加载、预解码缓存和执行器仍待实现。

## 通用布局

```text
Opcode:u16 | Operand0 | Operand1 | ...
```

`u8`、`u16`、`u32`、`u64` 分别表示相应位宽的无符号整数，`i64` 表示 64 位二进制补码。下文的字节序列、操作码和寄存器编号均以十六进制表示，位宽及长度以十进制表示。方案中的多字节字段遵循镜像采用的字节序，包括操作码、偏移和立即数。`vm::ByteOrder::Little` 和 `Big` 定义于 `executable/byte_order.h`，`ExecutableFile::getByteOrder()` 用于保存镜像字节序，模型默认为小端序。磁盘格式仍待约定。方案中的字段连续排列，没有隐式填充、对齐字节、单条指令长度字段或操作数长度字段，不使用 C++ 类或结构体的宿主内存布局。

操作码唯一确定操作数的顺序、种类和立即数位宽。有显式结果目标的指令将目标放在最前；加法随后依次是左来源和右来源。比较只有左、右来源，跳转只有目标基本块，调用只有目标来源，返回没有显式操作数。`r`、`m`、`i` 分别表示寄存器值、内存地址和立即数；`b` 表示内存模型中的目标基本块。已约定的字节布局不附加逻辑操作数的种类标签，也不保存基本块宿主指针或进程内的名称句柄。

| `opcode.def` 中的形式 | 操作数顺序 |
| --- | --- |
| `Mm` | `m, m`；加载、存储为目标和来源，比较为左、右来源 |
| `Mi` | `m, i` |
| `Mmm` | `m, m, m` |
| `Mmi` | `m, m, i` |
| `Mii` | `m, i, i` |
| `Rrr` | `r, r, r` |
| `Rrm` | `r, r, m` |
| `Rri` | `r, r, i` |
| `Mrr` | `m, r, r` |
| `Mrm` | `m, r, m` |
| `Mri` | `m, r, i` |
| `Rr` | `r, r`，比较的左、右来源 |
| `Rm` | `r, m` |
| `Ri` | `r, i` |
| `Mr` | `m, r` |
| `B` | `b`，内存模型中的目标基本块 |
| `I` | `i`，直接调用目标 |
| `M` | `m`，存放间接调用目标的内存地址 |
| `None` | 无显式操作数 |

操作数计数包含所有显式操作数；EFLAGS 等隐式读写不计入其中。指令长度由操作码与各字段宽度确定，符号解析不会改变指令长度。

## 寄存器操作数 `r`

```text
RegisterId:u8
```

寄存器操作数恰好占一个字节，直接引用 VM 寄存器的值。

| 编码 | 寄存器 |
| --- | --- |
| `00` | `BP` |
| `01` | `SP` |
| `02` | `PC` |
| `03` | `EFLAGS`，仅用于标识隐式状态 |

`r` 直接使用寄存器编号。当前普通寄存器来源仅接受 BP、SP、PC，寄存器加法的目标仅接受 BP、SP。EFLAGS 是单独的 32 位状态寄存器，由算术、比较及条件跳转隐式读写，不编码成这些指令的显式 `r` 操作数，也不能用作内存地址基址。

## 地址操作数 `m`

地址操作数保存寄存器或符号链接名，以及有符号字节偏移。加载、存储、加法、比较和间接调用使用它定位内存中的值，访问位宽由操作码决定。符号形式通过 `core::Name` 保存名称，例如文本 `[@counter + 8]`。读取符号地址本身时使用立即数形式 `@counter + 8`。跳转的内存模型保存基本块指针，后续镜像生成阶段需要将其转换为 PC 相对地址字段；该编码地址用于控制转移，不读取该地址处的内存内容。

| 基址来源 | 字节布局 | 长度 |
| --- | --- | --- |
| 寄存器 | `RegisterId:u8, ByteOffset:i64` | 9 字节 |
| 宿主数据地址 | `Base:u8 = ff, NativeAddress:u64` | 9 字节 |

寄存器地址只接受 BP、SP、PC，EFLAGS 和未知编号必须拒绝。`ByteOffset` 在这一形式中占八字节，表示相对寄存器基址的有符号字节位移。

按本文方案，VM 符号内存地址 `Symbol + ByteOffset` 需转换为相对当前指令起始 PC 的有符号位移，使用 `PC = 02` 基址及八字节偏移编码。宿主数据符号使用 `ff` 地址基址标记；后续八字节字段由 `NativeDataRelocation` 记录其位置、导入索引及 addend，留待加载器绑定。上述编码、逻辑符号转换和重定位记录生成尚未实现。`ff` 是方案中的地址基址标记，不是普通寄存器编号。Native 函数符号作为内存地址时的可调用 adapter 指针协议尚未确定。进程内的 `core::Name` 索引不能进入指令字节。

下面的竖线只用于说明字段边界，不属于字节流：

| 地址 | 小端序 | 大端序 |
| --- | --- | --- |
| `[bp + 16]` | `00 \| 10 00 00 00 00 00 00 00` | `00 \| 00 00 00 00 00 00 00 10` |
| `[sp - 8]` | `01 \| f8 ff ff ff ff ff ff ff` | `01 \| ff ff ff ff ff ff ff f8` |

## 立即数操作数 `i`

逻辑立即数支持数值和符号地址两种形式。`ImmediateValueKind::Numeric` 保存完整的 `u64` 位模式；`ImmediateValueKind::Symbol` 保存 `core::Name` 链接名和有符号 `i64` 字节偏移。数值立即数的最终编码没有自身的类型标签或长度字段，`opcode.def` 的 `Width` 决定字段宽度：

| `Width` | 字节数 |
| --- | --- |
| `8` | 1 |
| `16` | 2 |
| `32` | 4 |
| `64` | 8 |

`ImmediateOperand::getValueKind()` 区分两种形式。`bits()` 只在数值形式返回位模式；`getSymbol()` 和 `getByteOffset()` 只在符号形式返回名称与偏移，各访问器用 `std::optional` 表示不适用的情况。负数值按相应位宽的二进制补码表示。当前寄存器目标的 64 位加法将右来源解释为有符号 64 位字节位移；普通整数结果的加法仍遵循操作码规定的位宽回绕语义。

符号立即数表示 `AddressOf(Symbol) + ByteOffset`，不读取该地址处的内存。文本采用 `@name` 或 `@"escaped name"`，并可附加 `+ Offset` 或 `- Offset`。例如 `add64mii [bp + 16], @counter + 8, 1` 的左来源是 counter 地址加 8。文本读写保留完整的名称和 `i64` 偏移。后续镜像生成阶段需要将普通 VM 符号地址转换为相对镜像开头的无符号字节偏移，并检查 addend 运算及立即数字段范围，不能通过截断保留地址低位。宿主数据地址立即数需要使用 64 位字段及数据地址重定位，不能自动作为普通 VM 地址使用；Native 函数地址的 adapter 指针协议尚未确定。

按本文方案，普通数值立即数保留目标位宽的低位，负数按相应位宽的补码编码；`CallNativei` 的数值索引必须适合 u32 字段。符号地址的范围检查依赖最终镜像布局，不沿用普通数值截断规则。这些编码和范围检查尚未实现。

直接调用复用 `ImmediateOperand`，但按调用操作码解释目标：`Calli` 的立即数是 PC 相对位移或待解析的 VM 函数名，`CallNativei` 的立即数是 u32 导入索引或待解析的 Native 函数名。调用符号不允许非零 addend，具体规则见下文。

## 完整指令示例

`add64rri sp, sp, 16` 的操作码是 `001b`，其布局为：

```text
Opcode:u16 | Destination:RegisterId:u8 | LHS:RegisterId:u8 | RHS:i64
```

该指令总共占 12 字节：

```text
小端序：1b 00 | 01 | 01 | 10 00 00 00 00 00 00 00
大端序：00 1b | 01 | 01 | 00 00 00 00 00 00 00 10
```

将右来源改为 `-16` 时，立即数字节为小端序 `f0 ff ff ff ff ff ff ff` 或大端序 `ff ff ff ff ff ff ff f0`。

## 算术标志与比较

`eflags.h` 定义 `EFlags = std::uint32_t` 和四个算术标志的掩码，`EFlag` 的位号沿用 x86。其余位保留为零，当前不实现 PF、AF 或系统控制位。

| 标志 | 位号 | 含义 |
| --- | --- | --- |
| `CF` | 0 | 加法产生无符号进位；比较减法产生无符号借位 |
| `ZF` | 6 | 截断到指令位宽的结果为零 |
| `SF` | 7 | 截断结果的最高位 |
| `OF` | 11 | 按二进制补码解释时发生有符号溢出 |

所有 `add` 及 `cmp` 的指令元数据声明隐式覆盖这四位，条件跳转通过 `conditionEFlagsMask()` 声明所读取的标志子集。`eflags.h` 只提供标志定义和依赖元数据，不计算运算结果、标志值或分支条件；这些执行语义由后续执行器实现。

`cmp` 按 `LHS - RHS` 更新标志，不写入减法结果，也没有显式目标。8、16、32、64 位各有 `cmpWmm`、`cmpWmi`；64 位另有 `cmp64rr`、`cmp64rm`、`cmp64ri`、`cmp64mr`。来源顺序不能交换。操作码范围为 `001f` 至 `002a`，普通整数比较不区分有符号或无符号操作码；后续条件码决定如何解释标志。

寄存器目标的 `add64` 成功后，同样按两个来源的 64 位位模式更新标志。其右来源仍解释为有符号地址增量，地址或栈检查使用独立的数学地址计算；CF 不作为该检查是否失败的判据。检查失败时，目标寄存器和 EFLAGS 都保持原值。`load`、`store`、无条件跳转和 `ret` 保持 EFLAGS 不变。

## 基本块目标编码与返回

所有跳转的内存表示统一保存 `const MachineBasicBlock *`。`BasicBlockOperand` 提供操作数包装，`getTarget()` 直接返回目标指针，`getTargetOperand()` 返回包装引用。Builder 可直接接收目标块指针，要求目标非空，且源块、目标块属于同一函数。文本写为 `jne failure`、`jmp done`，支持函数内的前向、后向和自引用；数值 PC 相对地址不作为逻辑跳转操作数接受。

后续镜像生成阶段需要先完成代码布局，再根据目标块指针计算代码位置，并将目标转换为相对当前指令起始 PC 的字节位移，沿用通用寄存器地址编码：

```text
Opcode:u16 | RegisterId:u8 = 02 | ByteOffset:i64
TargetPC = PC + ByteOffset
```

按本文方案，每条跳转编码占 11 字节，位移为有符号 `i64` 字节数并遵循镜像字节序。执行约定为：条件不满足时继续执行 `NextPC = PC + 11`，满足时转移到 `TargetPC`；零位移表示跳回当前指令。条件跳转只读取所需标志，不修改标志。指令编码与解码、独立模块校验、镜像中的指令边界校验、最终位移计算及运行时控制转移检查仍待实现。

内存中的跳转目标随基本块对象保持稳定，指令插入或块布局变化不要求调用方手工维护位移。文本写出根据目标块对象身份选择标签，读取将函数内标签解析回对应指针；匿名和重名块通过独立标签保留精确身份。按本文方案，PC 相对位移只存在于生成的可执行代码中，宿主指针不进入文件；后续解码使用地址值，不重建基本块指针。

| 指令 | 操作码 | 转移条件 |
| --- | --- | --- |
| `jmp` | `002b` | 始终跳转 |
| `je` | `002c` | `ZF` |
| `jne` | `002d` | `!ZF` |
| `jb` | `002e` | `CF` |
| `jbe` | `002f` | `CF || ZF` |
| `ja` | `0030` | `!CF && !ZF` |
| `jae` | `0031` | `!CF` |
| `jl` | `0032` | `SF != OF` |
| `jle` | `0033` | `ZF || SF != OF` |
| `jg` | `0034` | `!ZF && SF == OF` |
| `jge` | `0035` | `SF == OF` |
| `jo` | `0036` | `OF` |
| `jno` | `0037` | `!OF` |
| `js` | `0038` | `SF` |
| `jns` | `0039` | `!SF` |

`jb/jbe/ja/jae` 用于无符号比较；`jl/jle/jg/jge` 用于有符号比较。当前没有基于 PF 的跳转，也不额外提供 `jz/jnz` 等拼写别名。

`ret` 的操作码是 `003a`，没有显式操作数，仅占两个字节。执行约定从 `[bp]`、`[bp + 8]` 读取保存的 BP、返回 PC，将 SP 恢复为返回前的 BP，再恢复 BP、PC；参数和结果区仍由调用方回收。执行前须完成全部保存区和返回目标检查，失败时不提交部分状态。当前没有立即数清栈形式的 `ret`，执行器仍待实现。

## 函数调用目标与编码

`call` 和 `callNative` 指令族各有立即数与内存两种形式，共用文本名称，由操作数种类选择具体操作码。直接调用使用 `I` 形式的 `ImmediateOperand`，间接调用使用 `M` 形式的 `AddressOperand`；参数和结果不作为显式指令操作数。文本示例：

```text
call @sum
call 16
call [bp + 16]
call [@function_pointer_slot]
callNative @vm_log
callNative 0
callNative [sp - 8]
callNative [@native_pointer_slot]
```

`Calli` 的数值立即数保存相对当前指令起始 PC 的有符号 i64 位模式；符号立即数表示 VM 函数入口，后续镜像生成阶段需要根据最终布局计算该位移。`CallNativei` 的数值立即数必须可表示为 u32，作为 native 导入索引；符号立即数表示显式 Native 函数声明的链接名，后续需转换为导入索引。两种直接调用都不接受符号的非零 addend。名称支持 `@"escaped name"` 转义形式。

链接时，直接 `call @name` 的目标必须是 VM 函数定义，支持本模块 Local 函数以及跨模块 External 函数；直接 `callNative @name` 的目标必须是当前输入显式声明的 Native 函数。全局数据、Native 数据和错误调用类别都会被拒绝。数值目标不参与符号绑定；数值字段编码、镜像中的指令边界及实际导入表范围检查仍需由后续编码、镜像校验、加载及执行阶段完成。

`Callm` 从有效地址处读取一个 64 位 VM 函数地址，再转移至该地址；`CallNativem` 从有效地址处读取一个 64 位 native 可调用指针，该值不是 u32 导入索引。`[@pointer_slot]` 中的符号定位存放指针的内存，链接时按普通地址符号规则解析，不要求它是函数定义或 Native 函数声明。内存地址允许有符号字节偏移，目标值及其可调用性仍须由运行时校验。native 指针的 adapter 与宿主 ABI 协议尚未实现，当前指令模型不表示任意 C 函数指针都可直接调用。

已解析调用目标的字节编码方案如下，尚未实现编码与解码：

| C++ 操作码 | 文本名称 | 编码 | 操作数字段 | 总长度 |
| --- | --- | --- | --- | --- |
| `Calli` | `call` | `003b` | `TargetOffset:i64`，相对当前指令起始 PC 的有符号字节位移 | 10 字节 |
| `CallNativei` | `callNative` | `003c` | `NativeFunctionIndex:u32`，引用 `ExecutableFile::NativeFunctions` | 6 字节 |
| `Callm` | `call` | `003d` | 寄存器相对 `m`：`RegisterId:u8, ByteOffset:i64`，读取 64 位 VM 函数地址 | 11 字节 |
| `CallNativem` | `callNative` | `003e` | 寄存器相对 `m`：`RegisterId:u8, ByteOffset:i64`，读取 64 位 native 可调用指针 | 11 字节 |

`Calli` 的目标为 `PC + TargetOffset`，后继地址为 `NextPC = PC + 10`；`CallNativei` 的后继地址为 `NextPC = PC + 6`。间接形式的后继地址为 `PC + 11`，符号内存地址按上文转换为 PC 相对或宿主地址形式。函数名称不进入字节编码，按索引导入的 native 函数地址不回填到镜像，也不使用数据地址重定位表。符号调用目标随 Local 重命名和 Native 别名合并一并改写，文本读取只恢复逻辑操作数，不代替链接检查。

四条指令的类别都是 `InstructionCategory::Call`，都没有显式结果目标，也不是基本块终结指令。`isCall()` 识别全部调用，`isNativeCall()` 识别两种 `callNative` 形式。它们都不读取 EFLAGS，并通过 `writtenEFlagsMask() == ArithmeticEFlagsMask` 表达 CF、ZF、SF、OF 在调用后不可依赖；这不规定返回后的具体标志值。逻辑指令、构建、遍历、文本读写和链接接入均已实现；指令编码与解码、执行状态转换、调用区自动生成及宿主 ABI adapter 尚未实现。参数、返回槽和可变参数调用预留区布局见 [VM 调用约定](VM-Calling-Convention.md)。

## C++ 指令模型

`MachineModule` 的临时文本读写由 `backend/vm/serialization` 提供。文本保留数值立即数的完整 `u64` 位模式，或符号立即数的名称和完整 `i64` 偏移；本节的数值字节格式则由操作码位宽决定立即数字段的长度。

`opcode.def` 通过 `OPCODE_FORM(Kind, ...)` 集中定义操作数签名，`OPCODE` 记录引用其中的签名、指令类别和条件码。形式名称仅描述操作数类型及顺序，不包含指令语义或位宽，例如 `Add64rri` 引用 `Rri`，`Load64`、`Store64m` 和 `Cmp64mm` 共同引用 `Mm`。`instruction_form.h` 直接从同一份 `opcode.def` 生成 `InstructionForm` 枚举和 `FormTraits` 特化，每种签名只展开一次。每种形式提供 `OperandTuple` 和寄存器、地址、立即数、基本块的计数，包括 `BlockCount`；`None` 对应空元组，`B` 对应单个 `BasicBlockOperand`，`I` 和 `M` 分别对应单个 `ImmediateOperand` 和 `AddressOperand`。顺序来自签名定义；操作数计数用于一致性校验，不能单独推导顺序，例如 `Rrm` 和 `Mrr` 的计数相同而顺序不同。

`machine_instructions.h` 根据 `opcode.def` 为每个操作码生成 `OpcodeTraits`，将操作码绑定到对应的 `Form`、`BitWidth` 和操作数布局。`MachineInstructionImpl<Opcode>` 按值持有该布局的 `std::tuple`，并为每个现有操作码提供对应的指令别名，例如 `MachineAdd64rriInstruction`。构造函数要求操作数的数量和具体类型匹配布局，操作数存储随指令对象一起分配。

只有 Load、Store、Add 提供 `getDestination()`；具有目标和单个来源的形式提供 `getSource()`。Add 的 `getLHS()`、`getRHS()` 分别访问索引 1、2，Cmp 的同名访问器访问索引 0、1；跳转的 `getTarget()` 返回目标基本块指针，`getTargetOperand()` 返回基本块操作数的常量引用。调用的这两个访问器都返回 `const ImmediateOperand &` 或 `const AddressOperand &`，由具体调用形式决定。Ret 没有这些访问器。其余操作数访问器返回具体操作数类型的常量引用，调用者可以直接使用对应的操作数接口：

```cpp
#include "ink/backend/vm/instruction/machine_instructions.h"

namespace ink::vm
{
  const MachineAdd64rriInstruction Add(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(16));
  const RegisterOperand &Destination = Add.getDestination();
  const RegisterOperand &LHS = Add.getLHS();
  const ImmediateOperand &RHS = Add.getRHS();

  const MachineInstruction &Base = Add;
  const Operand *First = Base.getOperand(0);
  const Operand *Missing = Base.getOperand(3); // nullptr: this instruction has three operands.
} // namespace ink::vm
```

`MachineInstruction::operandCount()` 返回包含目标在内的操作数数量；`getOperand(Index)` 按相同的语义顺序返回 `const Operand *`，越界返回 `nullptr`。统一访问在实现文件中通过操作码分派到对应的具体指令布局；这是一层供编码器、打印器和校验器使用的模型遍历接口，不规定未来执行器的热路径。

`category()`、`branchCondition()`、`hasDestination()`、`isCompare()`、`isBranch()`、`isConditionalBranch()`、`isCall()`、`isNativeCall()`、`isReturn()`、`isTerminator()` 描述指令语义。`readEFlagsMask()`、`writtenEFlagsMask()` 表达隐式标志依赖，调度和 lowering 必须保持这些依赖；例如不能在 `cmp` 和依赖它的条件跳转之间插入另一条改写标志的 `add` 或调用。

具体指令通过 `MachineInstructionImpl` 构造函数绑定操作码与布局，该模板为 `final`，保证用于分派的具体类型一致。`MachineInstructionBuilder` 在将新指令交给基本块后设置其 `parent()`。指令对象禁止复制，使用具体类型销毁；基类没有虚函数或虚析构函数。构造函数在编译期约束操作数种类和数量，PC 写入限制、寄存器编号有效性、立即数范围等语义约束仍须由后续指令校验处理。

## 编码、解码与预解码边界

指令编码与解码尚未实现，也没有已解析字节指令的公共 API。当前 `MachineInstruction` 及操作数属于逻辑内存模型，`MachineModule` 文本读写不等同于指令字节编码。独立模块 validator 尚未实现。

指令编码有效与运行时可执行是不同层次的检查。VM 符号须由链接阶段解析，宿主导入由加载阶段绑定；寄存器写权限、指令边界、有效地址计算、访存范围和栈规则也须按执行语义检查。当前操作数构造函数不完成这些检查。

镜像布局与发射、加载和预解码阶段仍待实现。现有 `ExecutableFile::Sections` 模型区分代码、只读数据和可写数据；后续预解码应只遍历 Code 范围。运行时表示必须与逻辑符号名称、指令字节编码和宿主绑定保持清晰边界；任何宿主地址缓存均不得作为进程无关的可执行文件内容保存。
