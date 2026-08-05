# 议题 72：不设置 `comptime func` 函数类别

> 状态：已确认，2026-08-05 补充 `comptime` 只作表达式与控制结构前缀；Parser 议题 31 不提供 `comptime func` 修饰符，Parser 议题 32 统一区域控制
> 确认日期：2026-08-02

## 1. 普通函数可以在不同阶段执行

Ink 不把函数永久分类为“编译期函数”和“运行时函数”。同一个普通函数可以根据调用上下文由编译期解释器执行，也可以残留为运行时 InkIR：

```ink
func fibonacci(n: u32) -> u64 {
    // ...
}

const known = comptime fibonacci(20);
const dynamic = fibonacci(input);
```

第一个调用必须在编译期完成，第二个调用在运行时执行。两者引用同一个函数声明，不需要生成两个源码函数种类。

## 2. 不提供 `comptime func`

下列声明形式不是 Ink 语法：

```ink
comptime func helper() {
    // ...
}
```

需要在编译期调用辅助函数时，声明普通函数并在调用位置要求 `comptime`：

```ink
func helper() -> i32 {
    return 42;
}

const value = comptime helper();
```

编译期执行能力由 InkIR 操作、实参阶段和执行世界决定，而不是由函数声明上的类别修饰符决定。

## 3. `comptime` 的合法职责

`comptime` 是直接修饰表达式或结构化控制流的前缀，不是类型构造器、参数限定符或声明类别。直接形式限定为：

```text
comptime expression
comptime { ... }
comptime if (condition) { ... }
comptime match (value) { ... }
comptime for (binding_mode element in values) { ... }
comptime while (condition) { ... }
```

函数调用、构造调用和成员调用本来就是表达式，因此统一写成：

```ink
const value = comptime helper();
const object = comptime Object(arguments);
```

不增加 `comptime call` 或其他专用调用语法。赋值、局部绑定、`try`、`defer`、`return`、`break`、`continue` 和 `throw` 等非表达式语句在需要编译期执行时放入 `comptime { ... }`，并继承外层执行阶段；不增加 `comptime var`、`comptime return` 等逐项前缀。

泛型尖括号本身就是编译期参数域，所以泛型形参只写普通值类型：

```ink
func create<
    T: type,
    Count: ptrsize
>() {
    // ...
}
```

这里的 `T` 和 `Count` 因位于 generic parameter list 而必须在声明闭合期间已知，不需要也不允许 `T: comptime type`。attribute/decorator application 的实参以及 `[metadata]` 值构造同理由其上下文要求编译期求值，不在参数或字段类型前重复书写 `comptime`。

这不会把 `create` 变成特殊函数类型，也不参与运行时函数签名。

同一组 `comptime {}`、`if`、`match`、`for`、`while` 表面形式也可以位于 module 或类型成员声明区。Parser 议题 32 使用同一个 `ComptimeRegionControl`，由外层提供 `StatementRegion`、`TopLevelRegion`、`ClassMemberRegion`、`InterfaceMemberRegion` 或 `EnumMemberRegion`；对应 `{ ... }` 直接解析成该区域的 block，body 只接受当前区域允许的 items。区域种类只决定 item parser 和输出 sink，执行阶段仍由同一 Partial Evaluation 模型决定，不为声明区另设第二个 `comptime` 关键字或宏系统。

## 4. 含元类型的签名自然只能在编译期使用

普通函数可以接收或返回 `type`、`GenericDecl`、`FunctionDecl` 以及包含这些值的元组：

```ink
func primary_field() -> (Identifier, type) {
    return (Identifier.from("value"), i32);
}
```

由于返回值不能形成运行时表示，该调用必须在 Partial Evaluation 中被完全消去。编译器根据签名和值的可残留性得出这个结论，不需要 `comptime func` 标记。

试图从普通运行时路径调用该函数并保存完整结果是编译错误。若只在编译期投影并残留其中可运行时表示的部分，则使用议题 61、71 的普通残留规则。

## 5. 效果能力在实际操作处检查

函数是否能在编译期成功执行，取决于必经路径上的操作是否有对应编译期处理器和权限：

```ink
func load_schema(path: StringView) -> String {
    return stdio.read_file(path);
}

const schema = comptime load_schema("schema.json");
```

编译期解释器进入普通 `load_schema`，直到 `stdio.read_file` lowering 的 `fs.read` 操作。若 `ComptimeWorld` 提供并授权文件处理器，则执行并记录构建依赖；否则在该操作处报告编译期执行错误。

一个函数在某个编译期调用中成功，不保证它对所有实参、分支、目标和效果环境都能编译期执行。编译器不能仅按函数名称缓存一个永久的“可编译期”布尔属性。

## 6. Partial Evaluation

普通函数调用继续使用统一执行器的值状态：

```text
Known inputs + executable operations
→ compile-time result

Known and Runtime inputs
→ Partial Evaluation
→ residual closed InkIR
```

显式 `comptime` 上下文不允许最终结果依赖 `Runtime` 值。普通上下文可以执行已知纯计算并残留未知计算，但不能擅自把运行时效果移动到编译期。

递归、循环、普通辅助调用、泛型实例和反射调用遵守同一规则，并继续受编译期资源预算与固定点收敛检查约束。

`comptime if`、`comptime match` 和 `comptime for` 可以在选择或展开后残留所选 body 中依赖运行时输入的普通 InkIR；它们只要求控制条件、被匹配值或迭代源在编译期已知。`comptime while` 要求每轮条件都能在编译期决定，并必须受迭代次数、生成代码量和总执行 fuel 限制。

## 7. 函数类型、重载与 ABI

不存在 `comptime func`，因此函数类型、重载签名、虚函数槽、接口槽、函数指针和 C ABI 都不需要编码“编译期函数”位。

包含编译期专用参数或返回值的函数声明不能形成运行时 ABI，也不能跨 `extern "C"` 边界；这属于签名不可运行时表示，而不是另一种调用约定。

只使用普通可表示类型的函数，无论是否曾被编译期调用，都保持相同运行时函数类型和 ABI。

## 8. 诊断

编译器至少应区分：

- 在函数声明位置错误使用 `comptime func`；
- 在类型或参数位置错误使用 `comptime`，例如 `T: comptime type`；
- 强制编译期调用读取运行时未知值；
- 强制编译期调用遇到没有处理器或权限的效果；
- 含编译期专用值的结果试图残留到运行时；
- 编译期执行超过资源预算或不能收敛；
- 含元类型的函数签名试图进入函数指针、动态反射或外部 ABI。

诊断应指向真正阻止编译期执行的操作和调用链，而不是笼统声称整个函数“不是 constexpr”。

## 9. 实现边界

编译器可以内部分析某个函数在特定实参和效果世界下是否可执行，并缓存相应 Partial Evaluation 结果。该分析结论不是源码函数类别，也不能改变声明身份。

LLVM 只接收仍需运行时存在的闭合函数和残留控制流。已经完全编译期执行的调用及其元值不会进入 LLVM IR。
