# LLVM API 下的 PDB 降低方案

> 状态：实现方案已确认
> 性质：非规范性实现说明
> 对应语言议题：[`../topics/08-platform-dependent-behavior.md`](../topics/08-platform-dependent-behavior.md)

## 1. 结论

Ink 使用 LLVM 提供的 inline-asm API 实现 PDB：

- 不修改 LLVM 源码；
- 不维护 LLVM fork；
- 不注册新的 LLVM intrinsic；
- 不把 PDB 运算降低为带 UB 或 poison 的普通 LLVM 运算；
- 不插入范围检查、条件分支或结果修正；
- 直接生成目标机器的原生操作。

C++ API 使用 `llvm::InlineAsm::get`，C API 使用 `LLVMGetInlineAsm`。

## 2. 为什么不使用自定义 LLVM intrinsic

LLVM 没有公开的运行时 API 用于注册一种新的 LLVM intrinsic。真正增加 `llvm.ink.pdb.*` intrinsic 需要修改 LLVM 的 `Intrinsics*.td`、重新运行 TableGen、重新编译 LLVM，并增加各目标的 lowering。

LLVM pass plugin 可以注册和运行 pass，但不能通过插件为 LLVM IR 动态增加新的 intrinsic ID、instruction opcode 或 SelectionDAG opcode。

因此 Ink 不采用自定义 LLVM intrinsic 作为基础实现。

## 3. 不能使用普通 LLVM IR 运算

LLVM 的普通整数除法和移位不能完整表示 Ink PDB：

- LLVM `udiv` 和 `sdiv` 对除零规定 UB；
- LLVM `sdiv` 对 `MIN / -1` 规定 UB；
- LLVM `shl`、`lshr` 和 `ashr` 在移位量大于等于类型位宽时产生 poison；
- `freeze` 只能把 poison 变成任意但固定的值，不能保留目标机器的结果或 trap。

下面的降低是错误的：

```llvm
; 错误：优化器仍可利用 LLVM 自身的 UB/poison 规则。
%q = sdiv i64 %a, %b
%r = freeze i64 %q
```

## 4. Ink IR

在进入 LLVM IR 之前，Ink 自身的 IR 保留以下专用操作：

```text
pdb.sdiv
pdb.udiv
pdb.srem
pdb.urem
pdb.shl
pdb.lshr
pdb.ashr
```

这些节点不可推测、不可删除，并且可能产生目标平台 trap。

当编译器已经证明输入不在 PDB 输入域时，使用普通 LLVM IR：

```text
a / 3             -> LLVM sdiv
a / b（b 未知）   -> PDB inline asm
x << 4            -> LLVM shl，前提是 4 小于类型位宽
x << count        -> PDB inline asm，除非已证明 count 小于类型位宽
```

## 5. 使用 LLVM C++ API

下面以 AArch64 有符号 64 位除法为例：

```cpp
llvm::Value* emit_pdb_sdiv_aarch64(
    llvm::IRBuilder<>& builder,
    llvm::Value* lhs,
    llvm::Value* rhs) {
    llvm::Type* type = lhs->getType();
    llvm::FunctionType* function_type = llvm::FunctionType::get(
        type,
        {type, type},
        false
    );

    llvm::InlineAsm* operation = llvm::InlineAsm::get(
        function_type,
        "sdiv $0, $1, $2",
        "=r,r,r",
        true,                       // hasSideEffects
        false,                      // isAlignStack
        llvm::InlineAsm::AD_ATT,
        false                       // canThrow: 不是 LLVM unwind
    );

    return builder.CreateCall(function_type, operation, {lhs, rhs});
}
```

生成的 LLVM IR 形态是：

```llvm
%result = call i64 asm sideeffect
    "sdiv $0, $1, $2", "=r,r,r"(i64 %lhs, i64 %rhs)
```

`hasSideEffects = true` 非常重要。它使这段操作不能因为返回值未使用而被删除，也不能像普通纯表达式一样被推测执行。

汇编字符串和约束由 Ink 按目标 ISA、整数宽度和操作种类生成，不能把上述 AArch64 示例直接用于其他目标。

## 6. 使用 LLVM C API

如果 Ink 后端使用 LLVM C API，则调用：

