# 议题 66：开放泛型声明是一等编译期值

> 状态：已确认，议题 67、70、71 补充静态声明展开、泛型重载与元值元组；Parser 议题 40 定义 class 类型表达式
> 确认日期：2026-08-02

## 1. 目的

Ink 允许把开放泛型声明作为编译期实参、局部值和序列元素，以表达“把一个泛型交给另一个泛型”的高阶泛型能力：

```ink
func wrap_value<
    Wrapper: GenericTypeDecl,
    Source: type
>() -> type {
    const Wrapped: type = Wrapper.instantiate<Source>();

    return class {
        var value: Wrapped;
    };
}
```

调用：

```ink
const UserPatch: type =
    wrap_value<Optional, User>();
```

这里传递的是开放声明 `Optional`，不是某个已经闭合的 `Optional<T>`。

## 2. 内建编译期元类型

Ink 提供以下编译期专用元类型：

```text
GenericDecl          任意开放泛型声明的公共只读视图
GenericTypeDecl      实例化后产生闭合 type 的开放声明
GenericFunctionDecl  实例化后产生闭合函数声明的开放声明
FunctionDecl         已经闭合的函数声明句柄
```

这些元类型属于编译器语义域，不是带有运行时对象布局、vtable 或动态分配约定的普通用户类。它们可以由编译期代码保存、比较、放入编译期序列和传递给其他编译期调用，但不能残留到运行时程序。

`GenericTypeDecl` 至少覆盖开放的泛型 `class`、`enum`、`interface`，以及未来明确产生类型的泛型别名声明。`GenericFunctionDecl` 覆盖普通、成员、虚、接口和异步泛型函数声明；实例化后的函数种类不因此改变。

## 3. 开放声明与闭合类型严格区分

给定：

```ink
class Vector<T: type> {
    // ...
}
```

两种表达式具有不同编译期类型：

```text
Vector      : GenericTypeDecl
Vector<i32> : type
```

开放声明不是 `type`，不能用作字段类型、数组元素类型、函数运行时参数类型或 `sizeof` 的闭合操作数：

```ink
var field: Vector;              // 编译错误：开放声明不是 type
var field: Vector<i32>;         // 合法
reflect(Vector).size;       // 编译错误：开放声明没有对象大小
reflect(Vector<i32>).size;  // 合法
```

同理，`GenericFunctionDecl` 不是可直接进入运行时调用约定的闭合函数值。

## 4. 取得开放声明值

当表达式位置明确要求 `GenericTypeDecl` 时，可以直接使用未带尖括号的开放泛型名称：

```ink
func make_cache<
    Storage: GenericTypeDecl,
    Key: type,
    Value: type
>() -> type {
    return Storage.instantiate<Key, Value>();
}

const CacheType =
    make_cache<HashMap, String, User>();
```

编译期序列可以显式声明元素元类型：

```ink
const Containers: GenericTypeDecl[] = comptime [
    Vector,
    Deque,
    LinkedList,
];
```

议题 71 允许用现有元组把 `GenericTypeDecl` 与 `type`、整数、`Identifier` 等其他编译期值组合成一个异构结构，而不引入 `ComptimeAny`。只含声明句柄的同质可变长度集合仍适合使用本节的编译期序列。

普通非泛型类型名称不能转换为 `GenericTypeDecl`。名称指向重载集合、多个同名泛型函数或其他不能唯一确定声明的情况必须报歧义。议题 70 定义直接泛型调用的重载解析，但不允许把重载集合隐式压缩成一个 `GenericFunctionDecl`。需要传递其中一种行为时，用户定义一个名称唯一的普通泛型薄包装。

## 5. 完整实例化操作

开放类型声明值使用编译期内建操作 `instantiate` 请求闭合类型：

```ink
const First: type =
    Container.instantiate<i32>();

const Second: type =
    Matrix.instantiate<f32, 4, 4>();
```

`instantiate<...>()` 的尖括号是异构编译期实参列表，由目标声明自己的形参逐项检查；它不要求先把不同类型的实参擦除进一个运行时 `Any[]`。`instantiate` 不是可取得地址的普通运行时方法，而是编译器识别的编译期声明应用操作。

直接写出的泛型名称继续使用普通简写：

```ink
Vector<i32>
```

只有通过变量、参数或序列元素持有开放声明时才需要显式 `.instantiate<i32>()`。

## 6. 使用统一泛型绑定规则

`instantiate<...>()` 完全复用已经确认的泛型绑定规则：

1. 议题 64：不从其他值推导缺少的实参；
2. 显式实参按照位置绑定固定形参；
3. 议题 65：省略的连续尾部固定形参使用声明默认值；
4. 议题 62：最终参数包接收其余显式实参，或绑定为空序列；
5. 所有编译期值规范化后请求议题 61 的闭合实例；
6. Partial Evaluation 和生成结果验证完成后返回闭合 `type` 或函数声明句柄。

参数数量、实参类型、默认表达式、访问权限或实例体检查失败均为普通编译期错误，并附带开放声明、实际参数和实例化调用栈。

## 7. v0 不支持偏应用

`instantiate` 必须绑定全部没有默认值的固定形参，并确定最终参数包。它只返回闭合结果，不返回一个形参数量减少的新开放声明：

```ink
class Map<
    Key: type,
    Value: type
> {}

Map.instantiate<String>();       // 编译错误：缺少 Value
Map.instantiate<String, User>(); // 合法，返回 type
```

Ink v0 不定义柯里化、占位参数、泛型 lambda 或高阶 kind 运算。需要预绑定一部分参数时，用户显式写一个新的普通泛型包装声明。

