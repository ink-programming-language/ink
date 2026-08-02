# 议题 06：构造函数、隐式构造与字面量初始化

> 状态：已确认，议题 34、54、72 修订
> 确认日期：2026-08-01

## 1. 构造函数的内建名称

Ink 提供语言级构造函数。构造函数使用固定内建名称 `constructor`，不重复所属类型名称。

```ink
class Point {
    x: int;
    y: int;

    func constructor(x: int, y: int) {
        this.x = x;
        this.y = y;
    }
}
```

构造函数规则：

- 名称固定为 `constructor`；
- 必须声明在所属类型中；
- 可以重载；
- 不声明返回类型；
- 成功返回时必须完成全部字段初始化；
- 构造期间具有隐式的目标对象 `this`；
- 字段只能在完成初始化后读取；
- 完整对象构造完成前，`this` 不能逃逸；
- 构造函数不能作为已有对象的普通方法调用。

议题 54 的异步成员调用会把原始 `this` 指针保存进任务帧。构造函数内即使创建这种任务，也不得在完整对象构造完成前把它返回、存入外部对象或交给库设施，使它越过构造边界继续持有 `this`；这仍是本议题规定的构造期 `this` 逃逸编译错误。未被驱动且在构造函数内销毁的 `created` 任务不会执行方法体。

显式构造使用类型调用语法：

```ink
let point = Point(10, 20);
```

构造函数直接初始化目标对象的最终存储，不产生要求移动的中间对象。

## 2. 普通构造函数

普通 `constructor` 默认只能显式调用：

```ink
class Duration {
    milliseconds: int;

    func constructor(milliseconds: int) {
        this.milliseconds = milliseconds;
    }
}
```

```ink
let duration = Duration(1000); // 合法
let duration: Duration = 1000; // 编译错误
```

普通构造函数可以执行运行时代码，也可以像普通未标记函数一样抛出异常；函数签名不声明异常：

```ink
class File {
    func constructor(path: string) {
        this.handle = os.open(path);
    }
}

let file = File("data.txt");
```

## 3. `implicit` 构造函数

`implicit` 是构造函数修饰符，不是独立的函数类别。它扩大构造函数的可调用场景：

```text
普通 constructor：
    允许显式 Type(arguments) 调用

implicit constructor：
    允许显式 Type(arguments) 调用
    也允许目标类型明确时隐式调用
```

```ink
class Duration {
    milliseconds: int;

    implicit func constructor(milliseconds: int) {
        this.milliseconds = milliseconds;
    }
}
```

以下两种形式都合法：

```ink
let first = Duration(1000);
let second: Duration = 1000;
```

如果类型只声明了 `implicit constructor`，显式 `Type(argument)` 调用仍然合法。任何允许编译器隐式调用的构造函数，也必须允许程序员显式调用。

## 4. 隐式构造的目标位置

`implicit constructor` 可以在目标类型已经独立明确的新对象初始化位置使用，包括：

- 带显式类型的局部绑定；
- 已确定形参类型的函数调用；
- 已确定返回类型的返回表达式；
- 类字段初始化；
- 已知元素类型的数组初始化。

```ink
func wait(duration: Duration);

wait(1000);
```

```ink
func default_timeout() -> Duration {
    return 1000;
}
```

```ink
class Options {
    timeout: Duration;
}

let options = Options {
    timeout: 1000,
};
```

`implicit constructor` 不用于修改已经存在的对象：

```ink
var duration: Duration = 1000;
duration = 2000; // 编译错误，除非以后另行定义已有对象赋值规则
```

已有对象赋值与新对象初始化是两个独立的语言行为。Ink v0 不允许用户重载已有对象的赋值运算符。

## 5. `implicit constructor` 的限制

隐式构造函数：

- 必须恰好接受一个参数；
- 隐式满足议题 34 的 `[nothrow]`，异常不能逃逸；
- 可以是 `comptime`，也可以在运行时执行；
- 可以由普通运行时值触发，不限于编译器字面量；
- 不允许形成连续的隐式构造链；
- 不帮助推断原本未知的目标类型；
- 多个同级候选都可行时必须报告歧义。

隐式构造可以执行运行时代码和产生运行时副作用，但 `[nothrow]` 保证异常不能逃逸。API 作者应当优先把它用于不会失败、语义自然且成本可预期的初始化。可能抛出异常的构造必须保持显式。

```ink
class Port {
    value: u16;

    func constructor(value: int) {
        if value < 0 || value > 65535 {
            throw InvalidPort {};
        }

        this.value = cast<u16>(value);
    }
}

let port = Port(read_port());
```

## 6. 构造函数重载解析

显式 `Type(argument)` 调用考虑普通构造函数和 `implicit constructor`。

目标初始化只考虑 `implicit constructor`。

重载解析遵守以下原则：

1. 参数类型完全一致的候选优先；
2. 字面量可以直接实例化为候选参数要求的内建类型；
3. 选择构造函数前后都不调用其他用户定义隐式构造；
4. 两个候选处于同一优先级时直接报告歧义；
5. 不使用返回类型以外的未知上下文猜测目标类型。

```ink
class Value {
    implicit func constructor(value: i32) {
        // ...
    }

    implicit func constructor(value: i64) {
        // ...
    }
}

let value: Value = 10;    // 歧义
let value: Value = 10i32; // 选择 i32 重载
```

添加新的 `implicit constructor` 可能使已有源码产生歧义，因此属于需要进行源码兼容性评估的 API 变更。

## 7. 字面量的抽象值与默认类型

字面量在目标类型确定前是编译期抽象值，不先创建默认运行时值再进行隐式转换。

```ink
let small: u8 = 10;
```

这里的 `10` 直接实例化为 `u8`，不是先成为 `int` 再从 `int` 隐式转换到 `u8`。

字面量目标类型来自变量标注、构造函数参数、函数形参、返回类型、字段类型或数组元素类型。字面量不能由目标类型表示时产生编译错误。

上下文不足时使用以下默认类型：

```text
整数字面量       int
浮点字面量       f64
Unicode 标量字面量 u32
```

数字字面量可以使用类型后缀明确类型：

```ink
10u8
10i32
4096ptrsize
1.5f32
```

普通运行时变量不享受字面量实例化规则：

```ink
let source: int = 10;
let target: u8 = source; // 编译错误，不是字面量初始化
```

## 8. 标准库 `UnicodeScalar`

标准库的 `UnicodeScalar` 通过参数类型为编译期专用 `ScalarLiteral` 的 `implicit constructor` 接收编译器标量字面量：

```ink
class UnicodeScalar {
    private value: u32;

    implicit func constructor(literal: ScalarLiteral) {
        this.value = literal.codepoint;
    }

    func constructor(value: u32) {
        if !is_valid_unicode_scalar(value) {
            throw InvalidScalar {};
        }

        this.value = value;
    }
}
```

以下形式都合法：

```ink
let first = UnicodeScalar('A');
let second: UnicodeScalar = '中';
```

运行时整数不会隐式调用可能失败的普通构造函数：

```ink
let raw: u32 = read_codepoint();
let third = UnicodeScalar(raw);     // 合法，可能抛出异常
let fourth: UnicodeScalar = raw;    // 编译错误
```

Ink 不再需要独立的通用 `FromLiteral` 隐式转换接口。

## 9. 构造失败与部分初始化

普通构造函数抛出异常并在部分字段初始化后失败时，只清理已经成功初始化的字段，顺序遵守议题 03 和 34。

完整对象构造成功前不调用该对象自身的 `destructor`。构造成功以后，对象进入正常生命周期并按照 RAII 规则清理。
