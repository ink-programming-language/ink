# 议题 05：基础类型与 `ptrsize`

> 状态：已确认，字面量类型规则由议题 06 补充
> 确认日期：2026-08-01

## 1. 基础类型集合

Ink 核心语言提供以下基础类型：

```text
i8  i16  i32  i64  i128
u8  u16  u32  u64  u128
int uint ptrsize

f16 f32 f64

bool byte void never
```

其中：

- `iN` 是 N 位有符号整数；
- `uN` 是 N 位无符号整数；
- `int` 固定等于 `i64`；
- `uint` 固定等于 `u64`；
- `byte` 是 `u8` 的透明别名；
- `bool` 只具有 `true` 和 `false`；
- `void` 表示没有返回值；
- `never` 表示控制流不会正常返回；
- `f16`、`f32` 和 `f64` 使用对应的 IEEE 754 二进制格式。

`bool` 不与整数进行隐式转换。

## 2. 单一 `ptrsize`

Ink 不提供 `isize` 和 `usize`，统一使用 `ptrsize`。

`ptrsize`：

- 宽度与目标指针相同；
- 数值范围为 `0 .. 2^N - 1`；
- 采用无符号、模 `2^N` 的整数语义；
- 名称不区分 `i`/`u`，但算术规则必须明确；
- 用于数组和切片长度、索引、内存大小、对齐值和地址整数表示；
- 与指针显式往返转换时保留全部地址位。

```ink
let length: ptrsize = values.length;
```

指针与 `ptrsize` 之间必须使用议题 11 定义的 `ptrcast` 显式转换，不使用 `as`。

Ink 不提供返回指针宽度有符号整数的普通指针减法运算符。需要计算地址或元素距离时使用显式标准库函数：

```ink
let distance: int = pointer_distance(begin, end);
```

结果不能由 `int` 表示时产生 trap。首批 Ink 目标的对象大小必须受到相应可表示范围限制；未来宽于 64 位的目标需要在目标规范中重新确认距离结果类型。

## 3. 不提供内置 `char`

Ink 核心语言不提供 `char` 基础类型。字节使用 `byte` 或 `u8` 表示；Unicode 标量类型由标准库提供。

核心语言仍然识别 Unicode 标量字面量：

```ink
'A'
'中'
'\n'
'\u{1F600}'
```

字面量必须恰好表示一个 Unicode 标量值。代理项范围 `U+D800..U+DFFF` 和大于 `U+10FFFF` 的值不是合法 Unicode 标量字面量。

字面量的默认类型、上下文类型解析和构造规则由议题 06“构造函数、隐式构造与字面量初始化”规定。

## 4. 标准库 Unicode 标量类型

标准库提供概念上的 32 位透明包装类型：

```ink
[transparent]
class UnicodeScalar {
    value: u32;
}
```

`UnicodeScalar` 只允许以下数值：

```text
U+0000 .. U+D7FF
U+E000 .. U+10FFFF
```

标准库负责：

- Unicode 标量验证；
- UTF-8、UTF-16 和 UTF-32 编解码；
- Unicode 分类与大小写操作；
- 字素簇遍历；
- Unicode 数据版本管理。

固定 32 位只表示单个 Unicode 标量，不表示用户感知字符或字素簇。拥有型字符串仍可使用 UTF-8 存储，不会因为 `UnicodeScalar` 为 32 位而整体膨胀为 UTF-32。

## 5. 核心与标准库的边界

核心语言只负责解析并验证 Unicode 标量字面量的词法合法性，不内置 Unicode 分类数据库。

标准库通过议题 06 定义的 `implicit constructor`，把标量字面量初始化为具有有效值不变量的 `UnicodeScalar`。
