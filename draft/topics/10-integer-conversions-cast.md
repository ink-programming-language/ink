# 议题 10：整数转换与 `cast`

> 状态：已确认，议题 34、37 确认异常绑定中的 `as`，其他用途待定
> 确认日期：2026-08-01

## 1. 不进行整数隐式转换

除抽象字面量的目标类型实例化外，两个不同的具体整数类型之间不进行隐式转换。

```ink
const source: u8 = 10;
const invalid: u16 = source;        // 编译错误
const valid: u16 = cast<u16>(source);
```

该规则也适用于函数实参、返回值、字段初始化和数组元素初始化。

```ink
func consume(value: i64) {}

const value: i32 = 10;
consume(value);             // 编译错误
consume(cast<i64>(value));  // 合法
```

## 2. 字面量不是整数隐式转换

抽象字面量继续遵守议题 06 的目标类型实例化规则：

```ink
const value: u8 = 10; // 10 直接实例化为 u8
```

这里不存在从 `int` 到 `u8` 的隐式转换。目标类型不能表示字面量时产生编译错误。

```ink
const value: u8 = 300; // 编译错误
```

## 3. 混合类型整数运算

普通二元整数运算要求两个操作数具有相同的具体类型。Ink 不采用 C/C++ 的整数提升和 usual arithmetic conversions。

```ink
const small: u8 = 10;
const large: u16 = 20;

const first = small + 1;                  // 合法：1 实例化为 u8
const second = small + large;             // 编译错误
const third = cast<u16>(small) + large;   // 合法
```

有符号整数与无符号整数也不会自动寻找共同类型：

```ink
const signed: i32 = -1;
const unsigned: u32 = 1;

signed < unsigned; // 编译错误
```

## 4. `cast<T>(value)`

Ink 使用编译器内建语法 `cast<T>(value)` 表示显式数值转换。

```ink
const wider = cast<u64>(value);
const narrower = cast<u8>(value);
```

`cast`：

- 是保留的编译器内建语法，不是可被遮蔽或重载的普通函数；
- 不调用用户定义的构造函数；
- 不参与用户定义的隐式构造链；
- 不执行运行时范围检查；
- 不因目标值超出范围而产生隐式 trap。

用户类型继续通过 `Type(arguments)` 调用构造函数：

```ink
const number = cast<u16>(source); // 内建数值转换
const point = Point(10, 20);      // 用户构造函数
```

整数与浮点数之间以及不同浮点宽度之间的 `cast` 由议题 13 规定。

## 5. 整数转换规则

从整数类型 S 转换到整数类型 T 时：

1. 取源值模 `2^N`，其中 N 是目标类型 T 的位宽；
2. 如果 T 是无符号类型，直接把所得位模式解释为非负整数；
3. 如果 T 是有符号类型，按照 N 位二进制补码解释所得位模式。

该规则统一覆盖扩宽、缩窄和有无符号转换：

- 有符号扩宽等价于符号扩展；
- 无符号扩宽等价于零扩展；
- 缩窄保留目标宽度的低位；
- 同宽度有无符号转换保留全部位。

```ink
const first = cast<i8>(255u16); // -1
const second = cast<u8>(-1i32); // 255
const third = cast<u8>(300);    // 44，并产生 warning
```

这些结果是固定语义，不是 PDB。

## 6. 编译期诊断

当编译器能够确定 `cast` 改变了源值的数学数值时，必须产生 warning，但仍生成规定的模转换结果。

```ink
const value = cast<u8>(300);
// warning: integer cast changes value from 300 to 44
```

warning 不阻止编译，也不允许优化器把该路径视为不可达。

标准库可以提供显式回绕、检查和饱和转换，以表达意图或获得不同语义：

```ink
value.wrapping_to<u8>()   // 与 cast 数值结果相同，抑制回绕 warning
value.checked_to<u8>()    // 不能表示时返回可选结果
value.saturating_to<u8>() // 饱和到目标边界
```

这些 API 的最终命名留给标准库议题确认。

## 7. `ptrsize`

`ptrsize` 与所有固定宽度整数类型之间也不进行隐式转换。

```ink
const length: ptrsize = values.length;
const fixed: u64 = cast<u64>(length);
const native: ptrsize = cast<ptrsize>(fixed);
```

转换使用目标平台上 `ptrsize` 的实际位宽，并遵守同一模转换规则。转换结果随 `ptrsize` 位宽改变是目标类型本身的性质，不构成 UB。

指针与 `ptrsize` 的转换、指针类型转换以及位模式重解释不属于普通整数 `cast`，由后续独立议题规定。

## 8. `bool`

`bool` 不是整数，不能通过普通 `cast` 与整数互相转换。

```ink
cast<bool>(value); // 编译错误
cast<u8>(flag);    // 编译错误
```

程序必须明确表达所需逻辑：

```ink
const flag = value != 0;
const numeric: u8 = if flag { 1 } else { 0 };
```

## 9. 其他转换类别

数值转换以外的转换不复用普通 `cast` 语义。议题 11 已确认以下独立内建转换：

```ink
bitcast<T>(value) // 位模式重解释
ptrcast<T>(value) // 裸指针类型和地址整数转换
```

详见 [`11-bitcast-ptrcast.md`](./11-bitcast-ptrcast.md)。

## 10. `as` 用于异常捕获绑定

`as` 不用于类型转换。议题 34 已确认它用于为被捕获异常绑定局部名称：

```ink
catch IoError as error {
    log(error);
}
```

其他“绑定名称”或“建立别名”的语法场景仍待定，例如：

```ink
import network as net;
```

资源作用域绑定和模式别名是否使用 `as`，必须分别在对应语言特性议题中讨论。本议题不把异常语法中的决定自动推广到其他上下文。
