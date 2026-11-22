# VM 汇编文本格式

`src/include/ink/backend/vm/serialization/machine_module_text.h` 提供无异常读写接口：

```cpp
std::expected<void, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue, std::string& Output);
std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue);
std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> readMachineModuleText(std::string_view Text);
```

- `readMachineModuleText()` 从 Ink VM 汇编文本构建独立拥有的 `MachineModule`，不借用输入字符串，也不执行模块链接或镜像发射。
- `writeMachineModuleText()` 将逻辑 `MachineModule` 序列化为规范（canonical）Ink VM 汇编文本。
- 读写失败均返回 `core::Diagnostic` 诊断：
  - **读取失败**返回 `core::DiagnosticKind::VMTextInvalid`（`INK-V0001`），并携带输入文本中出错词法单元或结构对应的字节范围 `SourceRange`（半开区间 `[Begin, End)`，调用方可补充 `SourceId` 后交给 `DiagnosticEngine`）。
  - **写出失败**（输入 `MachineModule` 内存模型违反结构或指令契约，无源文本可指）通过 `verifyMachineModule()` 返回 `core::DiagnosticKind::VMModuleInvalid`（`INK-V0006`）和无效 `SourceRange{}`（`Span.isValid() == false`）。
- LLVM InkVM 后端（`InkVMTarget` / `InkVMAsmPrinter`）与 `ink-interpreter` 共用这一套正式读写实现，保证直接内存模型发射与汇编文本输出完全一致。

## 文本示例

```text
.module "demo"
.declare function @host_log

.section .data
.global @greeting size 16 align 8 readonly external
    .bytes "hello\00"
    .zero 2
    .i64 42
.endglobal
.global @dispatch_table size 16 align 8 readonly
    .reloc 0, @compute
    .reloc 8, @greeting + 8
.endglobal

.section .text
.function @compute external
    .stackslot size 16 align 8
    .frame base 16
entry:
    add64rri r0, zero, 42
    call @host_log
    ret
.endfunction
```

## 正式文法

```ebnf
Module              := ModuleHeader SymbolDeclaration* Section*
ModuleHeader        := '.module' QuotedString
SymbolDeclaration   := '.declare' ('function' | 'global') Symbol ['readonly']
Section             := '.section' ('.data' GlobalDefinition* | '.text' FunctionDefinition*)

GlobalDefinition    := '.global' [Symbol] GlobalAttr+
                       GlobalBodyDirective*
                       '.endglobal'
GlobalAttr          := 'size' Unsigned
                     | 'align' Unsigned
                     | 'readonly'
                     | 'local'
                     | 'external'

GlobalBodyDirective := ByteDirective
                     | BytesDirective
                     | ZeroDirective
                     | IntDataDirective
                     | FloatDataDirective
                     | RelocDirective
ByteDirective       := '.byte' Unsigned (',' Unsigned)*
BytesDirective      := '.bytes' QuotedString
ZeroDirective       := '.zero' Unsigned
IntDataDirective    := ('.i16' | '.i32' | '.i64') SignedOrUnsignedInt (',' SignedOrUnsignedInt)*
FloatDataDirective  := '.f64' FloatLiteral (',' FloatLiteral)*
RelocDirective      := '.reloc' Unsigned ',' Symbol [Displacement]

FunctionDefinition  := '.function' [Symbol] [Linkage]
                       StackSlotDirective*
                       [FrameDirective]
                       BasicBlock*
                       '.endfunction'
StackSlotDirective  := '.stackslot' StackSlotAttr+
StackSlotAttr       := 'size' Unsigned | 'align' Unsigned
FrameDirective      := '.frame' 'base' Unsigned

BasicBlock          := Identifier ':' [QuotedString]
                       Instruction*
Instruction         := Mnemonic [Operand (',' Operand)*]

Linkage             := 'local' | 'external'
Symbol              := '@' (Identifier | QuotedString)
Operand             := '[' (Register | Symbol) [Displacement] ']'
                     | Symbol [Displacement]
                     | Register [Displacement]
                     | SignedOrUnsignedInt
                     | Identifier
Displacement        := ('+' | '-') Unsigned
```

