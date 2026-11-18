# VM 指令分派性能优化方案

本文记录 `src/lib/backend/vm/interpreter/dispatch.cpp` 中 `Interpreter::dispatch()` 的性能瓶颈分析与后续分阶段优化方案。当前分派实现采用单一 `switch` 循环并对 `PC` 做了局部缓存（`LocalPC`），相关运行时背景见 [VM 运行时设计讨论稿](VM-Runtime.md)、[VM 指令编码格式](VM-Instruction-Format.md) 和 [VM 调用约定](VM-Calling-Convention.md)。以下方案目前仅作设计归档，暂未实现。

## 1. 消除寄存器访问的二级指针间接寻址与别名障碍

### 现状瓶颈

1. **双重内存加载（Pointer Chasing）**：`Frame` 中的 `READ_REGISTER` 与 `WRITE_REGISTER` 通过 `Frame.Register[Id]`（14 项 `const void*` 指针表，占 112 字节、跨越两条缓存行）间接访问底层寄存器成员。每次寄存器读写都需要先从指针表加载地址，再解引用读写目标 `std::uint64_t` 值。
2. **`std::memcpy` 引发的编译器别名分析（Alias Analysis）失效**：`Frame::initializeRegisters()` 将 `BP`、`SP`、`PC` 及 `GeneralPurpose` 的地址存入了 `Frame.Register`，导致这些成员的地址在编译器视角下已逃逸。当 `Add64mrr`、`Load64`、`Store64i` 等指令通过 `std::memcpy(reinterpret_cast<void*>(DestinationAddress), ...)` 向任意运行时地址写入时，C++ 编译器无法证明该写入不会覆盖 `Frame` 成员、`Frame.Register` 指针表以及 `this->CodeSegmentSize`、`this->StackSize`，从而被迫在跨指令边界时反复从内存重载寄存器与解释器成员，无法将热点 VM 寄存器长期驻留在宿主 CPU 寄存器中。

### 优化方案

1. 在 `Interpreter::dispatch()` 入口处，在宿主栈上创建非逃逸的局部数组 `std::uint64_t LocalRegisters[14]`（顺序与 `RegisterId` 严格一致：`BP`、`SP`、`PC`、`ZERO`、`R0`–`R9`），并将 `CodeSegmentSize`、`StackSize` 缓存为局部常量。
2. 分派循环内直接通过 `LocalRegisters[static_cast<std::size_t>(Register)]` 索引读写寄存器：
   - 将单次寄存器访问从 2 次串联内存加载降为 1 次局部数组访问。
   - 由于 `LocalRegisters` 的地址不向外部函数或裸指针逃逸，编译器别名分析与标量替换（SROA）能够证明任意 VM 内存写入不会污染 `LocalRegisters`。
3. 仅在以下边界同步 `LocalRegisters` 与 `Frame`：
   - 进入 `dispatch()` 时从 `Frame` 載入初始寄存器值。
   - `EXECUTE_CALL` 转入 Native 桩 `Stub(&Frame)` 前将 `LocalRegisters` 与 `LocalPC` 写回 `Frame`，并在 `Stub(&Frame)` 返回后从 `Frame` 重新载入（允许 Native 桩按调用约定修改寄存器）。
   - `dispatch()` 正常返回或因错误退出前写回 `Frame`，确保外部 `getFrame()` 和故障诊断观察到一致的执行状态。

## 2. 消除每条指令无条件写回 `Frame.setPC(LocalPC)` 与全量寄存器解码

### 现状瓶颈

1. **每轮循环无条件同步 `PC`**：当前循环顶部在每条指令执行前都调用 `Frame.setPC(LocalPC)` 写回内存。对于不以 `RegisterId::PC` 作为源或基址操作数的指令（如 `Fadd64rrr`–`Fsqrt64rr`、`Jmpi`、`Calli`、`Ret`，以及基址为 `BP`、`SP`、`R0`–`R9` 的常规算术和访存指令），该写回属于冗余内存存储。
2. **分派前全量解码三个寄存器槽**：循环顶部在进入 `switch (Operation)` 前无条件解码 `DestinationRegister`、`LeftRegister` 和 `RightRegister`。而 `Ret`、`Jmpi`、`Calli` 不使用任何寄存器字段，`Jmp`、`Jmpr`、`Jmpm`、`Callm`、`Store32i`、`Store64i` 只使用 1 个寄存器字段，`Load64`、`Fsqrt64rr`、`Add64rri`、`Add64mmi` 及 40 条 `Mib` 条件分支只使用 2 个寄存器字段。在大型 `switch` 分派点前提前计算全部三个字段会增加分派点处的宿主寄存器分配压力。

