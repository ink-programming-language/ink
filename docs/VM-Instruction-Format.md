# VM 指令编码格式

本文固定 Ink VM 指令及操作数的字节布局。操作码来自 `src/include/ink/backend/vm/instruction/opcode.def`，寄存器编号来自同目录的 `register.def`。默认指令集版本为 `1.1.0`，外层对象格式版本为 `2.0.0`。本次增加 EFLAGS、比较、跳转和返回指令，保留已有指令的编码。

当前实现提供 `Operand` 基类以及 `RegisterOperand`、`AddressOperand`、`ImmediateOperand`、`BasicBlockOperand` 四种逻辑操作数，并通过强类型指令布局将操作数接入 `MachineInstruction`。基本块操作数在内存中持有目标块指针，直到对象发射阶段才转换成 PC 相对地址字段。[VM 内存模型与指令构建](VM-Module.md) 提供模块、函数、基本块的所有权结构和 `MachineInstructionBuilder`，供 lowering 构建逻辑指令。指令字节编码器、解码器、预解码器和执行器仍待后续实现；现有对象文件读写器只搬运原始代码字节，不验证这些字节是否构成有效指令。

## 通用布局

```text
Opcode:u16 | Operand0 | Operand1 | ...
```

`u8`、`u16`、`u32`、`u64` 分别表示相应位宽的无符号整数，`i64` 表示 64 位二进制补码。下文的字节序列、操作码和前缀编号均以十六进制表示，位宽及长度以十进制表示。每个多字节字段都遵循对象头声明的字节序，包括操作码、符号引用索引、偏移和立即数。字段连续排列，没有隐式填充、对齐字节、单条指令长度字段或操作数长度字段。编码器必须逐字段写出，不能直接写入 C++ 类或结构体的宿主内存布局。

操作码唯一确定操作数的顺序、种类和立即数位宽。有显式结果目标的指令将目标放在最前；加法随后依次是左来源和右来源。比较只有左、右来源，跳转只有目标基本块，返回没有显式操作数。`r`、`m`、`i` 分别表示寄存器值、地址和立即数；`b` 表示内存模型中的目标基本块，在生成对象代码时转换成 PC 相对地址。字节流中不再附加这些逻辑操作数的种类标签，也不保存基本块宿主指针。

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
| `None` | 无显式操作数 |

操作数计数包含所有显式操作数；EFLAGS 等隐式读写不计入其中。指令长度由操作码及各地址操作数的基址前缀计算得出。

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

`r` 不通过对象文件的 `BaseReferences` 查找寄存器。当前普通寄存器来源仅接受 BP、SP、PC，寄存器加法的目标仅接受 BP、SP。EFLAGS 是单独的 32 位状态寄存器，由算术、比较及条件跳转隐式读写，不编码成这些指令的显式 `r` 操作数，也不能用作内存地址基址。

## 地址操作数 `m`

地址操作数保存基址来源和有符号字节偏移。加载、存储、加法和比较使用它定位内存中的值，访问位宽由操作码决定。跳转的内存模型保存基本块指针，对象发射器才将其转换为 PC 相对地址字段；该编码地址用于控制转移，不读取该地址处的内存内容。

| 基址来源 | 字节布局 | 长度 |
| --- | --- | --- |
| 寄存器 | `RegisterId:u8, ByteOffset:i64` | 9 字节 |
| 对象内的符号引用 | `80:u8, BaseReferenceIndex:u32, ByteOffset:i64` | 13 字节 |

首字节的取值规则如下：

| 前缀范围 | 含义 |
| --- | --- |
| `00` 至 `7f` | 寄存器基址区域；只接受 BP、SP、PC 这三个地址寄存器编号 |
| `80` | 符号引用基址，随后读取 `u32` 引用索引和 `i64` 偏移 |
| `81` 至 `ff` | 保留，必须拒绝 |

因此，当前 `03` 至 `7f` 在内存基址位置均属于无效编码，包括已定义但不能寻址的 EFLAGS。前缀 `80` 不占用符号索引的任何位，符号引用索引保留完整的 32 位范围。以后新增可寻址寄存器时，其编号必须小于 `80`。