## 词法规则

- **行尾与空白**：水平空白仅接受空格（`0x20`）与水平制表符（`0x09`）。行结束同时支持 LF（`\n`）、CRLF（`\r\n`）和独立 CR（`\r`）。空行与仅含注释的行被忽略。每条伪指令、基本块标签行和指令独占一行。
- **注释**：`;` 引入行注释，直至行尾（不含换行符）或文件结束；字符串字面量内部的 `;` 属于普通字符。
- **标识符（`Identifier`）**：首字符属于 `[A-Za-z_.$]`，后续字符属于 `[A-Za-z0-9_.$]`。伪指令（以 `.` 开头）、助记符、关键字、寄存器名和基本块标签在词法层均为标识符，区分大小写。
- **符号（`Symbol`）**：必须以 `@` 开头，后接合法标识符（如 `@counter`、`@.L0`）或非空双引号字符串（如 `@"entry point"`）。空符号 `@""` 非法。规范输出对满足标识符规则的符号名直接输出 `@name`，其余输出 `@"..."`。
- **双引号字符串（`QuotedString`）**：由 `"` 包裹，不得跨行。字符串内只允许可打印 ASCII（`0x20`–`0x7E`）与转义序列：
  - `\"` 表示双引号，`\\` 表示反斜杠；
  - `\HH`（两位十六进制数字，大小写均可）表示任意单字节（包括 `\00` 及 UTF-8 原始字节）。
  - 规范输出对 `"` 和 `\` 输出 `\"`、`\\`，对可打印 ASCII 原样输出，对其余字节输出大写 `\HH`。
- **整数字面量**：无符号整数接受十进制与 `0x`/`0X` 前缀十六进制；带符号整数允许可选前导 `+` 或 `-`。
- **浮点字面量（仅 `.f64`）**：接受可选前导 `+`/`-` 后接十进制浮点（如 `0.01`、`-0.0`、`1.5e-3`）、带二进制指数 `p`/`P` 的十六进制浮点（如 `0x1.8p+1`），以及关键字 `inf` 和 `nan`（`-nan` 置符号位为 `1`）。**不带 `p`/`P` 的裸 `0x...` 在 `.f64` 中会被显式拒绝**，原始 64 位 IEEE 754 位模式应通过 `.i64` 编写，避免将十六进制位模式误解析为整数浮点值。

## 模块头、符号声明与节

- 模块第一条有效语句必须是 `.module "<name>"`，模块名可以为空字符串 `""`。
- `.declare` 声明必须在所有 `.section` 之前出现，形式为：
  - `.declare function @symbol`
  - `.declare global @symbol [readonly]`
  - 函数声明不允许携带 `readonly`。
- `.section` 支持 `.data` 和 `.text` 两种节（不再存在 `.rodata` 或 `.constant`）。同类节可以重复或交错出现，读取时按声明顺序追加到对应的全局或函数列表。
- **空节规范省略**：规范写出在存在全局变量时才输出 `.section .data`，在存在函数时才输出 `.section .text`；空模块仅输出 `.module "<name>"`。

## 全局变量定义（`.section .data`）

全局变量仅允许出现在 `.section .data` 中，由 `.global` 开头、`.endglobal` 结束：

```text
.global [@symbol] size <N> [align <A>] [readonly] [local|external]
    ...
.endglobal
```

