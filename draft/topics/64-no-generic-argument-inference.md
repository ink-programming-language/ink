# 议题 64：Ink v0 不支持泛型实参推导

> 状态：已确认，议题 65、70 补充显式默认值与重载解析  
> 确认日期：2026-08-02

## 1. 泛型调用显式提供编译期实参

Ink v0 不从普通函数实参、返回类型、赋值目标或函数体使用方式推导泛型编译期参数。调用开放泛型声明时，调用者必须显式提供每个没有其他既定绑定方式的编译期实参：

```ink
func duplicate<T: comptime type>(
    value: const T&,
) -> (T, T) {
    return (value, value);
}

let pair = duplicate<i32>(&value); // 合法
let pair = duplicate(&value);      // 编译错误：缺少 T
```

`value` 的准确类型即使显然是 `i32`，也不会隐式绑定 `T`。

## 2. 类型和值参数遵循同一规则

值形式的编译期参数同样不从运行时参数类型中提取：

```ink
func array_length<
    T: comptime type,
    N: comptime ptrsize,
>(value: const T[N]&) -> ptrsize {
    return N;
}
```

调用者必须写出完整实参：

```ink
array_length<byte, 16>(&bytes); // 合法
array_length(&bytes);           // 编译错误：缺少 T、N
```

编译器随后仍会检查 `bytes` 是否确实能绑定到 `const byte[16]&`。显式编译期实参不会绕过普通参数类型检查。

## 3. 不从返回上下文反推

返回类型和赋值目标不参与泛型绑定：

```ink
func make<T: comptime type>() -> T;

let first: i32 = make<i32>(); // 合法
let second: i32 = make();     // 编译错误：缺少 T
```

嵌套调用、函数返回、显式变量类型和其他预期类型上下文都不能补全未给出的泛型实参。

## 4. 不求解类型模式或编译期方程

Ink v0 不从 `T[N]`、`Container<T>`、指针层级或其他类型模式中提取泛型参数，也不反向求解任意编译期表达式：

```ink
func consume<N: comptime ptrsize>(
    value: const byte[N + 1]&,
) {}

consume<9>(&bytes); // 显式 N；随后验证 bytes 是 byte[10]
consume(&bytes);    // 不尝试求解 N + 1 = 10
```

该规则避免把普通 Partial Evaluation 扩展成类型统一器或编译期符号方程求解器。

## 5. 不提供泛型类型构造推导

开放泛型类型不能仅凭构造函数实参闭合：

```ink
let first = Box<i32>(10); // 合法
let second = Box(10);     // 编译错误：Box 缺少类型实参
```

Ink v0 不提供 C++ class template argument deduction 或 deduction guide。普通工厂函数如果自身是泛型，也仍须显式给出其泛型实参：

```ink
let value = make_box<i32>(10);
```

## 6. 参数包仍可显式为空

议题 62 的尾随参数包可以接收调用点明确提供的剩余泛型实参：

```ink
Tuple<>                  // Types 明确绑定为空序列
Tuple<i32, String, bool> // Types 明确绑定三个类型
```

空包不是推导结果，而是泛型实参列表在包位置没有剩余元素时的确定绑定。

## 7. 重载解析不尝试推导

泛型候选缺少必需编译期实参时，不会根据普通调用实参尝试形成实例，也不会进入函数体后再决定是否可用：

```ink
func inspect<T: comptime type>(value: const T&);
func inspect(value: const i32&);

inspect(&number);      // 只按普通非泛型候选处理
inspect<i32>(&number); // 明确请求泛型实例
```

函数体中的 `if comptime`、成员是否存在以及具体实例能否通过类型检查都不用于反推缺少的泛型实参。

议题 70 固定候选集合边界：`function(...)` 只考虑非泛型声明，`function<...>(...)` 才考虑能够绑定这些显式编译期实参的泛型声明。闭合签名或已选中函数体失败都不会触发 SFINAE 式回退。

## 8. Partial Evaluation

议题 61 的 Partial Evaluation 只在泛型编译期参数已经由显式实参、议题 62 的包绑定或未来独立确认的其他规则完全绑定后开始。

```text
explicit comptime arguments
→ canonical argument values
→ request closed instance
→ partial evaluation
```

缺少实参时不会创建待猜测实例，也不会把运行时实参作为未知约束交给部分求值器。

## 9. 诊断

缺少泛型实参的诊断必须列出：

- 被调用的开放泛型声明；
- 尚未绑定的参数名称和类型；
- 正确的显式调用形式；
- 参数包是否已经绑定为空或接收了剩余实参。

诊断不建议添加 `where` 约束、deduction guide 或改变普通函数参数类型，因为这些机制都不能在 v0 启用推导。

## 10. 后续兼容性

未来可以独立增加有限的函数泛型实参推导，但不能改变已经显式写出实参的程序所选择的闭合实例。

议题 65 允许参数声明使用尾随默认值，但不把默认值视为推导。调用者必须写出泛型 `<>`，随后才能按声明规则省略有默认值的尾部编译期参数；v0 不支持名称绑定。

未来推导是否只限函数输入的直接类型位置仍须独立讨论。