`BaseReferenceIndex` 是所属对象 `BaseReferences` 的下标，从 `0` 开始。该表项的 `SymbolIndex` 才是所属对象 `Symbols` 的下标；两者不能混用。引用可以指向本地定义、导出定义或尚待链接的导入符号。

`ByteOffset` 始终占八字节，表示相对基址的有符号字节位移。符号引用索引 `0` 与寄存器 BP 的编号 `0` 分别通过符号前缀和寄存器前缀解释，没有相同编号下的隐式回退。

下面的竖线只用于说明字段边界，不属于字节流：

| 地址 | 小端序 | 大端序 |
| --- | --- | --- |
| `[bp + 16]` | `00 \| 10 00 00 00 00 00 00 00` | `00 \| 00 00 00 00 00 00 00 10` |
| `[符号引用 5 + 8]` | `80 \| 05 00 00 00 \| 08 00 00 00 00 00 00 00` | `80 \| 00 00 00 05 \| 00 00 00 00 00 00 00 08` |
| `[sp - 8]` | `01 \| f8 ff ff ff ff ff ff ff` | `01 \| ff ff ff ff ff ff ff f8` |

## 立即数操作数 `i`

立即数仅保存指定位宽的位模式，没有自身的类型标签或长度字段。`opcode.def` 的 `Width` 决定其编码宽度：

| `Width` | 字节数 |
| --- | --- |
| `8` | 1 |
| `16` | 2 |
| `32` | 4 |
| `64` | 8 |

`ImmediateOperand` 使用 `u64` 容纳位模式，指令决定实际使用的位宽和数值解释。负整数按相应位宽的二进制补码表示。当前寄存器目标的 64 位加法将右来源解释为有符号 64 位字节位移；普通整数结果的加法仍遵循操作码规定的位宽回绕语义。

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

## 基本块目标的对象编码与返回

所有跳转的内存表示统一保存 `const MachineBasicBlock *`。`BasicBlockOperand` 提供操作数包装，`getTarget()` 直接返回目标指针，`getTargetOperand()` 返回包装引用。Builder 可直接接收目标块指针，要求目标非空，且源块、目标块属于同一函数。文本写为 `jne failure`、`jmp done`，支持函数内的前向、后向和自引用；数值 PC 相对地址不作为逻辑跳转操作数接受。

在生成 `ObjectFile` 的阶段，后续对象发射器先完成布局，再根据目标块指针计算代码位置，并将目标转换为相对当前指令起始 PC 的字节位移，沿用通用寄存器地址编码：

```text
Opcode:u16 | RegisterId:u8 = 02 | ByteOffset:i64
TargetPC = PC + ByteOffset
```

编码后的每条跳转固定占 11 字节，位移为有符号 `i64` 字节数并遵循对象头指定的字节序。条件不满足时继续执行 `NextPC = PC + 11`，满足时转移到 `TargetPC`；零位移表示跳回当前指令。条件跳转只读取所需标志，不修改标志。后续发射器负责检查位移的可表示性，执行器负责检查地址计算和合法指令边界。

内存中的跳转目标随基本块对象保持稳定，指令插入或块布局变化不要求调用方手工维护位移。文本写出根据目标块对象身份选择标签，读取将函数内标签解析回对应指针；匿名和重名块通过独立标签保留精确身份。PC 相对位移只存在于生成的对象代码中，宿主指针不进入文件。当前尚未实现对象发射器或指令字节编解码器。

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

`ret` 的操作码是 `003a`，没有显式操作数，仅占两个字节。执行约定从 `[bp]`、`[bp + 8]` 读取保存的 BP、返回 PC，将 SP 恢复为返回前的 BP，再恢复 BP、PC；参数和结果区仍由调用方回收。执行前须完成全部保存区和返回目标检查，失败时不提交部分状态。当前没有立即数清栈形式的 `ret`，CALL 和执行器仍待实现。

## C++ 指令模型

