# 议题 71：现有元组直接承载异构编译期值

> 状态：已确认，Parser 议题 30 确认无期望类型时的类型值元组
> 确认日期：2026-08-02

## 1. 不增加新的元值容器

Ink 不为编译期元编程另外引入 `MetaTuple`、`ComptimeAny`、`ValueList` 或类型擦除容器。议题 69 的普通元组语法可以直接在 `comptime` 求值中保存异构编译期值：

```ink
const specification = comptime (
    i32,
    cast::<ptrsize>(16),
    Vector
);
```

这些元素的编译期类型依次可以是：

```text
type
ptrsize
GenericTypeDecl
```

因此整个值具有结构化编译期元组类型：

```text
(type, ptrsize, GenericTypeDecl, Identifier)
```

异构性由元组每个位置的准确静态类型表达，不需要把元素统一转换成 `Any`。

Parser 议题 30 规定没有 `type` 期望时，圆括号中的多个类型值默认形成普通元组值，而不是合并成一个元组类型值。因此：

```ink
const Types = (i32, String);       // 值类型为 (type, type)
const Pair: type = (i32, String);  // 值本身是元组类型 (i32, String)
```

本议题使用前一种普通元组值承载异构编译期元值。

## 2. `comptime` 只决定求值阶段

元组不会因为在编译期构造就变成另一种语言类型：

```ink
const dimensions = comptime (1920, 1080);
```

如果所有元素都是可运行时表示的普通值，编译器可以把已知结果作为普通 `(i32, i32)` 常量残留到运行时；它继续使用议题 69 的正常布局和生命周期规则。

如果任一元素是 `type`、`GenericDecl`、`FunctionDecl` 或其他编译期专用值，完整元组不能物化到运行时：

```ink
const metadata = comptime (i32, Vector);

runtime_store(metadata); // 编译错误：元组包含编译期专用值
```

从该元组投影出的可运行时表示元素仍可以单独残留。编译器判断的是最终残留值中是否含有编译期专用成分，不是见到整个元组后永久污染每个元素。

## 3. 位置访问保持准确类型

位置访问沿用议题 69 的 `.0`、`.1` 以及编译期常量索引：

```ink
const ElementType: type = specification.0;
const Alignment: ptrsize = specification.1;
const Container: GenericTypeDecl = specification.2;
```

每个投影的类型由位置静态确定。普通运行时整数不能索引异构编译期元组；编译期索引必须在访问被 elaboration 和类型检查之前成为已知常量。

访问越界是编译错误，不产生运行时检查或可恢复的运行时结果。

## 4. `comptime for` 逐项展开异构元组

异构元组可以使用 `comptime for` 静态遍历：

```ink
comptime for (const index in 0 .. specification.length) {
    const element = specification[index];
    // 每轮的 index 和 element 都是编译期已知值。
}
```

编译器概念上把循环展开为若干独立迭代：

```text
iteration 0: element : type
iteration 1: element : ptrsize
iteration 2: element : GenericTypeDecl
iteration 3: element : Identifier
```

循环体不是先在一个虚构的公共元素类型下检查一次。每一轮都使用该位置的准确类型重新进行依赖名称绑定、类型检查和 Partial Evaluation，然后把仍依赖运行时值的部分残留为 InkIR。

普通运行时 `for (const element in specification)` 非法，因为单个运行时循环变量不能在不同迭代中改变静态类型。

## 5. 循环体必须对实际每一项合法

逐项 elaboration 不等于 SFINAE。循环体会执行的每个实例都必须合法：

- 某轮调用了该元素类型不存在的操作，整个编译期循环失败；
- `comptime if` 可以根据当前元素的类型或反射信息选择合法分支；
- 未选择的编译期分支不生成运行时代码；
- 失败迭代不能被静默丢弃；
- 循环变量的迭代相关类型不能逃逸到循环外成为一个未知运行时类型。

如果需要收集不同类型的结果，应构造另一个结构化元组、请求闭合声明或使用其他编译期已知结构，而不是要求运行时数组容纳它们。

## 6. 驱动静态声明展开

编译期元组可以保存若干类型值，再由 `comptime for` 将同一个静态声明按不同类型展开：

```ink
const supported_types = comptime (i64, String, bool);

return class {
    comptime for (const Element in supported_types) {
        func encode(value: Element) -> String {
            return encode_value(value);
        }
    }
};
```

每轮的 `Element` 是值为 `i64`、`String` 或 `bool` 的一等编译期 `type` 值。声明名称 `encode` 是源码中的真实 Identifier；循环只产生参数类型不同的普通重载。