- **符号名**：可选；省略表示匿名局部全局变量。`external` 定义必须提供非空 `@symbol`。
- **属性**：`size <N>` 必填；`align <A>` 可选（必须为非零二次幂，默认 `1`）；`readonly` 标记常量属性；`local`/`external` 指定链接属性（默认 `local`）。文本解析允许这些属性以**任意非重复顺序**出现。
- **规范属性顺序**：`size <N>` → `align <A>`（当 `A != 1`）→ `readonly`（若为常量）→ `external`（若为外部链接；省略 `local` 与 `align 1`）。
- **数据体伪指令**按出现顺序追加初始化字节（小端序），累计长度不得超过 `size`；未覆盖的尾部在运行时默认为零，且不会在内存中展开：
  - `.byte v0, v1, ...`：每个元素须在 `[0, 255]` 内。
  - `.bytes "..."`：将解码后的原始字节序列直接追加（不自动添加 NUL 终止符）。
  - `.zero <count>`：追加 `<count>` 个零字节（`count = 0` 允许且无副作用）。
  - `.i16` / `.i32` / `.i64`：每个元素接受对应位宽的无符号范围或二进制补码有符号范围（如 `.i16` 接受 `[-32768, 65535]`），按小端序写入 2/4/8 字节。
  - `.f64`：按 IEEE 754 binary64 小端序写入 8 字节。
  - `.reloc <offset>, @symbol [+|- <disp>]`：在 `ByteOffset` 处登记一个 8 字节绝对符号地址重定位（`MachineGlobalRelocation`），要求全局对齐 `>= 8`、`offset % 8 == 0`、`[offset, offset + 8)` 在 `size` 范围内且互不重叠、对应 8 字节初始化数据全为零，`<disp>` 为有符号 64 位整数（规范输出省略 `+ 0`）。
- **规范数据体输出**：连续长度 `>= 8` 的零字节段压缩为 `.zero <count>`；其余初始化字节按每行最多 16 字节输出为小写十六进制 `.byte 0x..`；重定位按升序偏移输出在初始化字节之后。

## 函数定义与基本块（`.section .text`）

函数仅允许出现在 `.section .text` 中，由 `.function` 开头、`.endfunction` 结束：

```text
.function [@symbol] [local|external]
    .stackslot size <N> [align <A>]
    .frame base <Offset>
entry:
    ...
.endfunction
```

- **函数头**：`@symbol` 可选（`external` 必须提供非空 `@symbol`）；链接属性默认 `local`，规范输出省略 `local`。
- **栈帧元数据**（均须出现在首个基本块之前）：
  - `.stackslot` 接受任意非重复顺序的 `size <N>`（必填）与 `align <A>`（可选，非零二次幂，默认 `1`）；规范输出为 `size <N>` 后接可选 `align <A>`（当 `A != 1`）。
  - `.frame base <Offset>` 最多出现一次，必须位于全部 `.stackslot` 之后；读取时触发 `MachineFrameInfo::layout(Offset)` 恢复已布局状态。未出现 `.frame base` 时保留未布局状态。
- **基本块标签**：
  - 形式为 `Label: ["OriginalName"]`。
  - 文本标签在所属函数内唯一，用于解析同一函数内的前向、后向和自引用分支目标；跨函数引用或未定义块标签一律报错。
  - 当基本块为匿名、与前块重名或原名不是合法标识符时，规范写出为其分配不冲突的文本别名（如 `.L0`、`entry.1`），并在冒号后附加引号包裹的原始名称（如 `.L0: ""`、`entry.1: "entry"`），从而在往返中无损保留块名与跳转目标对象身份。

## 指令、操作数与语义约束

每条指令由助记符和 0 至 3 个逗号分隔的操作数构成，合法操作码与形式由 `src/include/ink/backend/vm/instruction/opcode.def` 统一定义：

