# 议题 13：整数与浮点转换

> 状态：已确认
> 确认日期：2026-08-01

本议题使用议题 08 定义的 PDB，并使用议题 10 定义的 `cast<T>(value)` 内建语法。

## 1. 不进行隐式转换

具体整数与浮点类型之间不进行隐式转换：

```ink
let integer: i32 = 10;
let invalid: f32 = integer;           // 编译错误
let valid: f32 = cast<f32>(integer);  // 合法
```

不同宽度的具体浮点类型之间也不进行隐式转换：

```ink
let narrow: f32 = 1.5f32;
let invalid: f64 = narrow;           // 编译错误
let valid: f64 = cast<f64>(narrow);  // 合法
```

抽象数字字面量仍按照议题 06 直接实例化为目标类型，不属于具体类型间的隐式转换。

## 2. 混合类型运算

普通算术运算要求两个操作数具有相同的具体数值类型。Ink 不自动把整数提升为浮点数，也不自动寻找共同浮点宽度。

```ink
let integer: i32 = 10;
let floating: f32 = 2.5f32;

integer + floating;                    // 编译错误
cast<f32>(integer) + floating;         // 合法
```

```ink
let a: f32 = 1.0f32;
let b: f64 = 2.0f64;

a + b;                                 // 编译错误
cast<f64>(a) + b;                      // 合法
```

## 3. 整数转换为浮点数

`cast<F>(integer)` 把整数的数学数值转换为浮点类型 F：

- 使用 round-to-nearest、ties-to-even；
- 能精确表示时保留相同数学数值；
- 不能精确表示时舍入到最近的可表示浮点值；
- 超过有限范围时按照该浮点格式产生相应 Infinity；
- 不执行运行时精度检查。

```ink
let exact = cast<f32>(16_777_216);   // 16_777_216.0
let rounded = cast<f32>(16_777_217); // 16_777_216.0
```

当编译器能够确定转换丢失精度或溢出为 Infinity 时，必须产生 warning，但仍生成规定结果。

## 4. 浮点数转换为整数

`cast<I>(floating)` 首先向零截断浮点数的小数部分：

```ink
let positive = cast<i32>(12.9f64);  // 12
let negative = cast<i32>(-12.9f64); // -12
```

如果截断后的数学整数能由目标整数类型 I 表示，结果具有跨平台固定语义。

以下输入是 PDB：

- NaN；
- 正 Infinity 或负 Infinity；
- 向零截断后的整数超出目标整数类型的范围。

```ink
let first = cast<i32>(f64.infinity); // PDB
let second = cast<u8>(-1.0f32);      // PDB
let third = cast<i8>(128.0f32);      // PDB
```

编译器不为这些输入自动插入范围检查、饱和、回绕或 trap。它必须保留目标平台配置中记录的原生转换行为，并且不能把 PDB 输入当作 UB 或不可达路径。

## 5. 浮点宽度转换

`cast<F>(floating)` 用于不同浮点宽度之间的转换：

- 扩宽转换精确保留所有有限源值；
- 缩窄转换使用 round-to-nearest、ties-to-even；
- 缩窄溢出按照目标浮点格式产生相应 Infinity；
- 缩窄下溢遵守议题 12 及后续浮点环境议题规定的 subnormal 规则。

```ink
let wider = cast<f64>(1.5f32);
let narrower = cast<f32>(wider);
```

NaN payload 的具体传播继续由议题 12 保留为后续问题。

## 6. 编译期诊断

当编译器能够确定普通 `cast` 会发生下列情况时，必须产生 warning：

- 整数转浮点丢失精度；
- 整数转浮点溢出为 Infinity；
- 浮点缩窄丢失精度或溢出；
- 浮点转整数必然进入 PDB 输入域。

warning 不改变转换结果，也不授权优化器假设相应路径不可达。

## 7. 显式受检查转换

标准库可以提供跨平台固定语义的受检查或饱和转换：

```ink
value.checked_to<i32>()    // NaN、Infinity 或越界时返回可选结果
value.saturating_to<i32>() // 按标准库规定进行饱和
```

这些 API 可以产生必要的运行时比较，其成本由程序主动选择。最终 API 名称和 NaN 的饱和结果由标准库议题确认。

## 8. LLVM 降低约束

整数转浮点可以使用 LLVM `sitofp` 或 `uitofp`。不同浮点宽度可以在符合默认严格浮点环境时使用 `fpext` 或 `fptrunc`。

只有当编译器已经证明输入不是 NaN、Infinity 且截断结果位于目标整数范围内时，浮点转整数才可以使用普通 LLVM `fptosi` 或 `fptoui`。

对于仍可能进入 PDB 输入域的浮点转整数，Ink LLVM 后端必须使用议题 08 已确定的 inline-asm API 方案，直接生成目标平台转换操作。不能直接使用 LLVM `fptosi`/`fptoui`，因为 LLVM 会把越界结果表示为 poison。
