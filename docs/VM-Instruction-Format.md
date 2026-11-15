# VM 指令编码格式讨论稿

本文记录 Ink VM 指令及操作数编码。逻辑指令、模块链接、镜像布局、发射与加载已实现，解释器支持部分指令执行，公共解码器仍待实现。操作码来自 `src/include/ink/backend/vm/instruction/opcode.def`，寄存器编号来自 `register.def`。调用族统一为 `Calli`、`Callm`，文本都写 `call`；Native stub 与 Ink 函数的差别由执行时的目标地址分派处理。

当前实现提供 `Operand` 基类以及 `RegisterOperand`、`AddressOperand`、`ImmediateOperand`、`BasicBlockOperand` 四种逻辑操作数，并通过强类型指令布局将操作数接入 `MachineInstruction`。基本块操作数在内存中持有目标块指针；调用复用立即数和地址操作数，分别表达直接目标和存放间接目标的内存位置。[VM 内存模型与指令构建](VM-Module.md) 提供模块、函数、基本块的所有权结构和 `MachineInstructionBuilder`，供 lowering 构建逻辑指令。[模块链接器](VM-Linker.md) 已实现逻辑模块合并、符号解析和深克隆，`executable` 目录提供镜像模型与结构校验。加载和部分执行分派已实现，预解码缓存及其余指令执行仍待实现。

发射代码分为 `ExecutableEmitter`、`FunctionEmitter` 和 `InstructionEmitter`。共享上下文保存模块布局、`ImportSymbols` 和待解析重定位；未被本镜像定义满足的声明形成导入候选，可以在加载时绑定到其他 Ink 库或 Native。指令采用八字节头和逐操作数八字节槽。基本块目标和直接跳转保持 PC 相对位移；普通符号地址和直接调用地址通过镜像基址或动态导入重定位填写。

