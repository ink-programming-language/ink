# VM 运行时设计讨论稿

本文讨论 Ink 底层虚拟机的执行状态和栈操作。函数调用布局单独记录在 [VM 调用约定](VM-Calling-Convention.md)；下文的运行时建议不表示执行器已经实现。

## 已确认的方向

1. VM 提供 `pc`、`bp`、`sp` 三个地址寄存器，以及隐式算术状态寄存器 `eflags`。
2. 函数参数通过栈传递。
3. 栈空间通过支持寄存器操作数的 `add` 指令调整 `sp`，基本状态变化为 `sp := sp + x`。

逻辑寄存器属于 VM 的可见执行状态，与宿主 CPU 的寄存器分配无关。解释器计算有效地址、解码操作数时使用的宿主临时变量不增加 VM 的逻辑寄存器。

[VM 内存模型与指令构建](VM-Module.md) 已提供 `MachineModule`、`MachineFunction`、`MachineBasicBlock` 和 `MachineInstructionBuilder`，用于持有和构建尚未编码的 VM 指令，并记录统一符号声明、可写或只读全局数据和栈帧描述。[模块链接器](VM-Linker.md) 已实现逻辑模块合并、符号绑定和 Native 符号推导，`ExecutableFile` 提供镜像模型和结构校验。指令编码、镜像布局与发射已实现，解码、加载器及执行器仍待实现；下文描述运行时规划。

`interpreter` 目录中的 `Interpreter` 持有 `Loader`，后者借用所属解释器。`Interpreter::loadLibrary(ExecutableFile)` 接收并持有镜像，将 `Image` 追加到 `CodeSegment` 后先调用 `loadExportSymbol(const LibraryDescriptor &)`，再调用 `Loader::loadLibrary(CodeSegment, ImageBase, ExecutableFile &)`，其中 `ImageBase` 保存追加前的代码段字节偏移。解释器分配从 1 开始且不复用的 `uint32_t` handle，并保存 handle 到 `LibraryDescriptor` 的映射；描述符包含 `ImageBase` 和 `ExecutableFile`，handle 空间耗尽时返回 0。`loadExportSymbol` 遍历库的 `ExportSymbols`，以 `core::Name` 为键，将 `ImageBase + Export.Offset` 写入 `ExportSymbolMap`；同名记录直接覆盖，其他记录保留。当前表中地址是以 `CodeSegment` 开头为零点的绝对字节位置，代码段扩容不改变这些数值；`getExportSymbolMap` 提供只读查询。`findExecutable` 提供只读查询，未知 handle 返回 nullptr。`tools/interpreter` 中的 `ink-interpreter` 工具持有解释器实例，将输入文件原始字节读入 `ExecutableFile::Image` 后调用 `loadLibrary`，输入为 `-` 时读取标准输入。可执行文件解码器尚未实现，此时不读取入口、导入或重定位元数据；Loader 的绑定接口目前为空，尚不进行重定位、导入绑定或执行。

## 寄存器与符号操作数

`register.def` 通过 `REGISTER(Kind, Encoding, Spelling)` 定义寄存器，`register.h` 据此提供底层类型为 `std::uint8_t` 的 `RegisterId`：

| 寄存器 | `RegisterId` | 编号 | 文本名称 |
| --- | --- | --- | --- |
| 帧基址 | `BP` | `0` | `bp` |
| 栈顶 | `SP` | `1` | `sp` |
| 程序计数器 | `PC` | `2` | `pc` |
| 算术标志 | `EFLAGS` | `3` | `eflags` |

寄存器值使用上述独立编号，符号引用则通过 `core::Name` 保存链接名。符号名称与寄存器编号属于不同的表示，文本用 `@name` 或 `@"escaped name"` 标识符号。EFLAGS 的编号用于识别隐式状态；当前不能将其作为普通 `r` 来源、算术目标或内存基址。

指令中的寄存器来源保存寄存器编号和有符号 `ByteOffset`，使用 `RegisterValue + ByteOffset` 的值，不访问内存。例如 PC 为 2、SP 为 0 时，`add64rrr bp, sp, pc + 2` 得到 BP 为 4；寄存器目标必须为 BP 或 SP，且偏移为零。地址操作数用于定位内存中的值，例如 `[bp + 16]` 或 `[@counter + 8]`。立即数可以保存数值，也可以保存符号地址值，例如 `@counter + 8`。内存模型还提供持有目标基本块指针的 `BasicBlockOperand`，跳转构建时直接引用同一函数的目标块。[VM 指令编码格式讨论稿](VM-Instruction-Format.md) 约定 `r` 与寄存器形式的 `m` 共用九字节的寄存器编号和偏移字段，由指令语义区分寄存器值与内存访问；操作数字段发射共用 `padRegister`，完整指令发射与执行仍待实现。按该方案，后续镜像生成阶段需要将 VM 符号内存地址和基本块目标转换为 PC 相对字段，将 VM 符号地址立即数转换为镜像相对偏移；宿主数据内存地址使用独立的 `ff` 基址标记及八字节重定位字段。

