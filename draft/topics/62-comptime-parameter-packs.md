# 议题 62：编译期参数包是普通编译期序列

> 状态：已确认，议题 64、66、68—71 补充绑定、反射、运行时包、元组与重载；Parser 议题 29、31 统一列表展开与泛型包声明；议题 67 规定声明名称必须静态写在源码中
> 确认日期：2026-08-02

## 1. 参数包只负责接收可变数量的编译期实参

Ink 使用尾随 `...` 声明编译期参数包：

```ink
class Tuple<Types: type...> {
    // ...
}

class Tensor<
    Element: type,
    Dimensions: ptrsize...
> {
    // ...
}
```

泛型声明的参数列表 `<...>` 天然声明编译期形参，所以这里只写 `Types: type...`、`Dimensions: ptrsize...`；旧写法 `Types: comptime type...` 不属于语法。

调用时，参数包接收泛型实参列表中对应位置之后的零个或多个实参：

```ink
Tuple::<>
Tuple::<i32>
Tuple::<i32, String, bool>

Tensor::<f32, 3, 224, 224>
```

`Tuple::<>` 中的 `Types` 是空序列；`Tensor::<f32, 3, 224, 224>` 中的 `Element` 是 `f32`，`Dimensions` 是 `[3, 224, 224]`。

议题 64 规定编译器不从普通函数实参或返回上下文推导参数包。包只接收尖括号中显式提供的剩余泛型实参；没有剩余实参时确定地绑定为空序列。

## 2. 参数包在声明体内是普通编译期序列

参数绑定完成后，参数包不再是一种需要特殊展开规则的模板实体：

```text
Types      : type[] known in the generic context
Dimensions : ptrsize[] known in the generic context
```

它们是不可变、编译器管理的编译期序列，可以使用普通序列操作：

```ink
comptime {
    print(Types.length);

    for (const T in Types) {
        print(reflect(T).name);
    }
}
```

辅助函数可以直接接收整个序列：

```ink
func total_size(Types: const type[]) -> ptrsize {
    var result: ptrsize = 0;

    for (const T in Types) {
        result += reflect(T).size;
    }

    return result;
}
```

序列的元素类型仍然准确。`type...` 只能接收类型值，`ptrsize...` 只能接收 `ptrsize` 编译期值；参数包不会为了容纳异构值而隐式擦除成运行时 `Any`。

## 3. 使用元组存储并以 `comptime for` 展开静态声明

异构参数包需要落入一个对象时，可以直接展开为普通元组类型：

```ink
class Tuple<Types: type...> {
    var storage: (...Types);
}
```

需要为每个元素重复名称已经写在源码中的声明时，使用议题 61 的普通编译期循环。例如，参数类型不同会形成普通重载：

```ink
class Encoder<Types: type...> {
    comptime for (const Element in Types) {
        func encode(value: Element) -> String {
            return encode_value(value);
        }
    }
}
```

这取代 C++ 模板中散布在类型、表达式和声明各处的包展开语法。循环体可以使用普通条件、辅助函数、反射和错误处理；每项展开结果仍须经过名称绑定、类型检查、布局依赖检查和 IR verifier。

循环值可以进入声明中的类型和值表达式，但不能替换声明名称。因此 v0 不能根据序号动态产生 `item0`、`item1` 等字段；这类异构存储应使用上面的元组成员。若将来确实需要动态声明名称，必须另行设计统一、卫生的 identifier splice，而不是为参数包增加字段专用语法。

参数包顺序属于语义的一部分。`Tuple::<i32, bool>` 与 `Tuple::<bool, i32>` 是不同闭合类型；循环必须按照实参顺序观察元素。

## 4. 显式展开只用于允许多个元素的列表

已有编译期序列需要逐项传给另一个可变参数声明时，使用前缀展开：

```ink
Other::<...Types>
```

普通元素和展开序列可以组合：

```ink
Other::<Header, ...Types, Footer>
```