`MachineModule` 的临时文本读写由 `backend/vm/serialization` 提供。文本保留 `ImmediateOperand` 的完整 `u64` 位模式；本节的字节格式则由操作码位宽决定立即数字段的长度。

`opcode.def` 通过 `OPCODE_FORM(Kind, ...)` 集中定义操作数签名，`OPCODE` 记录引用其中的签名、指令类别和条件码。形式名称仅描述操作数类型及顺序，不包含指令语义或位宽，例如 `Add64rri` 引用 `Rri`，`Load64`、`Store64m` 和 `Cmp64mm` 共同引用 `Mm`。`instruction_form.h` 直接从同一份 `opcode.def` 生成 `InstructionForm` 枚举和 `FormTraits` 特化，每种签名只展开一次。每种形式提供 `OperandTuple` 和寄存器、地址、立即数、基本块的计数，包括 `BlockCount`；`None` 对应空元组，`B` 对应单个 `BasicBlockOperand`。顺序来自签名定义；操作数计数用于一致性校验，不能单独推导顺序，例如 `Rrm` 和 `Mrr` 的计数相同而顺序不同。

`machine_instructions.h` 根据 `opcode.def` 为每个操作码生成 `OpcodeTraits`，将操作码绑定到对应的 `Form`、`BitWidth` 和操作数布局。`MachineInstructionImpl<Opcode>` 按值持有该布局的 `std::tuple`，并为每个现有操作码提供对应的指令别名，例如 `MachineAdd64rriInstruction`。构造函数要求操作数的数量和具体类型匹配布局，操作数存储随指令对象一起分配。

只有 Load、Store、Add 提供 `getDestination()`；具有目标和单个来源的形式提供 `getSource()`。Add 的 `getLHS()`、`getRHS()` 分别访问索引 1、2，Cmp 的同名访问器访问索引 0、1；跳转的 `getTarget()` 返回目标基本块指针，`getTargetOperand()` 返回基本块操作数包装的常量引用，Ret 没有这些访问器。其余操作数访问器返回具体操作数类型的常量引用，调用者可以直接使用对应的操作数接口：

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

`category()`、`branchCondition()`、`hasDestination()`、`isCompare()`、`isBranch()`、`isConditionalBranch()`、`isReturn()`、`isTerminator()` 描述指令语义。`readEFlagsMask()`、`writtenEFlagsMask()` 表达隐式标志依赖，调度和 lowering 必须保持这些依赖；例如不能在 `cmp` 和依赖它的条件跳转之间插入另一条改写标志的 `add`。

具体指令通过 `MachineInstructionImpl` 构造函数绑定操作码与布局，该模板为 `final`，保证用于分派的具体类型一致。`MachineInstructionBuilder` 在将新指令交给基本块后设置其 `parent()`。指令对象禁止复制，使用具体类型销毁；基类没有虚函数或虚析构函数。构造函数在编译期约束操作数种类和数量，PC 写入限制、寄存器编号有效性、立即数范围等语义约束仍须由后续指令校验处理。

## 解码与预解码边界

后续指令解码器需要拒绝未知操作码、未定义的寄存器编号、保留的地址前缀以及任何不完整的字段。在知道完整字段长度前不能读取其内容；例如符号前缀后必须还有四字节引用索引和八字节偏移。引用索引必须小于所属对象的 `BaseReferences` 项数。

指令编码有效、引用索引有效与运行时可执行是不同层次的检查。导入符号仍须由链接或加载阶段解析；寄存器写权限、指令边界、有效地址计算、访存范围和栈规则也须按执行语义检查。上述后续检查不由当前的 `ObjectFileValidator` 或操作数构造函数完成。

计划在加载或预解码阶段将两种内存基址统一映射为运行时的 `BaseSlot`，执行时使用 `BaseValues[BaseSlot] + ByteOffset` 定位内存。寄存器槽存放实时值，符号槽存放已解析地址；多对象的局部引用索引需要映射到同一已加载程序的槽布局。此运行时表示不写入对象文件，目前也尚未实现。