### 优化方案

1. 将 `PC` 的按需可见性收敛到真正读取 `PC` 的路径：在局部寄存器模型下，仅在操作数可能读取 `PC`、进入 Native 桩或退出分派时更新 `PC` 槽位。
2. 将 `InstructionEncoding::decodeRegister(Header, Index)` 下沉到各指令分支或按指令形式（`Rrr`、`Rri`、`Mm`、`Mmb`、`Mib` 等）划分的内联辅助函数中，按需提取寄存器编号。

## 3. 指令分派机制升级：Computed Goto 或尾调用线索化（Threaded Dispatch）

### 现状瓶颈

1. **单一间接跳转点的分支预测冲突**：`for (;;) { switch (Operation) { ... } }` 会被编译为所有操作码共享的单一宿主间接跳转指令。现代 CPU 的间接分支预测器（BTB / ITTAGE）以跳转指令 PC 结合分支历史作为索引，所有 VM 指令共用同一间接跳转点会导致不同字节码序列互相覆盖预测项。
2. **稀疏操作码区间的多级跳转检查**：`Opcode` 底层为 `std::uint16_t`，有效编码位于 `0x0001`–`0x0096`，其中存在未分配的编号（如 `0x001F`–`0x002A`、`0x002C`–`0x0039` 等），编译器容易生成多段范围比较加子跳转表，而非单次索引跳转。

### 优化方案

1. **Direct-Threaded / Token-Threaded Computed Goto（Clang / GCC）**：
   利用所有合法操作码均小于 256 的特性，构建固定 256 项的标签地址表 `static constexpr const void* DispatchTable[256]`（未分配项指向错误处理标签 `&&HandleUnsupported`）。在每个指令 handler 末尾直接预取下一条指令头并按标签跳转：
   ```cpp
   #define DISPATCH_NEXT()                                                                    \
       do                                                                                     \
       {                                                                                      \
           InstructionAddress = LocalPC;                                                      \
           Instruction = reinterpret_cast<const std::uint64_t*>(InstructionAddress);          \
           Header = Instruction[0];                                                           \
           const auto NextOpcode = static_cast<std::uint16_t>(Header & 0xFFFFU);              \
           goto *DispatchTable[NextOpcode < 256 ? NextOpcode : 0];                            \
       } while (false)
   ```
   每个操作码拥有独立的宿主间接跳转指令，可大幅降低 BTB 冲突，并允许 CPU 将下一条指令的取指解码与当前指令尾部的写回重叠执行。
2. **Tail-Call Threading / 紧凑单表分派（跨平台兼容 MSVC）**：
   在不启用 GNU 标签地址扩展的编译器上，可使用 256 项 handler 函数指针表配合 `[[clang::musttail]]` / `[[msvc::musttail]]` 尾调用分派，把 `LocalPC`、`CallDepth`、`Frame` 保持在调用约定寄存器中；即便保留 `switch`，也可先对 `Operation <= Opcode::Fsqrt64rr` 做单次无符号上界检查，引导编译器生成单一平坦跳转表。

## 4. 精简 `Call` 与 `Ret` 的边界检查算术并分离冷路径

### 现状瓶颈

1. **多分支范围与对齐检查**：`IsInstructionAddress` 包含 4 次比较与取模运算，并通过捕获 `this` 读取 `CodeSegmentSize`：
   ```cpp
   return Address >= CodeAddress && Address - CodeAddress < CodeSegmentSize && (Address - CodeAddress) % InstructionEncoding::WordBytes == 0 && CodeSegmentSize - static_cast<std::size_t>(Address - CodeAddress) >= InstructionEncoding::WordBytes;
   ```
2. **热循环内联错误格式化**：`EXECUTE_CALL` 内部直接包含 `fmt::format` 与 `Diagnostics.report` 调用点，未标记 `[[unlikely]]`，导致异常处理与字符串格式化代码留在热路径附近，影响指令缓存局部性与寄存器分配。

