# VM 指令编码格式

本文固定 Ink VM 指令及三种操作数的第一版字节布局。操作码来自 `src/include/ink/vm/instruction/opcode.def`，寄存器编号来自同目录的 `register.def`。默认指令集版本为 `1.0.0`，外层对象格式版本为 `2.0.0`。

当前实现提供 `Operand` 基类以及 `RegisterOperand`、`AddressOperand`、`ImmediateOperand` 三种逻辑操作数，并通过强类型指令布局将操作数接入 `Instruction`。指令字节编码器、解码器、预解码器和执行器仍待后续实现；现有对象文件读写器只搬运原始代码字节，不验证这些字节是否构成有效指令。

## 通用布局

```text
Opcode:u16 | Operand0 | Operand1 | ...
```

`u8`、`u16`、`u32`、`u64` 分别表示相应位宽的无符号整数，`i64` 表示 64 位二进制补码。下文的字节序列、操作码和前缀编号均以十六进制表示，位宽及长度以十进制表示。每个多字节字段都遵循对象头声明的字节序，包括操作码、符号引用索引、偏移和立即数。字段连续排列，没有隐式填充、对齐字节、单条指令长度字段或操作数长度字段。编码器必须逐字段写出，不能直接写入 C++ 类或结构体的宿主内存布局。

操作码唯一确定操作数的顺序、种类和立即数位宽。操作数按指令的语义顺序排列，目标在前，加法随后依次是左来源和右来源。`r`、`m`、`i` 分别表示寄存器值、内存地址和立即数；字节流中不再附加这三种操作数的种类标签。

| `opcode.def` 中的形式 | 操作数顺序 |
| --- | --- |
| `Mm` | `m, m`，依次为目标和来源 |
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

操作数计数包含目标。指令长度由操作码及各地址操作数的基址前缀计算得出。

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

当前只有这三个编号有效。`r` 不通过对象文件的 `BaseReferences` 查找寄存器。一个寄存器能否作为特定位置的目标还须遵循操作码的语义；例如当前加法指令的寄存器目标可以是 BP 或 SP，不能是 PC。

## 内存地址操作数 `m`

地址操作数保存基址来源和有符号字节偏移。它描述访问的位置，访问值的位宽由操作码决定。

| 基址来源 | 字节布局 | 长度 |
| --- | --- | --- |
| 寄存器 | `RegisterId:u8, ByteOffset:i64` | 9 字节 |
| 对象内的符号引用 | `80:u8, BaseReferenceIndex:u32, ByteOffset:i64` | 13 字节 |

首字节的取值规则如下：

| 前缀范围 | 含义 |
| --- | --- |
| `00` 至 `7f` | 寄存器基址区域；只接受 `register.def` 中已定义且位于该区域的编号 |
| `80` | 符号引用基址，随后读取 `u32` 引用索引和 `i64` 偏移 |
| `81` 至 `ff` | 保留，必须拒绝 |

因此，当前 `03` 至 `7f` 也属于无效编码。前缀 `80` 不占用符号索引的任何位，符号引用索引保留完整的 32 位范围。以后新增寄存器时，能直接用作内存基址前缀的寄存器编号必须小于 `80`。

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

## C++ 指令模型

`opcode.def` 通过 `OPCODE_FORM(Kind, ...)` 集中定义当前十一种操作数签名，`OPCODE` 记录引用其中的签名。形式名称仅描述操作数类型及顺序，不包含指令语义或位宽，例如 `Add64rri` 引用 `Rri`，`Load64` 和 `Store64m` 共同引用 `Mm`。`instruction_form.h` 直接从同一份 `opcode.def` 生成 `InstructionForm` 枚举和 `FormTraits` 特化，每种签名只展开一次。每种形式提供 `OperandTuple` 和各类操作数计数，例如 `Rri` 的元组依次包含 `RegisterOperand`、`RegisterOperand`、`ImmediateOperand`。顺序来自签名定义；操作数计数用于一致性校验，不能单独推导顺序，例如 `Rrm` 和 `Mrr` 的计数相同而顺序不同。

`instructions.h` 根据 `opcode.def` 为每个操作码生成 `OpcodeTraits`，将操作码绑定到对应的 `Form`、`BitWidth` 和操作数布局。`InstructionImpl<Opcode>` 按值持有该布局的 `std::tuple`，并为每个现有操作码提供对应的指令别名，例如 `Add64rriInstruction`。构造函数要求操作数的数量和具体类型匹配布局，操作数存储随指令对象一起分配。

所有指令都提供 `getDestination()`；二元形式提供 `getSource()`，三元形式提供 `getLHS()` 和 `getRHS()`。这些访问器返回具体操作数类型的常量引用，调用者可以直接使用对应的寄存器、地址或立即数接口：

```cpp
#include "ink/vm/instruction/instructions.h"

namespace ink::vm
{
  const Add64rriInstruction Add(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(16));
  const RegisterOperand &Destination = Add.getDestination();
  const RegisterOperand &LHS = Add.getLHS();
  const ImmediateOperand &RHS = Add.getRHS();

  const Instruction &Base = Add;
  const Operand *First = Base.getOperand(0);
  const Operand *Missing = Base.getOperand(3); // nullptr: this instruction has three operands.
} // namespace ink::vm
```

`Instruction::operandCount()` 返回包含目标在内的操作数数量；`getOperand(Index)` 按相同的语义顺序返回 `const Operand *`，越界返回 `nullptr`。统一访问在实现文件中通过操作码分派到对应的具体指令布局；这是一层供编码器、打印器和校验器使用的模型遍历接口，不规定未来执行器的热路径。

基类构造函数仅授权 `InstructionImpl` 调用，具体指令模板为 `final`，保证操作码与用于分派的具体类型一致。指令对象禁止复制，使用具体类型销毁；基类没有虚函数或虚析构函数。构造函数在编译期约束操作数种类和数量，PC 写入限制、寄存器编号有效性、立即数范围等语义约束仍须由后续指令校验处理。

## 解码与预解码边界

后续指令解码器需要拒绝未知操作码、未定义的寄存器编号、保留的地址前缀以及任何不完整的字段。在知道完整字段长度前不能读取其内容；例如符号前缀后必须还有四字节引用索引和八字节偏移。引用索引必须小于所属对象的 `BaseReferences` 项数。

指令编码有效、引用索引有效与运行时可执行是不同层次的检查。导入符号仍须由链接或加载阶段解析；寄存器写权限、指令边界、有效地址计算、访存范围和栈规则也须按执行语义检查。上述后续检查不由当前的 `ObjectFileValidator` 或操作数构造函数完成。

计划在加载或预解码阶段将两种内存基址统一映射为运行时的 `BaseSlot`，执行时使用 `BaseValues[BaseSlot] + ByteOffset` 定位内存。寄存器槽存放实时值，符号槽存放已解析地址；多对象的局部引用索引需要映射到同一已加载程序的槽布局。此运行时表示不写入对象文件，目前也尚未实现。