`ExecutableEmitter::emit` 必须显式传入模块、入口函数名和导出名称列表。入口必须是具名且包含指令的 VM 函数，导出仅选择列表中的 VM 函数或全局变量；发射器在布局和静态重定位完成后生成 `EntryOffset` 和 `ExportSymbols`，每次发射会清理旧元数据。调用示例与完整选择规则见 [VM 可执行镜像模型](VM-Executable.md#显式入口与导出选择)。

## 通用布局

```text
Header:u64 | Operand0:u64 | Operand1:u64 | ...
```

`u8`、`u16`、`u32`、`u64` 分别表示相应位宽的无符号整数，`i64` 表示 64 位二进制补码。下文的字节序列、操作码和寄存器编号均以十六进制表示，位宽及长度以十进制表示。VM 固定使用小端序，操作码、偏移、立即数和地址重定位字段均按小端序编码，镜像不保存字节序配置。磁盘格式仍待约定。函数、基本块和指令起点均对齐到八字节；对齐填充在登记函数与基本块偏移前写入。头和每个语义操作数都占一个八字节槽，不使用 C++ 类或结构体的宿主内存布局，也不附加长度字段。指令长度固定为 `8 * (1 + OperandCount)`，仅由 opcode 决定。旧的紧排字节码不兼容此布局，必须重新发射；当前没有新旧格式自动识别或转换。

头的位布局为：

| 位范围 | 内容 |
| --- | --- |
| `0..15` | opcode |
| `16..23` | 语义操作数 0 的寄存器编号 |
| `24..31` | 语义操作数 1 的寄存器编号 |
| `32..39` | 语义操作数 2 的寄存器编号 |
| `40..63` | 保留，必须为零 |

立即数及未使用的寄存器槽统一填 `ZERO = 03`；基本块目标使用 `PC = 02`。`instruction_encoding.h` 集中提供头编码、解码、操作数槽偏移与指令长度。运行时须保证实际指令地址对齐；指令对齐不保证 VM 数据地址对齐，任意数据内存读写仍使用按字节复制。

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

操作数计数包含所有显式操作数。指令长度由操作码确定的操作数数量决定，符号解析不会改变指令长度。

## 寄存器操作数 `r`

```text
Header.RegisterId[OperandIndex]:u8
Payload[OperandIndex]: ByteOffset:i64
```

寄存器编号保存在头中，偏移独占一个八字节操作数槽。作为来源时，其值为 `RegisterValue + ByteOffset`，不读取内存；`ByteOffset` 是有符号 64 位字节偏移，默认为零。文本 `pc + 2` 对应 `RegisterOperand(RegisterId::PC, 2)`，`sp - 8` 对应 `RegisterOperand(RegisterId::SP, -8)`。

| 编码 | 寄存器 |
| --- | --- |
| `00` | `BP` |
| `01` | `SP` |
| `02` | `PC` |
| `03` | `ZERO`，文本拼写为 `zero`，恒为零且不可写 |

当前普通寄存器来源接受 BP、SP、PC、ZERO；寄存器加法的目标仅接受 BP、SP，目标的 `ByteOffset` 必须为零，PC 和 ZERO 均不可作为普通写入目标。PC 保存当前指令起始位置的宿主地址，`pc + 2` 表示该地址之后第二个字节的地址；BP、SP 保存栈中的宿主地址。ZERO 每次读取都为零，`zero + N` 表示偏移 N 的位模式，不改变 ZERO。

`InstructionEmitter::padOperand(const RegisterOperand& Operand)` 与寄存器形式的地址操作数共用 `padRegister`，追加八字节偏移；寄存器编号由 `padInstruction` 写入头中。两者的槽布局相同；操作码及操作数位置决定取寄存器值加偏移，还是访问该地址处的内存。

## 地址操作数 `m`

地址操作数保存寄存器或符号链接名，以及有符号字节偏移。加载、存储、加法、比较和间接调用使用它定位内存中的值，访问位宽由操作码决定。符号形式通过 `core::Name` 保存名称，例如文本 `[@counter + 8]`。读取符号地址本身时使用立即数形式 `@counter + 8`。跳转的内存模型保存基本块指针，指令发射将其转换为 PC 相对地址字段；该编码地址用于控制转移，不读取该地址处的内存内容。

```text
Header.RegisterId[OperandIndex]:u8
Payload[OperandIndex]: OffsetBits:u64
EffectiveAddress = Registers[RegisterId] + OffsetBits
```

地址操作数的偏移固定占一个八字节槽，基址编号保存在头中，基址只接受 BP、SP、PC、ZERO，未知编号不是有效基址。BP、SP、PC 的有符号字节位移按 `i64` 补码保存为 `OffsetBits`；ZERO 相对字段可保存完整的 `u64` 绝对地址位模式，无需将地址限制在 `i64` 正数范围内。执行器可直接按寄存器编号索引基址并执行统一的 64 位地址计算，无需按 PC 相对或绝对寻址分支；地址运算、范围与权限检查仍须独立完成。`ff` 不再表示绝对地址基址，也不是有效寄存器编号。

符号内存地址统一使用 ZERO 基址和八字节地址占位字段。本镜像内符号生成 `ImageBaseAddressRelocation`，外部符号生成 `DynamicImportSymbolRelocation`，加载后字段保存完整运行时地址加 addend。函数和数据共享地址重定位。寄存器相对内存寻址不变，基本块目标仍使用 PC 相对位移。进程内的 `core::Name` 索引不进入指令字节。

`InstructionEmitter::padOperand(const AddressOperand& Operand, std::uint64_t InstructionOffset)` 追加内存地址字段。寄存器形式保留基址及偏移；符号形式使用 ZERO 基址。本镜像前向引用在 `PendingStaticRelocationSymbol` 中记录名称、字段偏移、addend 和 `Register = ZERO`，布局完成后生成镜像基址地址重定位；外部符号按名称复用导入并立即生成动态导入重定位。`Jmpi` 的符号目标仍使用 `Register = PC` 静态求差，同一符号的不同引用分别记录。

下面的竖线只用于说明字段边界，不属于字节流：

| 地址 | 头中寄存器编号 | 操作数槽的小端序字节 |
| --- | --- | --- |
| `[bp + 16]` | `00` | `10 00 00 00 00 00 00 00` |
| `[sp - 8]` | `01` | `f8 ff ff ff ff ff ff ff` |
| `[zero + 16]` | `03` | `10 00 00 00 00 00 00 00` |

## 立即数操作数 `i`

逻辑立即数支持数值和符号地址两种形式。`ImmediateValueKind::Numeric` 保存完整的 `u64` 位模式；`ImmediateValueKind::Symbol` 保存 `core::Name` 链接名和有符号 `i64` 字节偏移。数值立即数的最终编码没有自身的类型标签或长度字段，`opcode.def` 的 `Width` 决定有效低位，截断后统一零扩展到八字节槽：

| `Width` | 有效低位字节数 | 槽字节数 |
| --- | --- | --- |
| `8` | 1 | 8 |
| `16` | 2 | 8 |
| `32` | 4 | 8 |
| `64` | 8 | 8 |

`ImmediateOperand::getValueKind()` 区分两种形式。`bits()` 只在数值形式返回位模式；`getSymbol()` 和 `getByteOffset()` 只在符号形式返回名称与偏移，各访问器用 `std::optional` 表示不适用的情况。负数值先截断为相应位宽的二进制补码，再零扩展；执行器按 opcode 指定的位宽与符号语义解释低位。当前寄存器目标的 64 位加法将右来源解释为有符号 64 位字节位移；普通整数结果的加法仍遵循操作码规定的位宽回绕语义。

符号立即数表示 `AddressOf(Symbol) + ByteOffset`，不读取该地址处的内存。文本采用 `@name` 或 `@"escaped name"`，并可附加 `+ Offset` 或 `- Offset`。例如 `add64mii [bp + 16], @counter + 8, 1` 的左来源是 counter 地址加 8。文本读写保留完整的名称和 `i64` 偏移。普通符号取地址统一要求 64 位字段。VM 符号通过 `ImageBaseAddressRelocation` 在加载时填写 `ImageBase + TargetOffset + Addend`；宿主符号通过动态导入符号地址重定位填写绑定后的宿主地址加 Addend。两者在加载后都得到完整的运行时地址，不能截断保留地址低位。Native 函数地址的可调用表示及 adapter 协议尚未确定。

`InstructionEmitter::padOperand(const ImmediateOperand& Operand, std::uint32_t Width)` 已实现普通立即数字段的顺序追加。数值形式接受 Width 为 8、16、32、64，保留目标位宽的低位并零扩展到完整八字节槽，负数的低位按相应位宽的补码编码。符号形式只接受 Width 为 64，统一追加八字节零。已布局的 VM 符号生成包含字段起点、目标符号镜像偏移及 addend 的 `ImageBaseAddressRelocation`；未布局的 VM 符号先在上下文的 `PendingStaticRelocationSymbol` 中保存字段起点、名称、addend 和 `Register = ZERO`，最终解析根据 ZERO 基址生成镜像基址地址重定位；这里的寄存器编号仅属于临时记录，不写入普通立即数字段。`padGlobals` 只负责布局与登记符号偏移，`ExecutableEmitter::emit` 依次执行 `padGlobals`、`padFunction` 后统一执行最终解析；`padFunction` 在每个函数发射前先完成八字节对齐，再以当前镜像字节数登记函数名的入口偏移，并移除其未解析符号记录，再按函数、基本块和指令顺序发射字节。该登记用于符号引用解析；布局和静态重定位完成后，`ExecutableEmitter::emit` 按显式入口名称与导出列表填写 `EntryOffset` 和 `ExportSymbols`。外部符号生成 `DynamicImportSymbolRelocation`，按 Name 复用导入；普通符号立即数字段前不追加寄存器编号。未知符号、镜像定义与外部导入冲突或非法位宽报告内部编译错误。加载器负责检查实际绑定地址与 addend 的运算并按小端序填写字段；加载已由 `Loader` 实现。

直接调用复用 `ImmediateOperand`，数值形式是完整 u64 目标地址，符号形式在加载时填写函数地址；符号 addend 必须为零。调用目标与普通符号地址采用相同的镜像基址或动态导入地址重定位，不再保存 native 导入索引。

## 完整指令示例

`add64rri sp, sp, 16` 的操作码是 `001b`，其布局为：

```text
Header:(Opcode:u16, DestinationReg:u8, LHSReg:u8, RHSReg:u8, Reserved:u24) | DestinationOffset:i64 | LHSOffset:i64 | RHS:i64
```

该指令总共占 32 字节；目标和左来源的偏移均为零：

```text
1b 00 01 01 03 00 00 00 | 00 00 00 00 00 00 00 00 | 00 00 00 00 00 00 00 00 | 10 00 00 00 00 00 00 00
```

将右来源改为 `-16` 时，立即数的小端字节为 `f0 ff ff ff ff ff ff ff`。

## 无条件跳转与返回

四种跳转均不包含条件码，也不依赖隐式状态：

| C++ 操作码 | 文本 | 编码 | 操作数字段 | 总长度 |
| --- | --- | --- | --- | --- |
| `Jmp` | `jmp done` | `002b` | `RegisterId:u8 = 02` 加 PC 相对 `ByteOffset:i64` | 16 字节 |
| `Jmpi` | `jmpi 16` 或 `jmpi @target` | `003f` | PC 相对 `TargetOffset:i64` | 16 字节 |
| `Jmpr` | `jmpr pc + 16` | `0040` | `r`：`RegisterId:u8` 加 `ByteOffset:i64` | 16 字节 |
| `Jmpm` | `jmpm [bp + 8]` | `0041` | `m`：基址寄存器编号加八字节偏移位模式 | 16 字节 |

`Jmp` 在逻辑模块中只接受同一函数的目标基本块，发射后使用当前指令起始 PC 加相对位移，不读取目标内存。`Jmpi` 的数值立即数也是相对当前 PC 的有符号位移；函数符号目标使用静态 PC 相对解析，addend 必须为零。`Jmpr` 直接将寄存器值加偏移作为目标；`Jmpm` 先计算有效地址，再从该地址读取八字节目标宿主地址。目标取自运行时寄存器或内存时，镜像只保存相应操作数字段。

已删除操作码的编号 `001f` 至 `002a`、`002c` 至 `0039`、`003c` 和 `003e` 保留未分配。`ret` 为 `003a`，只有八字节头；普通返回恢复旧 BP 和返回 PC，最外层返回结束执行，布局见 [VM 调用约定](VM-Calling-Convention.md)。

## 显式比较条件跳转编码

`compare_branch.def` 以 `COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Width, Operator, Signed)` 集中记录 80 种比较分支。编码范围为 `0042` 至 `0091`，依次按 `je`、`jne`、`jlt`、`jle`、`jgt`、`jge`、`jltu`、`jleu`、`jgtu`、`jgeu` 排列；每组依次为位宽 8、16、32、64，各位宽先 `mmb` 后 `mib`。

| 前缀 | 比较 |
| --- | --- |
| `je`、`jne` | 等于、不等于，按位比较 |
| `jlt`、`jle`、`jgt`、`jge` | 有符号小于、小于等于、大于、大于等于 |
| `jltu`、`jleu`、`jgtu`、`jgeu` | 对应的无符号关系 |

```text
Mmb: Header:u64 | LHSOffset:u64 | RHSOffset:u64 | TargetOffset:i64
Mib: Header:u64 | LHSOffset:u64 | ZeroExtendedRHS:u64 | TargetOffset:i64
```

`Mmb` 与 `Mib` 均固定为 32 字节。头中按位置保存左来源、右来源和目标块的寄存器编号；`Mib` 的右来源编号固定为 ZERO，目标块编号固定为 PC。三个八字节槽分别位于指令偏移 8、16、24。来源内存的实际读取宽度由 opcode 决定；窄立即数先保留该位宽的低位，再零扩展到八字节槽。

例如 `je64mib [bp + 16], 42, done` 的 opcode 为 `0049`，`jlt64mib` 为 `0059`，`jltu64mib` 为 `0079`。目标块按与 `Jmp` 相同的八字节 PC 相对位移槽形式发射，但放在整条指令的最后。目标位移相对当前指令起点；成立时执行 `PC = InstructionPC + ByteOffset`，否则执行 `PC = InstructionPC + InstructionBytes`。目标只接受同一函数中的基本块，不为每种比较再展开立即数、寄存器、内存目标变体。

## 函数调用目标与编码

调用族只保留 `Calli` 和 `Callm`，文本统一写 `call`。直接调用使用 `ImmediateOperand`，间接调用使用 `AddressOperand`；参数和返回槽不作为显式操作数。

```text
call @sum
call @host_log
call [bp + 16]
call [@function_pointer_slot]
```

`Calli` 的数值目标按完整 u64 地址位模式写入。符号目标要求零 addend：本镜像函数生成 `ImageBaseAddressRelocation`，外部函数生成 `DynamicImportSymbolRelocation`。前向本镜像函数先记录 ZERO 地址静态引用，布局完成后生成镜像基址重定位。导入索引只存在于重定位元数据，不写入调用操作数。

链接器允许 `call @name` 绑定可见 VM 函数定义或显式函数声明，拒绝数据目标；未满足的声明在加载时可由其他 Ink 库或 Native stub 提供。`Callm` 的符号只定位指针槽，不要求该槽本身为函数符号；槽内目标的执行支持仍待实现。

| C++ 操作码 | 文本 | 编码 | 操作数字段 | 总长度 |
| --- | --- | --- | --- | --- |
| `Calli` | `call` | `003b` | `TargetAddress:u64`，完整运行时函数地址 | 16 字节 |
| `Callm` | `call` | `003d` | 寄存器相对内存地址，槽内保存完整函数地址 | 16 字节 |

`003c`、`003e` 保留未分配，旧 `callNative` 文本被拒绝。两条指令均属于 `InstructionCategory::Call`，`isCall()` 识别两者，没有显式结果目标，也不是基本块终结指令。

解释器先实现 `Calli`。目标位于有效代码段且按八字节对齐时直接走 Ink 快路径，不查询 hash set；保存旧 BP 与 `NextPC = PC + 16` 后转入 VM 函数。段外目标必须命中从 `NativeSymbolAddresses` 建立的 stub 地址 hash set，才按 `void(Frame*)` 调用，不建立 Ink 保存区；未命中时报执行错误。普通 `Ret` 恢复调用方，最外层 `Ret` 结束本次执行。Native 参数及返回槽由调用方按已有 stub 契约准备，完整调用区自动生成、通用宿主 ABI adapter 和 `Callm` 执行仍待实现。

## C++ 指令模型

`MachineModule` 的临时文本读写由 `backend/vm/serialization` 提供。文本保留寄存器来源的完整 `i64` 偏移、数值立即数的完整 `u64` 位模式，或符号立即数的名称和完整 `i64` 偏移；本节的数值字节格式则由操作码位宽决定立即数槽中有效低位的宽度。

`opcode.def` 通过 `OPCODE_FORM(Kind, ...)` 集中定义操作数签名，`OPCODE` 记录引用其中的签名和指令类别。形式名称仅描述操作数类型及顺序，不包含指令语义或位宽，例如 `Add64rri` 引用 `Rri`，`Load64` 和 `Store64m` 共同引用 `Mm`。`instruction_form.h` 直接从同一份 `opcode.def` 生成 `InstructionForm` 枚举和 `FormTraits` 特化，每种签名只展开一次。每种形式提供 `OperandTuple` 和寄存器、地址、立即数、基本块的计数，包括 `BlockCount`；`None` 对应空元组，`B` 对应单个 `BasicBlockOperand`，`I`、`R` 和 `M` 分别对应单个 `ImmediateOperand`、`RegisterOperand` 和 `AddressOperand`。顺序来自签名定义；操作数计数用于一致性校验，不能单独推导顺序，例如 `Rrm` 和 `Mrr` 的计数相同而顺序不同。

`machine_instructions.h` 根据 `opcode.def` 为每个操作码生成 `OpcodeTraits`，将操作码绑定到对应的 `Form`、`BitWidth` 和操作数布局。`MachineInstructionImpl<Opcode>` 按值持有该布局的 `std::tuple`，并为每个现有操作码提供对应的指令别名，例如 `MachineAdd64rriInstruction`。构造函数要求操作数的数量和具体类型匹配布局，操作数存储随指令对象一起分配。

只有 Load、Store、Add 提供 `getDestination()`；具有目标和单个来源的形式提供 `getSource()`。Add 的 `getLHS()`、`getRHS()` 分别访问索引 1、2，条件跳转的同名访问器访问索引 0、1。具有块目标的跳转由 `getTarget()` 返回最后一个操作数的基本块指针，`getTargetOperand()` 返回块操作数引用；其他跳转和调用的目标访问器返回对应立即数、寄存器或地址操作数的常量引用。Ret 没有这些访问器。

```cpp
#include "ink/backend/vm/instruction/machine_instructions.h"

namespace ink::vm
{
    const MachineAdd64rriInstruction Add(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(16));
    const RegisterOperand& Destination = Add.getDestination();
    const RegisterOperand& LHS = Add.getLHS();
    const ImmediateOperand& RHS = Add.getRHS();

    const MachineInstruction& Base = Add;
    const Operand* First = Base.getOperand(0);
    const Operand* Missing = Base.getOperand(3); // nullptr: this instruction has three operands.
} // namespace ink::vm
```

`MachineInstruction::operandCount()` 返回包含目标在内的操作数数量；`getOperand(Index)` 按相同的语义顺序返回 `const Operand* `，越界返回 `nullptr`。统一访问在实现文件中通过操作码分派到对应的具体指令布局；这是一层供编码器、打印器和校验器使用的模型遍历接口，不规定未来执行器的热路径。

`category()`、`hasDestination()`、`isBranch()`、`isCall()`、`isReturn()`、`isTerminator()` 描述指令语义；无条件跳转和比较条件跳转均为终结指令，后者还通过 `isConditionalBranch()` 标识。

具体指令通过 `MachineInstructionImpl` 构造函数绑定操作码与布局，该模板为 `final`，保证用于分派的具体类型一致。`MachineInstructionBuilder` 在将新指令交给基本块后设置其 `parent()`。指令对象禁止复制，使用具体类型销毁；基类没有虚函数或虚析构函数。构造函数在编译期约束操作数种类和数量，PC 写入限制、寄存器编号有效性、立即数范围等语义约束仍须由后续指令校验处理。

## 编码、解码与预解码边界

`InstructionEmitter::padInstruction` 已提供 opcode 写入、操作数字段分派、基本块目标、跳转和直接调用编码；解释器已实现三种 ADD 形式、四种无条件跳转、80 种显式比较条件跳转、地址型 Calli 及普通和最外层 Ret，公共解码接口仍待实现，也没有已解析字节指令的公共 API。当前模块发射流程完成全局布局、函数名入口偏移登记、基本块偏移登记、指令字节追加、重定位处理及显式入口和导出元数据生成。`MachineInstruction` 及操作数属于逻辑内存模型，`MachineModule` 文本读写不等同于指令字节编码。独立模块 validator 尚未实现。

指令编码有效与运行时可执行是不同层次的检查。VM 符号须由链接阶段解析，宿主导入由加载阶段绑定；寄存器写权限、指令边界、有效地址计算、访存范围和栈规则也须按执行语义检查。当前操作数构造函数不完成这些检查。

加载已实现，独立预解码阶段仍待实现。`ExecutableFile` 保存扁平的未绑定镜像，不携带代码与数据的段范围；后续预解码如何识别代码范围及建立指令边界仍待确定。运行时表示必须与逻辑符号名称、指令字节编码和宿主绑定保持清晰边界；任何宿主地址缓存均不得作为进程无关的可执行文件内容保存。加载器须在地址稳定的运行时存储中完成镜像基址和动态导入地址绑定，执行器以统一宿主地址处理指针值、寄存器和有效地址。