### 优化方案

1. **利用无符号减法回绕折叠边界检查**：
   `execute()` 入口已保证代码段至少包含一个完整指令字。在进入循环前预计算 `const std::uint64_t MaxInstructionOffset = LocalCodeSegmentSize - InstructionEncoding::WordBytes;`。当 `Address < CodeAddress` 时，无符号差值 `Address - CodeAddress` 会下溢回绕为大于 `MaxInstructionOffset` 的大整数，因此 4 项检查可合并为 1 次无符号减法、1 次比较与 1 次按位与：
   ```cpp
   const auto IsInstructionAddress = [CodeAddress, MaxInstructionOffset](std::uint64_t Address) noexcept
   {
       const std::uint64_t Offset = Address - CodeAddress;
       return (Offset <= MaxInstructionOffset) & ((Offset & (InstructionEncoding::WordBytes - 1)) == 0);
   };
   ```
   同理，`IsStackPosition` 可简化为：
   ```cpp
   const auto IsStackPosition = [StackAddress, LocalStackSize](std::uint64_t Address) noexcept
   {
       const std::uint64_t Offset = Address - StackAddress;
       return (Offset <= LocalStackSize) & ((Offset & (InstructionEncoding::WordBytes - 1)) == 0);
   };
   ```
2. **冷路径标记与外提**：
   栈溢出分支已标注 `[[unlikely]]`。建议为 `default` 分支加上相同属性，并将诊断构造逻辑提取到独立的不内联冷函数中。

## 5. 降低 80 条比较条件跳转（`compare_branch.def`）的 I-Cache 压力

### 现状瓶颈

`compare_branch.def` 在 `dispatch()` 内直接宏展开为 80 个独立的 `case` 块。每个块都重复生成寄存器寻址、内存拷贝、条件判断及 `LocalPC` 更新代码（在开启断言的构建中还包含多重 `check` 展开），显著增加了 `dispatch()` 的机器码体积，给 L1 指令缓存（通常为 32 KiB）带来压力。

### 优化方案

1. **抽取参数化内联模板与公共尾块**：将比较分支的左值加载、右值（内存或立即数）加载与目标 PC 选择封装为按 `<typename ValueType, bool MemoryRHS, typename CompareOp>` 实例化的小型辅助模板，确保编译器对相同位宽与寻址形式的块共享寻址与跳转收尾代码。
2. **利用正交编码规律按需分组**：`0x0042`–`0x0091` 的 80 个操作码按“比较关系（10 种）× 位宽（8/16/32/64）× 右操作数形式（`Mmb`/`Mib`）”严格正交排列。若后续基准测试显示 I-Cache 缺失占主导，可按位宽与形式合并加载逻辑，仅对比较算子做轻量分支或无分支条件选择。

## 6. 中远期架构演进：加载期预解码（Pre-decoding）与指令流紧缩

### 现状瓶颈

1. **指令流内存带宽开销**：当前 wire 格式固定每个操作数占 8 字节槽，三操作数指令占 32 字节（半条 64B 缓存行）。例如 `Fadd64rrr` 的三个操作数槽恒为 0（共 24 字节冗余），`Add64rrr` 在零偏移寄存器相加时仍需从指令流读取 `Instruction[2]` 与 `Instruction[3]` 并执行加法。
2. **`Calli` 运行时重复判断目标类别**：每次执行 `Calli` 都需要通过 `if (IsInstructionAddress(CallTarget))` 在运行时判断目标是 Ink VM 函数还是 Native 桩。

### 优化方案

1. **加载期或首次执行前的预解码表示**：
   - 在预解码阶段验证保留位、寄存器编号合法性与指令边界，使执行期无需在每条指令上重复结构校验。
   - 对直接调用 `Calli`，在加载重定位完成后目标地址类别已经固定，可在预解码流中直接分化为内部操作码 `CalliVM` 与 `CalliNative`，省去运行时的目标范围分支。
2. **高频指令特化（Superinstructions / 零偏移快路径）**：
   - 在预解码表示中将零位移的 `Add64rrr`、`Add64rri`（如栈指针调整和纯寄存器算术）特化为不读取冗余零位移槽的紧凑处理路径，提升指令缓存密度与吞吐率。
