# VM 指令编码格式讨论稿

本文保留 Ink VM 指令及操作数的编码方案讨论。当前已实现逻辑指令、模块链接、可执行镜像模型与结构校验，以及全局布局、函数名偏移登记、指令字节发射、显式入口选择和导出生成；解释器已实现部分指令解码与执行，独立公共解码器仍待实现。操作码来自 `src/include/ink/backend/vm/instruction/opcode.def`，寄存器编号来自同目录的 `register.def`。VM 与其他 Ink 组件统一使用 `InkVersion`，默认值为 `0.0.1`，可通过 `INK_VERSION` 配置；当前指令集包含 `call` 和 `callNative` 逻辑操作码。符号地址操作数在逻辑模型中保留链接名和偏移，由布局、发射与后续加载绑定阶段转换为数值字段。

当前实现提供 `Operand` 基类以及 `RegisterOperand`、`AddressOperand`、`ImmediateOperand`、`BasicBlockOperand` 四种逻辑操作数，并通过强类型指令布局将操作数接入 `MachineInstruction`。基本块操作数在内存中持有目标块指针；调用复用立即数和地址操作数，分别表达直接目标和存放间接目标的内存位置。[VM 内存模型与指令构建](VM-Module.md) 提供模块、函数、基本块的所有权结构和 `MachineInstructionBuilder`，供 lowering 构建逻辑指令。[模块链接器](VM-Linker.md) 已实现逻辑模块合并、符号解析和深克隆，`executable` 目录提供镜像模型与结构校验。加载和部分执行分派已实现，预解码缓存及其余指令执行仍待实现。

发射代码分为 `ExecutableEmitter`、`FunctionEmitter` 和 `InstructionEmitter`。`ExecutableEmitter` 拥有 `ExecutableEmissionContext`，管理全局布局和模块发射流程；该上下文保存镜像、模块符号、统一的 `NativeSymbols` 及待解析重定位，并提供字节追加和回填。Native 符号由 `SymbolDeclarations - (Functions + Globals)` 推导，与 VM 定义同名的声明不生成 Native 导入。`FunctionEmitter` 拥有 `FunctionEmissionContext`，保存当前函数的基本块偏移状态；`InstructionEmitter` 借用两个上下文，负责 opcode 和操作数字段编码。`padInstruction` 先写入两字节 opcode，再按语义顺序调用对应的 `padOperand`；`FunctionEmitter::emit` 重置和扫描基本块状态后，按函数内顺序调用 `padBasicBlock`；该接口在 `FunctionEmissionContext` 中登记当前镜像偏移并移除对应的未解析记录，再按块内顺序调用 `InstructionEmitter::padInstruction`。基本块操作数已编码为 PC 编号和相对位移，前向引用在当前函数的全部基本块发射后统一回填；直接调用已按操作码分别编码 PC 相对位移或 u32 导入索引。