| 操作数类别 | 文本形式 | 说明 |
| --- | --- | --- |
| 寄存器 `r` | `reg` 或 `reg + disp` / `reg - disp` | 合法寄存器：`bp`、`sp`、`pc`、`zero`、`r0`–`r9`。目标寄存器不可为 `pc`/`zero` 且位移必须为 `0`；全部 25 条浮点域指令要求目标属于 `r0`–`r9`、来源属于 `r0`–`r9`/`zero` 且所有显式寄存器操作数位移为 `0`。 |
| 内存地址 `m` | `[reg +/- disp]` 或 `[@symbol +/- disp]` | 基址寄存器接受 `bp`、`sp`、`pc`、`zero`、`r0`–`r9`；`disp` 为有符号 `i64`，默认 `0`。 |
| 数值立即数 `i` | `SignedOrUnsignedInt` | 保存完整 64 位位模式；规范输出对有符号范围 `[-1048576, 1048576]` 使用十进制，其余位模式（如掩码、IEEE 754 位模式）使用小写 `0x...`。 |
| 符号立即数 `i` | `@symbol` 或 `@symbol +/- disp` | 表示 `AddressOf(symbol) + disp`（不解引用内存）。**仅 64 位指令允许符号立即数**；8/16/32 位窄宽度指令使用符号立即数在读、写、链接和发射阶段均被拒绝。`call`/`callvm`/`jmpi` 的直接符号目标要求位移为 `0`。 |
| 基本块目标 `b` | `Label` | 仅出现在 `jmp` 与比较条件分支的最后一个操作数位置，必须解析到当前函数内定义的基本块。 |

### 重载助记符与 `callvm` 说明

- `call` 按操作数种类静态重载：`call @fn` / `call <u64>` 对应 `Calli`，`call [addr]` 对应 `Callm`，`call reg` 对应 `Callr`。编译器在编译期校验 `opcode.def` 中所有重载助记符的操作数签名互不冲突且块目标位置一致。
- `callvm` 对应 `Opcode::Callvmi`（编码 `0x003c`），是**解释器加载期将直接调用特化为 VM 内部快路径的内部操作码**，一般手写汇编和后端代码生成均应使用 `call`。文本格式保留 `callvm` 的完整读写支持，以确保对任意 `MachineModule` 的全操作码无损往返序列化与调试转储。

## 共享校验与往返契约

- 读取器采用独立的单遍流式 `AsmLexer`（`machine_module_text_lexer.cpp`）与单遍递归下降 `ModuleTextParser`（`machine_module_text_reader.cpp`），基于 `vm/instruction` 中的 `OpcodeInfo` 二分查找表和 `MachineInstructionBuilder::tryCreate(Opcode, std::span<const MachineOperand>)` 直接构建指令，并通过基本块前向占位与 `MachineFunction::moveBasicBlockToEnd()` 单遍解析前向跳转。
- 写出器（`machine_module_text_writer.cpp`）在写出前先调用 `verifyMachineModule()`（`src/include/ink/backend/vm/module/machine_module_verifier.h`，通过 `core::DiagnosticEngine` 报告结构化 `VMModuleInvalid` 诊断）做一次性结构与指令校验，校验通过后由无错纯格式化器 `ModuleTextPrinter` 追加输出至目标缓冲区；写出失败时不修改调用方传入的 `Output`。
- 读写器与链接器、发射器共享 `src/include/ink/backend/vm/instruction/machine_instruction_validator.h` 中的单一真源指令约束校验（寄存器合法性、目标写权限与零位移、浮点域契约、直接调用/跳转符号零位移、窄位宽禁止符号立即数）。
- 往返保证：对任意合法 `MachineModule M`，`readMachineModuleText(writeMachineModuleText(M))` 保留完整模块语义与结构，且二次规范输出满足 `write(read(write(M))) == write(M)`。注释、空行、属性先后顺序、手写字面量进制及 `.bytes`/`.i*`/`.f64` 写法在规范输出时会归一化为标准形式。
- 文本读写只负责语法解析与单模块内局部约束，不检查跨模块符号冲突、未声明引用或外部符号绑定；这些属于 [VM 模块链接器](VM-Linker.md) 与 [VM 可执行镜像模型](VM-Executable.md) 的职责。