VM 与其他 Ink 组件统一使用 `InkVersion`，默认值为 `0.0.1`，可通过 `INK_VERSION` 配置。当前指令集包含 EFLAGS、CMP、跳转、RET，以及 `call`、`callNative` 的立即数与内存操作数形式。函数调用的 PC 相对位移、native 导入索引和间接寻址编码见 [VM 指令编码格式](VM-Instruction-Format.md#函数调用目标与编码)。

## 算术状态与控制流

EFLAGS 使用 32 位值，当前定义 CF（bit 0）、ZF（bit 6）、SF（bit 7）、OF（bit 11），其余位保留为零。执行语义规定所有 ADD 和 CMP 隐式覆盖四位；CMP 按对应位宽计算 `LHS - RHS` 并丢弃结果。普通加载、存储、跳转和 RET 保持标志不变。`eflags.h` 仅定义类型、标志掩码和 `conditionEFlagsMask()` 所表达的依赖元数据，不实现标志值生成或条件判断。当前尚没有 VM 执行状态、执行循环或这些运行时计算函数。

跳转包括 `jmp`，以及 `je/jne`、`jb/jbe/ja/jae`、`jl/jle/jg/jge`、`jo/jno`、`js/jns`。无符号大小关系使用 CF、ZF，有符号大小关系使用 SF、OF、ZF；具体条件见 [指令编码格式](VM-Instruction-Format.md)。跳转在内存中保存非空的目标基本块指针，源块和目标块须属于同一函数；文本使用 `jne failure`、`jmp done`，支持函数内前向、后向和自引用。指令插入或块布局变化保留目标身份，调用方不需要手工维护内存中的跳转位移。

镜像生成阶段根据最终布局，将目标块转换成相对当前跳转指令起始 PC 的有符号字节位移。现有编码沿用通用寄存器地址形式，长度为 11 字节；执行约定为条件满足时转移到 `PC + ByteOffset`，不读取目标地址处的内存，条件不满足时执行 `NextPC`。若条件分支的另一条路径需要转移到不同位置，可在其后放置一条 `jmp`。目标块布局、位移计算和指令编码已实现，解码及指令执行仍待实现。

算术、比较和条件跳转的隐式 EFLAGS 依赖由指令元数据暴露给 lowering 和后续调度。不能把另一条写标志的指令插入 CMP 与使用其结果的跳转之间。flags 按 caller-clobbered 处理，调用方不能跨函数调用依赖原有比较结果；ABI 不隐式保存或恢复标志。

## 寄存器和栈：建议约定

建议以字节为地址、偏移和长度的单位，栈向高地址增长，`sp` 指向栈顶之后的首个空闲字节。

| 寄存器 | 建议含义 |
| --- | --- |
| `pc` | 当前指令起始位置；顺序执行成功后移至下一条指令，分支、调用和返回显式改变它。 |
| `bp` | 当前函数栈帧的固定基址；函数体内预留临时空间或准备下一次调用时保持稳定。 |
| `sp` | 当前有效栈空间的末端；有效区间为 `[StackBegin, sp)`，可用容量为 `[sp, StackEnd)`。 |
| `eflags` | 最近一次算术或比较指令产生的 CF、ZF、SF、OF，由条件跳转隐式读取。 |

现有地址偏移和地址寄存器运算使用 64 位宽度，初版运行时可以沿用这一宽度，`bp` 和 `sp` 使用完整的 64 位寄存器操作。`pc` 的写入仅由控制转移指令完成。VM 地址的数值表示不直接暴露宿主指针；宿主栈缓冲区增长或移动时，已经形成的 VM 栈地址仍应保持有效。

每次调用拥有独立的帧，因此递归调用不会覆盖上一层调用的参数和局部变量。函数内的长期槽位建议使用 `bp + 固定偏移` 访问，短期的调用准备区可以使用 `sp` 相对寻址。

## 栈空间调整：建议约定

以 `sp` 为目标的寄存器加法直接调整栈顶。下例中的 `rri` 按目标、左来源、右来源的顺序表示两个寄存器操作数和一个立即数操作数：

```text
add64rri sp, sp, 16     // 预留 16 字节
add64rri sp, sp, -16    // 回收 16 字节
```

建议写入 `sp` 时将来源立即数按有符号字节增量解释。正增量预留空间，负增量回收空间，零增量保持栈顶不变。预留的空间不初始化；回收的空间不再属于有效栈空间。

执行前按数学意义计算新的地址，检查地址计算没有溢出，且新栈顶位于 `[StackBegin, StackEnd]` 内；通过检查后一次性更新 `sp` 和 EFLAGS。成功时按两个来源的 64 位位模式计算算术标志，CF 与地址检查失败是不同的概念。例如负增量的位模式参与无符号加法时可以产生 CF，而地址调整仍然有效。普通栈调整还须满足 [VM 调用约定](VM-Calling-Convention.md) 的活动帧边界，不得回收至当前 `bp + 16` 以下。失败通过显式 trap 报告，`sp` 和 EFLAGS 都保持原值。

这一规则适用于所有写入 `sp` 的指令。普通整数 `add` 仍按指定位宽回绕；更新栈顶不能先回绕，再只检查最终数值是否位于栈范围内。

函数返回时回收整个函数帧中的局部空间。函数内提前回收空间时，执行该调整的函数须保证已回收空间不再被使用。

建议按增量精确移动栈顶，不暗中取整。槽位间的对齐填充由布局方计入调整量，调用边界的对齐也由调用方保证。

## 函数调用与返回

当前已提供 `call`、`callNative` 指令族和零操作数 `ret` 的指令模型。直接调用复用 `ImmediateOperand`：`Calli` 保存 PC 相对 i64 位模式或 VM 函数符号，`CallNativei` 保存统一动态导入表 `DynamicImportSymbols` 的 u32 索引或未被可见 VM 定义满足的函数声明名称；调用符号的 addend 必须为零。间接调用复用 `AddressOperand`：`Callm` 读取 64 位 VM 函数地址，`CallNativem` 读取 64 位 native 可调用指针；间接地址中的符号定位存放指针的内存，不要求它是函数符号。这些指令不包含参数、结果槽或签名描述。文本读写、链接、指令编码和镜像发射已实现，解码及执行器仍待实现。

间接 native 目标不是 u32 导入索引，其可调用指针的 adapter 与宿主 ABI 协议仍待确定。逻辑模块通过 `MachineSymbolDeclaration` 统一保存声明，未被 VM 函数或全局定义满足的声明形成 `NativeSymbols`；符号只有一个名称，同时用于逻辑引用和宿主查找。统一动态导入表只保存实际使用的 Native 符号名称，完整调用签名与执行描述仍待实现。函数和数据的取地址用途共用 `DynamicImportSymbolRelocations`，地址重定位本身不赋予地址可调用性。增加间接指令模型不表示已经实现任意 C 函数指针调用。

参数与返回槽的 8 字节对齐、仅可变参数调用包含的 24 字节预留区、CALL/RET 状态转换及完整示例统一见 [VM 调用约定](VM-Calling-Convention.md)。`ret` 从 `[bp]`、`[bp + 8]` 恢复旧 BP 和返回 PC，将 SP 恢复为返回前的 BP；调用区由调用方回收。RET 保持 EFLAGS 不变。

## 执行边界与失败：建议约定

`pc` 必须指向合法指令起点。解码需要验证操作码、操作数长度以及目标指令边界；普通指令成功后执行 `pc = NextPC`，调用保存的也是 `NextPC`。程序入口可以使用专用的初始调用帧，最外层返回结束执行，不将任意地址当作退出哨兵。

栈容量不足、地址运算溢出、非法访存、非法指令和无效跳转目标建议通过 VM 的显式执行结果或 trap 状态报告。trap 是 VM 的停止状态，不使用宿主 C++ 异常；失败前完成必要检查，避免只更新一部分寄存器或调用帧。退出值和错误信息的对外接口仍待运行时 API 设计确定。

可执行镜像结构验证与执行验证分开处理。现有 `ExecutableFileValidator` 检查入口在镜像内以及统一导入、地址重定位和导出符号的结构，不解码指令。镜像固定使用小端序，不保存字节序配置；也不保存段表，导出符号只记录名称及镜像内偏移。代码范围识别、指令边界建立及数据权限策略仍待确定。指令编码、镜像布局与发射已实现，解码和独立模块 validator 仍待实现。现有镜像结构校验不提供运行时调用约定或访存验证；后续加载器及执行器仍须完成指令边界、地址、栈和权限检查。

## 后续讨论

按 [VM 调用约定](VM-Calling-Convention.md) 生成调用区布局、执行 CALL、RET 与控制流的功能尚未实现。`opcode.def` 中已有形式的操作数编码以 [VM 指令编码格式](VM-Instruction-Format.md) 为准；镜像生成、程序入口退出及宿主 ABI adapter 仍需落实。