`ExecutableEmitter::emit` 必须显式传入模块、入口函数名和导出名称列表。入口必须是具名且包含指令的 VM 函数，导出仅选择列表中的 VM 函数或全局变量；发射器在布局和静态重定位完成后生成 `EntryOffset` 和 `ExportSymbols`，每次发射会清理旧元数据。调用示例与完整选择规则见 [VM 可执行镜像模型](VM-Executable.md#显式入口与导出选择)。

## 通用布局

```text
Opcode:u16 | Operand0 | Operand1 | ...
```

`u8`、`u16`、`u32`、`u64` 分别表示相应位宽的无符号整数，`i64` 表示 64 位二进制补码。下文的字节序列、操作码和寄存器编号均以十六进制表示，位宽及长度以十进制表示。VM 固定使用小端序，操作码、偏移、立即数和地址重定位字段均按小端序编码，镜像不保存字节序配置。磁盘格式仍待约定。方案中的字段连续排列，没有隐式填充、对齐字节、单条指令长度字段或操作数长度字段，不使用 C++ 类或结构体的宿主内存布局。

运行时数据指针、VM 函数入口及 BP、SP、PC 统一使用 64 位宿主地址，ZERO 恒为零。普通 VM 符号取地址在加载时计算为 `ImageBase + SymbolOffset + Addend`；Native 符号取地址为解析后的宿主地址加 Addend。这里的 SymbolOffset 相对整个未绑定 Image 开头。内存地址统一采用寄存器基址与八字节偏移位模式，PC 相对地址使用 PC 基址，绝对地址使用 ZERO 基址，计算结果采用同一运行时表示；保存、传递一个地址值时无需携带其重定位来源。未绑定镜像只保存偏移、占位字段和重定位记录，实际宿主指针只出现在地址稳定的已加载镜像及运行时状态中。

操作码唯一确定操作数的顺序、种类和立即数位宽。有显式结果目标的指令将目标放在最前；加法随后依次是左来源和右来源。无条件跳转只有一个目标来源；条件跳转依次保存两个比较来源和一个目标块，调用只有目标来源，返回没有显式操作数。`r`、`m`、`i` 分别表示寄存器值、内存地址和立即数；`b` 表示内存模型中的目标基本块。已约定的字节布局不附加逻辑操作数的种类标签，也不保存基本块宿主指针或进程内的名称句柄。

| `opcode.def` 中的形式 | 操作数顺序 |
| --- | --- |
| `Mm` | `m, m`；加载、存储为目标和来源 |
| `Mi` | `m, i` |
| `Mmm` | `m, m, m` |
| `Mmi` | `m, m, i` |
| `Mii` | `m, i, i` |
| `Mmb` | `m, m, b`；条件跳转的左来源、右来源、目标块 |
| `Mib` | `m, i, b`；条件跳转的左来源、右来源、目标块 |
| `Rrr` | `r, r, r` |
| `Rrm` | `r, r, m` |
| `Rri` | `r, r, i` |
| `Mrr` | `m, r, r` |
| `Mrm` | `m, r, m` |
| `Mri` | `m, r, i` |
| `Rr` | `r, r` |
| `Rm` | `r, m` |
| `Ri` | `r, i` |
| `Mr` | `m, r` |
| `B` | `b`，内存模型中的目标基本块 |
| `I` | `i`，直接跳转或调用目标 |
| `R` | `r`，寄存器跳转目标 |
| `M` | `m`，存放间接跳转或调用目标的内存地址 |
| `None` | 无显式操作数 |

操作数计数包含所有显式操作数。指令长度由操作码与各字段宽度确定，符号解析不会改变指令长度。

## 寄存器操作数 `r`

```text
RegisterId:u8 | ByteOffset:i64
```

寄存器操作数恰好占九个字节。作为来源时，其值为 `RegisterValue + ByteOffset`，不读取内存；`ByteOffset` 是有符号 64 位字节偏移，默认为零。文本 `pc + 2` 对应 `RegisterOperand(RegisterId::PC, 2)`，`sp - 8` 对应 `RegisterOperand(RegisterId::SP, -8)`。

| 编码 | 寄存器 |
| --- | --- |
| `00` | `BP` |
| `01` | `SP` |
| `02` | `PC` |
| `03` | `ZERO`，文本拼写为 `zero`，恒为零且不可写 |

当前普通寄存器来源接受 BP、SP、PC、ZERO；寄存器加法的目标仅接受 BP、SP，目标的 `ByteOffset` 必须为零，PC 和 ZERO 均不可作为普通写入目标。PC 保存当前指令起始位置的宿主地址，`pc + 2` 表示该地址之后第二个字节的地址；BP、SP 保存栈中的宿主地址。ZERO 每次读取都为零，`zero + N` 表示偏移 N 的位模式，不改变 ZERO。

`InstructionEmitter::padOperand(const RegisterOperand &Operand)` 与寄存器形式的地址操作数共用 `padRegister`，顺序追加寄存器编号和偏移。两者的九字节字段布局相同；操作码及操作数位置决定取寄存器值加偏移，还是访问该地址处的内存。

## 地址操作数 `m`

地址操作数保存寄存器或符号链接名，以及有符号字节偏移。加载、存储、加法、比较和间接调用使用它定位内存中的值，访问位宽由操作码决定。符号形式通过 `core::Name` 保存名称，例如文本 `[@counter + 8]`。读取符号地址本身时使用立即数形式 `@counter + 8`。跳转的内存模型保存基本块指针，指令发射将其转换为 PC 相对地址字段；该编码地址用于控制转移，不读取该地址处的内存内容。

```text
RegisterId:u8 | OffsetBits:u64
EffectiveAddress = Registers[RegisterId] + OffsetBits
```

地址操作数固定占九字节，基址只接受 BP、SP、PC、ZERO，未知编号不是有效基址。BP、SP、PC 的有符号字节位移按 `i64` 补码保存为 `OffsetBits`；ZERO 相对字段可保存完整的 `u64` 绝对地址位模式，无需将地址限制在 `i64` 正数范围内。执行器可直接按寄存器编号索引基址并执行统一的 64 位地址计算，无需按 PC 相对或绝对寻址分支；地址运算、范围与权限检查仍须独立完成。`ff` 不再表示绝对地址基址，也不是有效寄存器编号。

VM 符号内存地址 `Symbol + ByteOffset` 转换为相对当前指令起始 PC 的有符号位移，使用 `PC = 02` 基址及八字节偏移编码。若 `PC = ImageBase + InstructionOffset`，编码位移为 `SymbolOffset + ByteOffset - InstructionOffset`，计算出的有效地址就是 `ImageBase + SymbolOffset + ByteOffset`，无需加载时再修补位移。宿主符号使用 `ZERO = 03` 基址，后续八字节偏移字段由 `DynamicImportSymbolRelocation` 记录其位置、统一导入表 `DynamicImportSymbols` 的索引及 addend，加载时填写宿主地址加 addend。函数和数据取地址共用此表。ZERO 加该字段的值就是绑定后的绝对地址，执行阶段使用相同的寄存器相对计算。Native 函数地址的可调用表示及 adapter 协议尚未确定。进程内的 `core::Name` 索引不能进入指令字节。

`InstructionEmitter::padOperand(const AddressOperand &Operand, std::uint64_t InstructionOffset)` 已实现上述寄存器相对地址字段的顺序追加。`InstructionOffset` 必须是当前指令起点。`ExecutableEmissionContext::resolveRegisterRelativeOffset` 和 `InstructionEmitter::padRegisterRelativeSymbol` 显式接收基址寄存器编号；VM 符号已完成布局时使用 PC 基址写入相对位移，ZERO 基址的 VM 符号地址则生成镜像基址地址重定位。已登记在 `UnresolvedSymbol` 但尚未布局时，追加基址寄存器编号和八字节零占位，并在上下文的 `PendingStaticRelocationSymbol` 中记录字段的 `ImageOffset`、符号 `Name`、基址寄存器 `Register`、当前指令的镜像偏移 `InstructionOffset` 和符号偏移 `Addend`。最终解析直接根据 `Register` 处理：PC 引用按 `InstructionOffset` 求差并回填位移，ZERO 引用生成 `ImageBaseAddressRelocation`，由加载器填写 `ImageBase + TargetOffset + Addend`。同一符号的多次引用分别记录；每次 `ExecutableEmitter::emit` 清空旧记录，依次调用 `padGlobals`、`padFunction` 后统一调用 `resolveStaticRelocations`。最终解析仍找不到目标偏移时报告内部编译错误，成功后清空临时队列。`scanUnResolvedSymbol` 将模块中没有函数或全局定义的声明加入统一的 `NativeSymbols`；地址操作数使用这些名称时，按同一 `Name` 复用或添加导入，追加 ZERO 寄存器编号和八字节零占位，并为偏移字段生成重定位。

下面的竖线只用于说明字段边界，不属于字节流：

| 地址 | 小端序 |
| --- | --- |
| `[bp + 16]` | `00 \| 10 00 00 00 00 00 00 00` |
| `[sp - 8]` | `01 \| f8 ff ff ff ff ff ff ff` |
| `[zero + 16]` | `03 \| 10 00 00 00 00 00 00 00` |

## 立即数操作数 `i`

逻辑立即数支持数值和符号地址两种形式。`ImmediateValueKind::Numeric` 保存完整的 `u64` 位模式；`ImmediateValueKind::Symbol` 保存 `core::Name` 链接名和有符号 `i64` 字节偏移。数值立即数的最终编码没有自身的类型标签或长度字段，`opcode.def` 的 `Width` 决定字段宽度：

| `Width` | 字节数 |
| --- | --- |
| `8` | 1 |
| `16` | 2 |
| `32` | 4 |
| `64` | 8 |

`ImmediateOperand::getValueKind()` 区分两种形式。`bits()` 只在数值形式返回位模式；`getSymbol()` 和 `getByteOffset()` 只在符号形式返回名称与偏移，各访问器用 `std::optional` 表示不适用的情况。负数值按相应位宽的二进制补码表示。当前寄存器目标的 64 位加法将右来源解释为有符号 64 位字节位移；普通整数结果的加法仍遵循操作码规定的位宽回绕语义。

符号立即数表示 `AddressOf(Symbol) + ByteOffset`，不读取该地址处的内存。文本采用 `@name` 或 `@"escaped name"`，并可附加 `+ Offset` 或 `- Offset`。例如 `add64mii [bp + 16], @counter + 8, 1` 的左来源是 counter 地址加 8。文本读写保留完整的名称和 `i64` 偏移。普通符号取地址统一要求 64 位字段。VM 符号通过 `ImageBaseAddressRelocation` 在加载时填写 `ImageBase + TargetOffset + Addend`；宿主符号通过动态导入符号地址重定位填写绑定后的宿主地址加 Addend。两者在加载后都得到完整的运行时地址，不能截断保留地址低位。Native 函数地址的可调用表示及 adapter 协议尚未确定。

`InstructionEmitter::padOperand(const ImmediateOperand &Operand, std::uint32_t Width)` 已实现普通立即数字段的顺序追加。数值形式接受 Width 为 8、16、32、64，保留目标位宽的低位，负数按相应位宽的补码编码。符号形式只接受 Width 为 64，统一追加八字节零。已布局的 VM 符号生成包含字段起点、目标符号镜像偏移及 addend 的 `ImageBaseAddressRelocation`；未布局的 VM 符号先在上下文的 `PendingStaticRelocationSymbol` 中保存字段起点、名称、addend 和 `Register = ZERO`，最终解析根据 ZERO 基址生成镜像基址地址重定位；这里的寄存器编号仅属于临时记录，不写入普通立即数字段。`padGlobals` 只负责布局与登记符号偏移，`ExecutableEmitter::emit` 依次执行 `padGlobals`、`padFunction` 后统一执行最终解析；`padFunction` 在每个函数发射前以当前镜像字节数登记函数名的入口偏移，并移除其未解析符号记录，再按函数、基本块和指令顺序发射字节。该登记用于符号引用解析；布局和静态重定位完成后，`ExecutableEmitter::emit` 按显式入口名称与导出列表填写 `EntryOffset` 和 `ExportSymbols`。宿主符号生成 `DynamicImportSymbolRelocation`，按宿主 Name 复用导入；普通符号立即数字段前不追加寄存器编号。未知符号、VM 与 Native 定义冲突或非法位宽报告内部编译错误。加载器负责检查实际绑定地址与 addend 的运算并按小端序填写字段；加载已由 `Loader` 实现。

直接调用复用 `ImmediateOperand`，但按调用操作码解释目标：`Calli` 的立即数是 PC 相对位移或待解析的 VM 函数名，`CallNativei` 的立即数是 u32 导入索引或待解析的 Native 函数名。调用符号不允许非零 addend，且 `CallNativei` 的数值索引必须适合 u32 字段；`InstructionEmitter::padInstruction` 已按操作码区分两种直接调用，与普通立即数的符号地址编码分开处理，具体规则见下文。

## 完整指令示例

`add64rri sp, sp, 16` 的操作码是 `001b`，其布局为：

```text
Opcode:u16 | Destination:(RegisterId:u8, ByteOffset:i64) | LHS:(RegisterId:u8, ByteOffset:i64) | RHS:i64
```

该指令总共占 28 字节；目标和左来源的偏移均为零：

```text
1b 00 | 01 00 00 00 00 00 00 00 00 | 01 00 00 00 00 00 00 00 00 | 10 00 00 00 00 00 00 00
```

将右来源改为 `-16` 时，立即数的小端字节为 `f0 ff ff ff ff ff ff ff`。

## 无条件跳转与返回

四种跳转均不包含条件码，也不依赖隐式状态：

| C++ 操作码 | 文本 | 编码 | 操作数字段 | 总长度 |
| --- | --- | --- | --- | --- |
| `Jmp` | `jmp done` | `002b` | `RegisterId:u8 = 02` 加 PC 相对 `ByteOffset:i64` | 11 字节 |
| `Jmpi` | `jmpi 16` 或 `jmpi @target` | `003f` | PC 相对 `TargetOffset:i64` | 10 字节 |
| `Jmpr` | `jmpr pc + 16` | `0040` | `r`：`RegisterId:u8` 加 `ByteOffset:i64` | 11 字节 |
| `Jmpm` | `jmpm [bp + 8]` | `0041` | `m`：基址寄存器编号加八字节偏移位模式 | 11 字节 |

`Jmp` 在逻辑模块中只接受同一函数的目标基本块，发射后使用当前指令起始 PC 加相对位移，不读取目标内存。`Jmpi` 的数值立即数也是相对当前 PC 的有符号位移；函数符号目标复用直接 VM 调用的静态 PC 相对解析，addend 必须为零。`Jmpr` 直接将寄存器值加偏移作为目标；`Jmpm` 先计算有效地址，再从该地址读取八字节目标宿主地址。目标取自运行时寄存器或内存时，镜像只保存相应操作数字段。

已删除操作码的编号 `001f` 至 `002a`、`002c` 至 `0039` 保留未分配。`ret` 仍为 `003a`，没有操作数，长度为两字节。当前解释器将最外层 Ret 作为本次执行成功结束；普通调用帧的保存与恢复协议见 [VM 调用约定](VM-Calling-Convention.md)，嵌套调用执行仍待实现。

## 显式比较条件跳转编码

`compare_branch.def` 以 `COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Width, Operator, Signed)` 集中记录 80 种比较分支。编码范围为 `0042` 至 `0091`，依次按 `je`、`jne`、`jlt`、`jle`、`jgt`、`jge`、`jltu`、`jleu`、`jgtu`、`jgeu` 排列；每组依次为位宽 8、16、32、64，各位宽先 `mmb` 后 `mib`。

| 前缀 | 比较 |
| --- | --- |
| `je`、`jne` | 等于、不等于，按位比较 |
| `jlt`、`jle`、`jgt`、`jge` | 有符号小于、小于等于、大于、大于等于 |
| `jltu`、`jleu`、`jgtu`、`jgeu` | 对应的无符号关系 |

```text
Mmb: Opcode:u16 | LHS:m | RHS:m | Target:(RegisterId:u8 = 02, ByteOffset:i64)
Mib: Opcode:u16 | LHS:m | RHS:i<Width> | Target:(RegisterId:u8 = 02, ByteOffset:i64)
```

`Mmb` 固定为 29 字节。`Mib` 为 `20 + Width / 8` 字节，位宽 8/16/32/64 对应长度 21/22/24/28 字节。两个 `m` 字段都是九字节有效地址描述，来源内存的实际读取宽度由 opcode 决定；立即数在镜像中只占该位宽，负数保存为相同位宽的二进制补码。

例如 `je64mib [bp + 16], 42, done` 的 opcode 为 `0049`，`jlt64mib` 为 `0059`，`jltu64mib` 为 `0079`。目标块按与 `Jmp` 相同的九字节 PC 相对形式发射，但放在整条指令的最后。目标位移相对当前指令起点；成立时执行 `PC = InstructionPC + ByteOffset`，否则执行 `PC = InstructionPC + InstructionBytes`。目标只接受同一函数中的基本块，不为每种比较再展开立即数、寄存器、内存目标变体。

## 函数调用目标与编码

`call` 和 `callNative` 指令族各有立即数与内存两种形式，共用文本名称，由操作数种类选择具体操作码。直接调用使用 `I` 形式的 `ImmediateOperand`，间接调用使用 `M` 形式的 `AddressOperand`；参数和结果不作为显式指令操作数。文本示例：

```text
call @sum
call 16
call [bp + 16]
call [@function_pointer_slot]
callNative @host_log
callNative 0
callNative [sp - 8]
callNative [@native_pointer_slot]
```

`Calli` 的数值立即数按 64 位位模式原样写入，表示相对当前 opcode 起点的有符号字节位移；符号立即数表示 VM 函数名，目标已布局时写入 `FunctionOffset - InstructionOffset`。前向引用在 `ExecutableEmissionContext::PendingStaticRelocationSymbol` 中登记位移字段偏移、函数名、opcode 起点、零 addend 和 `Register = PC`，追加八字节零，在模块发射结束时统一回填；字段前不写寄存器编号，也不生成加载地址重定位。`CallNativei` 的数值立即数按 32 位写入；符号立即数必须在统一的 `NativeSymbols` 中，通过 `ExecutableEmissionContext::getOrAddNativeSymbol` 按该名称复用导入并写入 u32 索引，不生成动态地址重定位，与普通地址取用共享导入表。逻辑符号名与宿主查找名相同，不经过别名映射。两种直接调用的符号 addend 均须为零；名称支持 `@"escaped name"` 转义形式。

链接时，直接 `call @name` 的目标必须是 VM 函数定义，支持本模块 Local 函数以及跨模块 External 函数；直接 `callNative @name` 的目标必须是当前输入通过 `declareSymbol` 声明且未被可见 VM 定义满足的函数符号。全局数据和错误调用类别都会被拒绝；本次链接中得到 VM 函数定义的声明也不能继续作为直接 Native 调用目标。数值目标不参与符号绑定；`CallNativei` 数值索引的 u32 合法性及符号调用的零 addend 由模块链接器检查，镜像中的指令边界和实际导入表范围仍待后续校验与执行阶段检查。

`Callm` 从有效地址处读取一个 64 位 VM 函数入口的宿主地址，再按 VM 调用协议转移至该地址；`CallNativem` 从有效地址处读取一个 64 位 native 可调用指针，该值不是 u32 导入索引。`[@pointer_slot]` 中的符号定位存放指针的内存，链接时按普通地址符号规则解析，不要求它是函数定义或 Native 函数声明；若该槽位于 VM 镜像内，定位槽的寻址仍可使用 PC 相对形式，槽内读出的目标值则是完整地址。内存地址允许有符号字节偏移，目标值及其可调用性仍须由运行时校验。统一地址表示不合并两种调用协议；native 指针的 adapter 与宿主 ABI 协议尚未实现，当前指令模型不表示任意 C 函数指针都可直接调用。

已实现的调用指令字节编码如下；解码仍待实现：

| C++ 操作码 | 文本名称 | 编码 | 操作数字段 | 总长度 |
| --- | --- | --- | --- | --- |
| `Calli` | `call` | `003b` | `TargetOffset:i64`，相对当前指令起始 PC 的有符号字节位移 | 10 字节 |
| `CallNativei` | `callNative` | `003c` | `DynamicImportSymbolIndex:u32`，引用 `ExecutableFile::DynamicImportSymbols` | 6 字节 |
| `Callm` | `call` | `003d` | `m`：统一寄存器相对形式，ZERO 基址表示绝对地址，读取 64 位 VM 函数入口宿主地址 | 11 字节 |
| `CallNativem` | `callNative` | `003e` | `m`：统一寄存器相对形式，ZERO 基址表示绝对地址，读取 64 位 native 可调用指针 | 11 字节 |

`Calli` 的目标为 `PC + TargetOffset`，后继地址为 `NextPC = PC + 10`；`CallNativei` 的后继地址为 `NextPC = PC + 6`。间接形式的后继地址为 `PC + 11`，符号内存地址按上文分别选择 PC 或 ZERO 基址，使用同一寄存器相对字段格式。函数名称不进入字节编码，直接 native 调用使用统一符号表索引，无需在调用指令中回填地址；同一符号的取地址用途则通过动态导入符号重定位填写地址。统一导入表不保存调用约定，调用描述及 adapter 协议仍待实现。符号调用目标随 Local 重命名一并改写，同名的兼容 Native 声明合并但不改写名称，文本读取只恢复逻辑操作数，不代替链接检查。

四条指令的类别都是 `InstructionCategory::Call`，没有显式结果目标，也不是基本块终结指令。`isCall()` 识别全部调用，`isNativeCall()` 识别两种 Native 调用。逻辑指令、构建、遍历、文本读写、链接接入和指令编码均已实现；调用执行与宿主 ABI adapter 尚未实现，参数、返回槽和可变参数调用预留区布局见 [VM 调用约定](VM-Calling-Convention.md)。

## C++ 指令模型

`MachineModule` 的临时文本读写由 `backend/vm/serialization` 提供。文本保留寄存器来源的完整 `i64` 偏移、数值立即数的完整 `u64` 位模式，或符号立即数的名称和完整 `i64` 偏移；本节的数值字节格式则由操作码位宽决定立即数字段的长度。

`opcode.def` 通过 `OPCODE_FORM(Kind, ...)` 集中定义操作数签名，`OPCODE` 记录引用其中的签名和指令类别。形式名称仅描述操作数类型及顺序，不包含指令语义或位宽，例如 `Add64rri` 引用 `Rri`，`Load64` 和 `Store64m` 共同引用 `Mm`。`instruction_form.h` 直接从同一份 `opcode.def` 生成 `InstructionForm` 枚举和 `FormTraits` 特化，每种签名只展开一次。每种形式提供 `OperandTuple` 和寄存器、地址、立即数、基本块的计数，包括 `BlockCount`；`None` 对应空元组，`B` 对应单个 `BasicBlockOperand`，`I`、`R` 和 `M` 分别对应单个 `ImmediateOperand`、`RegisterOperand` 和 `AddressOperand`。顺序来自签名定义；操作数计数用于一致性校验，不能单独推导顺序，例如 `Rrm` 和 `Mrr` 的计数相同而顺序不同。

`machine_instructions.h` 根据 `opcode.def` 为每个操作码生成 `OpcodeTraits`，将操作码绑定到对应的 `Form`、`BitWidth` 和操作数布局。`MachineInstructionImpl<Opcode>` 按值持有该布局的 `std::tuple`，并为每个现有操作码提供对应的指令别名，例如 `MachineAdd64rriInstruction`。构造函数要求操作数的数量和具体类型匹配布局，操作数存储随指令对象一起分配。

只有 Load、Store、Add 提供 `getDestination()`；具有目标和单个来源的形式提供 `getSource()`。Add 的 `getLHS()`、`getRHS()` 分别访问索引 1、2，条件跳转的同名访问器访问索引 0、1。具有块目标的跳转由 `getTarget()` 返回最后一个操作数的基本块指针，`getTargetOperand()` 返回块操作数引用；其他跳转和调用的目标访问器返回对应立即数、寄存器或地址操作数的常量引用。Ret 没有这些访问器。

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

`category()`、`hasDestination()`、`isBranch()`、`isCall()`、`isNativeCall()`、`isReturn()`、`isTerminator()` 描述指令语义；无条件跳转和比较条件跳转均为终结指令，后者还通过 `isConditionalBranch()` 标识。

具体指令通过 `MachineInstructionImpl` 构造函数绑定操作码与布局，该模板为 `final`，保证用于分派的具体类型一致。`MachineInstructionBuilder` 在将新指令交给基本块后设置其 `parent()`。指令对象禁止复制，使用具体类型销毁；基类没有虚函数或虚析构函数。构造函数在编译期约束操作数种类和数量，PC 写入限制、寄存器编号有效性、立即数范围等语义约束仍须由后续指令校验处理。

## 编码、解码与预解码边界

`InstructionEmitter::padInstruction` 已提供 opcode 写入、操作数字段分派、基本块目标、跳转和直接调用编码；解释器已实现三种 ADD 形式、四种无条件跳转、80 种显式比较条件跳转和最外层 Ret，公共解码接口仍待实现，也没有已解析字节指令的公共 API。当前模块发射流程完成全局布局、函数名入口偏移登记、基本块偏移登记、指令字节追加、重定位处理及显式入口和导出元数据生成。`MachineInstruction` 及操作数属于逻辑内存模型，`MachineModule` 文本读写不等同于指令字节编码。独立模块 validator 尚未实现。

指令编码有效与运行时可执行是不同层次的检查。VM 符号须由链接阶段解析，宿主导入由加载阶段绑定；寄存器写权限、指令边界、有效地址计算、访存范围和栈规则也须按执行语义检查。当前操作数构造函数不完成这些检查。

加载已实现，独立预解码阶段仍待实现。`ExecutableFile` 保存扁平的未绑定镜像，不携带代码与数据的段范围；后续预解码如何识别代码范围及建立指令边界仍待确定。运行时表示必须与逻辑符号名称、指令字节编码和宿主绑定保持清晰边界；任何宿主地址缓存均不得作为进程无关的可执行文件内容保存。加载器须在地址稳定的运行时存储中完成镜像基址和动态导入地址绑定，执行器以统一宿主地址处理指针值、寄存器和有效地址。