`...expression` 只在规范明确允许多个元素的列表位置把序列展开成若干独立元素，不是普通一元运算符，不能产生运行时值，也不能用于创建同名重载候选。Parser 议题 29 除泛型实参列表外，还允许类型序列在元组类型列表和函数类型参数列表中展开：

```ink
(Header, ...Types, Footer)
func(Context&, ...Types) -> Result
```

展开操作数可以是完整表达式，例如 `Other::<...select_types()>`。对应位置要求它在编译期产生序列；元组和函数类型列表进一步要求每个展开元素都是 `type`。

如果被调用声明或列表消费者不接受对应数量或类型的元素，展开后的普通检查失败。展开空序列等价于在该位置没有元素。

## 5. v0 的声明限制

为了让参数绑定保持确定，Ink v0 规定：

- 一个泛型参数列表最多有一个参数包；
- 参数包必须是最后一个泛型形参；
- 参数包可以接收零个实参；
- 包中每个实参必须满足形参声明的准确类型；
- 参数包自身不能带默认实参；
- 不根据函数体使用方式反推参数包边界。

因此以下声明非法：

```ink
class Invalid<
    Prefix: type...,
    Last: type
> {}
```

未来即使允许命名泛型实参，也不能让位置实参在多个包或包后形参之间进行回溯分配。

## 6. 参数包不改变重定义规则

参数包的形参类型和位置参与普通泛型签名身份，但函数体中的 `comptime if` 和包长度条件不参与身份：

```ink
func inspect<Types: type...>() {
    comptime if (Types.length == 1) {
        // ...
    }
}

func inspect<Types: type...>() { // 编译错误：重定义
    comptime if (Types.length > 1) {
        // ...
    }
}
```

需要根据长度、元素类型或反射结果选择行为时，写在一个泛型体中。Ink 不使用包替换失败建立 SFINAE 候选集合。

按照议题 70，显式编译期实参绑定包之后才闭合普通候选签名；包绑定或展开失败不能静默移除一个重载候选。

## 7. Partial Evaluation 与实例身份

参数包是议题 61 Partial Evaluation 的普通 `Known(sequence)` 输入：

```text
generic declaration identity
+ canonical element sequence
+ target configuration
→ closed instance
```

序列长度、顺序、每个类型身份和每个值都进入实例缓存键。部分求值器可以执行编译期循环、展开闭合声明并残留仍依赖运行时值的 InkIR；固定点结束后没有开放参数包进入运行时代码。

相同规范化实参序列请求同一闭合实例。后端可以合并机器码完全相同的内部实现，但不能因此把语义不同的闭合类型变成同一类型。

## 8. 反射

编译期反射可以观察：

- 开放声明是否具有参数包；
- 参数包名称、元素类型和位置；
- 闭合实例绑定后的元素数量、顺序和值；
- 由各元素展开并验证后的成员声明。

运行时动态反射只观察编译期间已经实例化并登记的闭合实例，不携带开放参数包，也不能通过提交运行时序列请求新实例。

## 9. 与运行时可变参数的边界

本议题只定义泛型尖括号内的编译期参数包：

```ink
Tuple::<i32, String>
Tensor::<f32, 3, 224, 224>
```

它不定义 C 风格 varargs 或运行时元组 ABI。议题 68 使用本议题的类型序列定义 `values: Types...` 异构运行时参数包；议题 45 的 `all(task1, task2, ...)` 可以使用两项议题作为静态调用基础，其结果元组 ABI 仍须单独确定。

## 10. 源码分发与实现

开放参数包声明按照议题 61 只以源码分发。使用方编译时把实参绑定为规范化编译期序列，并在固定点展开循环中生成闭合 InkIR；不存在跨二进制模块传递开放参数包或请求远端实例化的 ABI。

LLVM 不需要理解参数包。前端在进入 LLVM lowering 前已经展开所有包循环和列表展开，只向后端交付闭合类型、普通函数签名和运行时控制流。

## 11. 后续问题

参数包长度和生成声明数量的资源预算留给独立议题。
