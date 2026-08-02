# Tokenizer 议题 05：数字字面量

> 状态：已确认  
> 确认日期：2026-08-02

## 1. IntegerLiteral 与 FloatLiteral

Tokenizer 将数字字面量分成：

```text
IntegerLiteral
FloatLiteral
```

Token 保存完整原始 `raw`，并可以附带已经分析出的进制、数字分段、小数、指数和类型后缀信息。Tokenizer 不根据宿主机器整数或浮点类型提前截断数值。

## 2. 整数进制

整数字面量支持四种进制：

```ink
123       // decimal
0b1010    // binary
0o755     // octal
0xFF_A0   // hexadecimal
```

进制前缀固定为小写：

```text
0b  binary
0o  octal
0x  hexadecimal
```

`0B`、`0O` 和 `0X` 不是合法前缀。十六进制数字可以使用 `a-f` 或 `A-F`，其数值相同，原始大小写由 `raw` 保留。

前缀后必须至少有一个对应进制的数字。`0b`、`0o`、`0x` 和 `0xG` 都形成错误数字 Token，而不是拆成整数 `0` 与后续 Identifier。

## 3. 十进制前导零

没有显式进制前缀的整数字面量始终是十进制。前导零不会触发隐式八进制：

```ink
0
00
0010 // decimal 10
```

前导零属于原始拼写并保留在 `raw` 中。格式化器可以提出规范化建议，但 Tokenizer 不改变其数值进制。

## 4. 数字分组下划线

下划线 `_` 可以提高长数字、位掩码和二进制分组的可读性：

```ink
1_000_000
0b1111_0000
0xFFFF_FFFF
4_096ptrsize
1.234_567
1e10_000
```

下划线不进入数学值：

```text
1_000_000 == 1000000
0xFFFF_FFFF == 0xFFFFFFFF
```

它仍然是数字 Token `raw` 的一部分，因此 full-fidelity Token 流能够恢复作者的分组方式。

每个下划线必须严格位于同一数字组成部分的两个合法数字之间。以下形式非法：

```ink
_100
100_
1__000
0x_FF
1_.0
1._0
1e_10
1e10_
10_i32
```

Tokenizer 不自动删除非法位置的下划线后继续正常构建。

## 5. 十进制浮点形式

浮点字面量可以由小数点、十进制指数或浮点类型后缀确定：

```ink
1.0
0.5
1.25e10
1e10
1.5e-3
1.5E+3
10f32
```

小数点前后都必须至少有一个十进制数字：

```ink
.5 // 不构成 FloatLiteral
1. // 不构成 FloatLiteral
```

小数部分只有在 `.` 后紧接十进制数字时才进入 FloatLiteral。因此：

```ink
1.member
```

扫描为 `IntegerLiteral("1")`、`.` 和 `Identifier("member")`。

如果数字后出现两个点：

```ink
1..10
```

第一个点后不是数字，因此数字 Token 在 `1` 结束；后续点号如何最长匹配由标点和运算符议题决定。

## 6. 十进制指数

指数部分为：

```text
("e" | "E") ("+" | "-")? DecimalDigits
```

指数必须至少有一个数字。以下是错误数字 Token：

```ink
1e
1e+
1e-
1e_10
```

指数数字之间可以使用合法分组下划线。指数的正负号属于 FloatLiteral Token，而字面量最前面的正负号不属于。

Ink v0 不支持十六进制、二进制或八进制浮点字面量；这些进制只产生 IntegerLiteral。

## 7. 整数类型后缀

IntegerLiteral 可以使用以下后缀：

```text
i8 i16 i32 i64 i128
u8 u16 u32 u64 u128
int uint ptrsize byte
```

例如：

```ink
10i32
255u8
4096ptrsize
0xFFFFu32
0b1010byte
```

后缀紧接最后一个数字，中间不能有下划线或 Trivia。后缀是同一个 IntegerLiteral Token 的组成部分，不产生独立 `BuiltinType` Token。

IntegerLiteral 后缀可以用于任意整数进制，只要进制数字与后缀边界能够按合法数字集合确定。十六进制扫描先取得最大合法十六进制数字序列，再识别其后的整数后缀。

## 8. 浮点类型后缀

十进制 FloatLiteral 可以使用：

```text
f16 f32 f64
```

例如：

```ink
1.5f32
1e10f64
10f16
```

没有小数点或指数的十进制数字只要带浮点后缀，也产生 FloatLiteral。

非十进制字面量不能使用浮点后缀。由于 `f` 本身是合法十六进制数字，类似 `0x10f32` 按最大十六进制数字序列解释为一个十六进制 IntegerLiteral，而不解释为 `0x10` 加 `f32` 后缀。

带小数点或指数的 FloatLiteral 不能使用整数后缀：

```ink
1.0i32 // 错误数字后缀
1e3u8  // 错误数字后缀
```

## 9. 正负号不是字面量的一部分

字面量开头的 `+` 和 `-` 始终是独立 Token：

```ink
-128i8
+10
```

分别扫描为：

```text
Minus
IntegerLiteral("128i8")

Plus
IntegerLiteral("10")
```

Tokenizer 不对带后缀整数执行目标范围检查。语义阶段将紧邻的一元负号作用到抽象数学整数后，再检查后缀或目标类型，因此 `-128i8` 可以表示 `i8` 最小值，而正的 `128i8` 不能表示为 `i8`。

Trivia 不改变词法上的 Token 分离；一元运算和常量范围由 parser 与语义阶段决定。

## 10. 任意精度与延迟类型化

Tokenizer 和常量解析层必须能够表示任意源码长度的非负整数系数，不按宿主 `u64`、`i64` 或目标机器字提前溢出。

Token 可以记录：

```text
raw
base
integer digit ranges
fraction digit range
exponent sign and digit range
optional suffix
```

实现可以延迟构造任意精度数学值，直到常量求值或类型检查需要。无论是否延迟，不能先经过宿主 `double` 并丢失十进制精度。

默认类型、上下文目标类型、舍入、溢出和后缀范围检查继续使用语义议题已经确定的规则：整数字面量默认 `int`，浮点字面量默认 `f64`。

## 11. 最大候选与非法后缀

数字后无 Trivia 地紧接 IdentifierStart 时，Tokenizer 将其作为数字后缀候选检查。只有本议题列出的准确后缀合法：

```ink
10foo
1.0meters
0x12u7
10用户
```

这些形式产生覆盖整个连续候选的错误数字 Token 和未知后缀诊断，而不是拆成一个合法数字与一个 Identifier。该规则用于发现遗漏空白和后缀拼写错误。

正确写成两个独立表达式或 Token 时必须使用语法标点或 Trivia 分隔；是否允许这种相邻语法由 parser 决定。

## 12. Full-Fidelity Token

合法数字 Token 的 `raw` 包含：

- 原始进制前缀；
- 数字大小写；
- 全部合法下划线；
- 小数点和指数符号；
- 原始指数标记 `e` 或 `E`；
- 类型后缀。

错误数字 Token 同样覆盖并保留其完整原始候选字节。格式化器可以生成规范化拼写，但 Tokenizer 本身不能改写 `raw`。

## 13. 诊断

数字诊断至少应区分：

- 进制前缀后缺少数字；
- 出现不属于当前进制的数字；
- 下划线位置非法；
- 小数点缺少一侧数字而用户明显试图书写浮点数；
- 指数缺少数字；
- 未知或不适用于当前字面量种类的后缀；
- 当前不支持的非十进制浮点形式。

数值超出目标类型范围、浮点舍入和抽象字面量无法实例化不属于词法错误，由常量求值和类型检查阶段诊断。