```c
LLVMValueRef LLVMGetInlineAsm(
    LLVMTypeRef Ty,
    const char *AsmString,
    size_t AsmStringSize,
    const char *Constraints,
    size_t ConstraintsSize,
    LLVMBool HasSideEffects,
    LLVMBool IsAlignStack,
    LLVMInlineAsmDialect Dialect,
    LLVMBool CanThrow
);
```

随后使用普通的 call-builder API，以 inline-asm value 为 callee 创建调用。

## 7. 目标指令映射

### x86-64

```text
pdb.sdiv / pdb.srem -> CQO + IDIV
pdb.udiv / pdb.urem -> 清理高半部 + DIV
pdb.shl             -> SHL
pdb.lshr            -> SHR
pdb.ashr            -> SAR
```

x86 除法具有固定寄存器约束。Inline asm 必须正确描述 `RAX`、`RDX`、除数输入和 condition-code clobber，不能只使用通用的 `=r,r,r` 约束。

### AArch64

```text
pdb.sdiv -> SDIV
pdb.udiv -> UDIV
pdb.srem -> SDIV + MSUB
pdb.urem -> UDIV + MSUB
pdb.shl  -> LSLV
pdb.lshr -> LSRV
pdb.ashr -> ASRV
```

对于超位宽的常量移位，如果立即数指令不能表达原始移位量，必须先把该常量放入寄存器，再使用寄存器移位指令。不能在编译期把它折叠为其他结果。

## 8. Side effect、clobber 与顺序

每个 inline-asm 模板必须完整列出：

- 固定输入和输出寄存器；
- 被覆盖的临时寄存器；
- condition-code clobber；
- 操作实际读写的其他目标状态。

PDB 整数运算不读写普通程序内存，因此默认不添加 `~{memory}`。`sideeffect` 用于表达可能出现的目标 trap 等不可删除效果。

如果后续确定 Ink 要求 trap 与普通内存操作之间具有严格的源码顺序，而 LLVM 对某个模板的移动可能违反该顺序，则该模板必须增加相应的 memory clobber 或采用更保守的调用式屏障。该选择不会增加机器检查指令，但可能限制周围代码优化。

## 9. 窄整数和非原生宽度

PDB 必须基于最终采用的机器操作定义。

例如 `u8 << count`：

- x86 可以使用窄整数移位指令；
- AArch64 通常需要提升到 32 位，完成移位后再截断；
- 两种降低在超位宽移位时可能得到不同结果。

这些差异必须记录在各目标平台的 PDB 表中。

`i128/u128` 在没有原生 128 位除法的目标上仍需要展开或运行库帮助函数。此时不存在可以直接表达完整操作的单条 inline asm，目标运行库必须规定并实现其 PDB。非原生类型原本需要的运行库成本不算用于统一 PDB 的额外检查成本。

## 10. 性能性质

inline-asm API 本身不会生成函数调用。最终机器代码只包含模板指定的机器指令以及满足寄存器约束所需的正常寄存器搬运：

- 没有自动除零检查；
- 没有 `MIN / -1` 检查；
- 没有移位量范围检查；
- 没有跨平台结果修正。

代价是 LLVM 无法理解 inline asm 内部的数学关系，因此不能对仍可能触发 PDB 的操作做常量折叠、公共子表达式消除、向量化或强度削减。这与 Ink 对 PDB 的优化限制一致。

已经证明不进入 PDB 输入域的操作仍使用普通 LLVM IR，不承担这项优化损失。

## 11. 必测用例

每个目标至少测试：

```text
x / 0
x % 0
MIN / -1
MIN % -1
x << bit_width
x >> bit_width
x << very_large_count
x >> very_large_count
```

还必须覆盖：

- 常量与变量操作数；
- `i8/i16/i32/i64/i128` 和对应无符号类型；
- `-O0`、`-O2`、`-O3` 和 LTO；
- 返回值使用与未使用；
- PDB 位于条件分支、循环和不可内联函数中；
- 交叉编译时宿主与目标平台行为不同的情况；
- `llvm::verifyModule` 和目标汇编器对所有约束的验证；
- 最终机器码中不存在额外的边界检查分支。

## 12. 可选的未来改进

如果未来 inline asm 的维护成本或优化损失不可接受，可以再考虑向 LLVM 上游提交通用的 target-behavior 整数 intrinsic，或者维护很小的 LLVM 扩展。

这不是 Ink 初始实现的必要条件，也不改变当前已经确定的 API-only 方案。
