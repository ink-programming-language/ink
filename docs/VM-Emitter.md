# VM 可执行镜像发射与指令编解码

`backend/vm/emitter` 提供 `ExecutableEmitter`，将一个已完成链接的 `MachineModule` 生成为独立拥有字节、区段、导入及重定位信息的 `ExecutableFile`。`backend/vm/bytecode` 提供与逻辑节点无关的最小指令编解码器。它们均属于 `ink::vm`，由现有 `ink::vm` 库构建。

```cpp
#include "ink/backend/vm/emitter/executable_emitter.h"
#include "ink/backend/vm/bytecode/instruction_codec.h"

// Linked 和 Entry 分别为已链接模块及其入口函数，Diagnostics 为诊断引擎。
auto Image = ink::vm::ExecutableEmitter::emit(Linked, Entry, Diagnostics, ink::vm::ByteOrder::Little);
if (Image)
{
  const auto &Code = Image->getSections().front();
  const auto Bytes = std::span<const std::uint8_t>(Image->getImage()).subspan(Code.ImageOffset, Code.ByteSize);
  auto First = ink::vm::InstructionCodec::decode(Bytes, Image->getByteOrder());
}
```

## 输入和失败

`emit(const MachineModule &, const MachineFunction &, const core::DiagnosticEngine &, ByteOrder)` 返回 `std::optional<ExecutableFile>`，默认小端。入口通过函数引用显式选择，可以是匿名 Local 函数，也可以不是模块的第一个函数。输入只在调用期间借用；成功后可销毁原模块，发射结果不包含原节点指针。发射器不会修改模块、函数、基本块、栈帧或指令。

发射器假定输入已经通过独立的模块 validator，直接使用已验证的结构和操作数，不重复检查模块合法性。前置条件包括入口及所有节点的归属、非空函数体、名称有效性与唯一性、符号存在、声明已完成链接、调用目标种类与 addend、寄存器权限、跳转目标及 native 数据地址立即数的 64 位字段。该 validator 尚待实现；发射器不会以内置检查或断言代替它。多个输入应先经 `ModuleLinker::link()` 合并。栈帧描述只作为输入元数据保留，发射不会自动插入栈空间调整或调用序列。

发射期失败时报告首条 `VMExecutableEmissionFailed`（`INK-V0004`，普通错误）并返回 `nullopt`，不返回部分镜像。错误仅覆盖无效字节序选项、依赖最终布局的镜像大小和地址或位移可表示性，以及尚未支持的 native 函数地址编码；宿主分配失败沿用仓库的进程级致命故障规则。`emit()` 不调用 `ExecutableFileValidator`；调用方需要时可在发射后单独校验镜像。

## 布局

VM 地址值为相对 `Image` 开头的无符号 64 位字节偏移，不是宿主指针。将来加载或执行时必须保留这一虚拟地址含义；不能根据宿主 buffer 地址重新解释立即数。

1. 从偏移 0 开始，按模块函数顺序、函数基本块顺序、块指令顺序连续发射代码，不添加代码对齐字节。
2. 按原模块顺序布局所有只读全局变量。
3. 按原模块顺序布局所有可写全局变量。

每个全局变量按自身 Alignment 对齐；初始化字节原样复制，对齐 padding 和未初始化尾部填零。原始初始化字节不根据镜像字节序转换。零大小全局变量仍具有布局地址，但可以与其他地址重合。

`Sections` 按物理顺序记录 Code、ReadOnlyData、WritableData；空区段省略。数据区段包含首个对象之前及对象之间的对齐 padding。所有区段无重叠、无间隙地覆盖完整镜像。`EntryOffset` 指向所选函数的第一条指令。字节序由 `getByteOrder()` 保存，适用于代码多字节字段和将来重定位写回。

## 地址解析与调用

| 逻辑操作数 | 发射结果 |
| --- | --- |
| 寄存器相对内存地址 | 原编号与 i64 位移，9 字节 |
| VM 符号相对内存地址 | `PC` 基址与 `SymbolOffset + Addend - InstructionPC`，9 字节 |
| VM 符号地址立即数 | `SymbolOffset + Addend`；必须精确适合指令的立即数字段 |
| 基本块跳转目标 | `PC` 基址与目标块相对当前指令起点的 i64 位移，9 字节 |
| `call @function` | 目标函数相对当前指令起点的 i64 位移 |
| `callNative @function` | u32 native 函数导入索引 |
| native 数据内存地址 | `0xff` 基址标记及 8 字节零占位字段，并生成数据重定位 |
| native 数据地址立即数 | 8 字节零占位字段，并生成数据重定位；要求指令位宽为 64 |

所有相对位移以当前指令起点计算，不以 NextPC 计算。跳转目标的归属和指令边界由模块 validator 保证；发射器直接按块布局计算位移，空中间块与后续非空块共享入口。数值 `call` 位移和 `callNative` 索引直接编码，不在发射器内检查函数入口、目标指令边界或导入表范围。间接调用只发射存放目标的内存操作数，其运行时指针值和可调用性由后续执行器检查。

Native 函数导入按首次出现顺序排列，按 Provider、宿主名称合并相同身份；相同身份的 CallKind 一致性由模块 validator 保证，发射器不重复检查。Native 数据按宿主名称合并。数据重定位的 `ImageOffset` 精确指向 8 字节字段，不包含地址基址标记，`Addend` 完整保留；没有宿主符号解析或地址回填。`0xff` 明确选择宿主地址空间，不能当作普通 VM 寄存器编号。直接 native 函数导入索引不参与数据重定位。

Native 函数符号作为普通地址立即数或内存基址时，发射器明确报错；可调用 adapter 指针的构造协议尚未定义。全局初始化原始字节中的符号地址记录亦尚未提供。

## 最小编解码器

`InstructionCodec::byteSize(Opcode)` 返回固定指令长度；未知操作码返回 `nullopt`。`encode(BytecodeInstruction, ByteOrder)` 返回字节 vector 或 `BytecodeError`；`encodeUnchecked(BytecodeInstruction, ByteOrder)` 直接返回字节 vector，要求输入已验证。发射器使用后者写出已解析值，避免在 emit 路径重复校验操作数。`decode(span, ByteOrder)` 返回 `DecodedInstruction` 或错误。解码只读取第一条指令，结果 `ByteSize` 用于前移游标；解码本身不分配内存。调用方应仅向解码器传入 Code 区段内的字节，避免将数据当指令。

`BytecodeInstruction` 保存 Operation、OperandCount 和最多三个有序操作数；`BytecodeOperand` 保存 Register、Address 或 Immediate。它们仅表示已解析的编码值，不包含名称、逻辑基本块或节点所有权，也不会重建 `MachineInstruction`。操作数数量、顺序、位宽来自 `opcode.def` 与 `OpcodeTraits`，不存在另一份手写指令表。

独立使用的 checked `encode()` 保留字节序、操作码、操作数数量和种类、显式寄存器读写权限、地址基址、跳转 PC 基址及 `CallNativei` 的 u32 范围检查；`encodeUnchecked()` 不执行这些检查。`decode()` 对不可信字节保留完整字段长度及操作数合法性检查。普通窄数值立即数写入低位，负数按补码位模式编码；依赖最终布局的符号地址字段可表示性仍由发射器处理。

编解码成功仅表示单条指令格式有效；不表示整个程序的指令边界、动态访存、调用栈或 native ABI 已通过执行验证。加载器、预解码缓存、执行器、IR lowering 和磁盘文件读写仍待实现。