展开声明的身份、固定点提交、访问权限和资源预算继续使用议题 61、63、67 的规则。元组只组织输入数据，不能把字符串或 `Identifier` 元值变成动态声明名称。

## 7. 编译期函数参数与返回值

编译期可执行函数可以接收和返回准确的元组类型，包括含有元类型的元组：

```ink
func primary_layout() -> (type, ptrsize) {
    return (i32, cast::<ptrsize>(4));
}
```

含编译期专用元素的签名不能形成运行时调用 ABI；调用必须在固定点编译期执行中被消去。只含普通可表示元素的元组仍可按照 Partial Evaluation 规则完整求值或残留。

元组也可以作为一个完整编译期实参传入准确声明为该元组类型的泛型形参。它是一个实参，而不是隐式拆成多个泛型实参；议题 64 的无推导规则保持不变。

## 8. 与编译期参数包和序列的关系

议题 62 的参数包仍是具有单一元素类型的不可变编译期序列：

```text
Types      : type[] known in the generic context
Dimensions : ptrsize[] known in the generic context
```

它适合数量可变但元素元类型相同的情况。编译期元组适合长度和每个位置类型已经结构化确定的异构情况；两者不是同一个容器，也不互相隐式转换。本议题不增加二者之间的新展开或转换语法。

## 9. 不提供动态增删和公共元素类型

异构元组的长度、顺序和每项类型属于其结构类型身份。Ink 不提供在不知道结果类型的情况下运行时追加、删除、排序或筛选元素的操作。

编译期代码可以通过构造新的元组完成静态变换，但结果元组的完整类型必须在 Partial Evaluation 后确定。需要同类型动态数量数据时继续使用普通编译期序列；需要运行时动态集合时使用数组、切片或标准库容器。

Ink 不自动计算“所有元类型的共同基类”，也不因控制流合并而产生 `ComptimeAny`。两个不同结构的元组值若要汇合到同一个表达式，必须具有同一个准确元组类型，或由编译期条件在实例闭合时消除未选择分支。

## 10. 复制、生命周期和效果

编译期解释器逐元素执行普通语义：

- 纯编译期值按其语义值保存，不要求宿主内存布局等同于目标布局；
- 普通可复制值沿用元素复制规则；
- 编译期资源和宿主句柄不能通过元组逃逸到残留程序；
- 元组不会隐藏文件、网络或其他外部效果；
- 构造元组时的编译期效果按照元素表达式从左到右执行。

如果一个完全由普通值组成的已知元组被残留，目标布局、构造与析构顺序继续由议题 69 决定。含有编译期专用值的语义元组不需要 LLVM 数据布局。

## 11. 反射、规范化与缓存

编译期反射可以观察元组的有序元素类型，并在元组值已知时逐项取得编译期值。元组作为泛型实参或静态声明展开输入时，规范化身份至少包含：

```text
ordered element types
+ canonical element values
+ target/dependency information required by each element
```

元素顺序不同、元素类型不同或任一规范化值不同，都表示不同的编译期实参。相同规范化元组值可以复用同一闭合实例缓存。

动态反射不接收这些编译期元组以触发运行时实例化。只有已经生成并登记的普通运行时元组类型和值可以进入动态反射；`type`、声明句柄和其他元值仍然不能运行时逃逸。

## 12. 实现边界与诊断

解释器可以把编译期元组实现为带有逐项语义类型和值的内部 aggregate，不要求分配符合目标 ABI 的连续内存。`comptime for` 在 Staged InkIR 固定点中展开，并把每轮循环体交给正常名称绑定、类型检查和 IR verifier。

编译器至少应诊断：

- 异构元组被普通运行时索引或迭代；
- 编译期常量索引越界；
- 含元值的元组试图残留到运行时；
- 某次静态迭代的循环体不合法；
- 编译期专用元组出现在 `extern "C"` 或动态反射 ABI；
- 元组变换不能得到一个确定的闭合结果类型；
- 展开或生成超过编译期资源预算。

LLVM 不理解编译期异构元组。固定点结束后，它只能看到已经消去的元值操作，或者只含普通可表示元素并按照议题 69 lowering 的闭合运行时元组。

## 13. 后续问题

以下内容留给独立议题：

- 编译期元组拼接、筛选和映射的标准库接口；
- 更完整的元组模式与 `comptime for` 解构语法；
- 大型编译期聚合值的缓存与增量编译编码；
- 编译期诊断中异构元组值的格式化上限。