## 8. 泛型参数反射

`reflect(GenericDecl)` 返回开放声明信息，至少包括：

```text
declaration kind
fully qualified name
lexical module and visibility
ordered generic parameter list
result kind: type or function declaration
attributes and metadata
```

每个泛型参数描述至少包含：

```text
name
value_type
position
is_pack
has_default
```

反射可以查询默认值是否存在以及它属于哪个声明，但不因此取得原始源码文本、token stream 或函数体 AST。默认表达式由声明自己的绑定器在实际省略时执行，调用者不能复制文本后在其他权限上下文中重新求值。

可以在实例化前给出定制诊断：

```ink
const info = reflect(Container);

comptime if (info.parameters.length != 1) {
    compile_error("Container must accept one argument");
}
```

这只是预检查；最终正确性仍由 `instantiate` 的标准绑定和实例验证保证。

## 9. 不引入高阶 kind 系统

`GenericTypeDecl` 表示所有产生类型的开放泛型声明，不在其静态元类型中编码类似下列形状：

```text
type -> type
(type, type) -> type
(type, ptrsize...) -> type
```

调用者通过反射查询形参，并在 `instantiate` 时接受准确检查。Ink v0 不引入 C++ `template<template<...>>`、高阶类型 kind、形状子类型或基于形参数量的重载身份。

这样允许一个高阶编译期函数接收多种泛型声明，同时把错误保持在明确的编译期应用位置。

## 10. 泛型函数声明

开放泛型函数名称在唯一确定且上下文要求时可以形成 `GenericFunctionDecl`：

```ink
const SortDecl: GenericFunctionDecl = comptime sort;
const SortI32: FunctionDecl =
    comptime SortDecl.instantiate<i32>();
```

这会请求并返回闭合 `sort<i32>` 声明，可用于编译期反射、验证以及静态声明区域控制中的类型或表达式选择。

擦除后的 `FunctionDecl` 不携带一个可由普通运行时类型检查器直接调用的静态函数签名，因此 v0 不规定：

```ink
SortI32(values); // 不由本议题允许
```

普通源码需要调用时直接使用类型明确的泛型调用：

```ink
sort<i32>(values);
```

未来若需要从声明句柄得到带准确签名的函数值，必须设计独立的类型化函数声明或函数指针转换，不得通过擦除句柄绕过参数检查。

## 11. 声明身份、相等与缓存

开放声明值按规范化源码声明身份比较，而不是按名称、形参数量或生成机器码比较。同一模块版本中的同一声明产生相等句柄；两个结构完全相同但分别声明的泛型仍不相等。

闭合实例缓存键继续使用：

```text
generic declaration identity
+ canonical comptime arguments
+ target configuration
+ tracked comptime dependencies
```

从开放声明值和从直接 `Vector<i32>` 语法请求相同规范化实例时得到同一闭合类型身份。

源码分发时，声明身份由当前兼容编译中的源码模块和声明位置建立。它不是可跨编译器版本保存或通过 C ABI 交换的稳定整数 ID。

## 12. 访问控制

取得、传递、反射和实例化泛型声明分别遵守议题 20 的词法访问规则：

- 公开开放声明可以传给其他模块；
- 私有开放声明只能在正常可访问的词法上下文中取得和实例化；
- 完整反射能够看见私有声明结构，不等于获得其实例化或调用权限；
- 把声明句柄交给外部高阶泛型不会让该高阶泛型借用调用模块的私有权限；
- 实例体的访问权限继续属于泛型定义模块，而不是实例化位置。

源码中声明的泛型只有在当前固定点轮次完成收集和验证后，才能形成稳定 `GenericTypeDecl` 或 `GenericFunctionDecl`；未完成的内部暂定声明不能冒充声明句柄。

## 13. 运行时禁止逃逸

以下元值都只能存在于编译期：

```text
GenericDecl
GenericTypeDecl
GenericFunctionDecl
FunctionDecl as a declaration handle
```

它们不能：

- 成为运行时字段或普通函数运行时参数；
- 写入静态运行时数据区；
- 通过动态反射 `DynamicRef` 传递；
- 跨 `extern "C"` 边界；
- 序列化为可在另一编译中恢复的稳定声明句柄；
- 在运行时提交新实参触发 JIT 实例化。

运行时动态反射仍只观察编译期间已经实例化并登记的闭合实例。

## 14. 高阶泛型与静态声明展开

高阶泛型可以把闭合 `type` 和 `FunctionDecl` 交给议题 61、63、67 的普通类型表达式、编译期条件和静态声明展开。用户不操作 Builder，也不能把声明名称格式化成源码后重新解析。

议题 67 明确 v0 不提供 `field(...)`、`function(...)` 或动态声明名称。高阶泛型可以改变普通声明中使用的类型和值，但声明名称仍必须真实写在源码中。

编译期代码根据反射批量实例化泛型时，每个请求都进入同一固定点队列和资源预算。无限地为新类型继续实例化同一泛型必须由实例栈、声明展开计数和固定点收敛检查诊断。

## 15. LLVM 与二进制边界

LLVM 不理解开放泛型声明或编译期声明句柄。前端在固定点展开中完成所有 `instantiate`，只把闭合类型、普通函数和运行时 InkIR 交给 LLVM lowering。

开放泛型声明按照议题 61 只能通过源码分发。需要二进制公开功能时，必须导出已经闭合并适配为 C ABI 的函数；`GenericTypeDecl` 和 `GenericFunctionDecl` 不属于可导出的二进制 ABI。
