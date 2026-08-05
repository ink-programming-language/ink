# 议题 65：普通参数与编译期参数统一使用默认实参

> 状态：已确认，议题 70 补充默认实参与重载候选关系，Parser 议题 13、15、29、31 补充求值顺序、命名实参、函数类型与默认形参语法
> 确认日期：2026-08-02

## 1. 统一 `=` 语法

普通函数参数使用以下默认值语法：

```ink
func connect(
    host: StringView,
    port: u16 = 443,
    timeout: Duration = Duration.seconds(30)
) {
    // ...
}
```

编译期参数使用完全相同的 `=`：

```ink
func parse<
    Base: u32 = 10
>(text: StringView) -> i64 {
    // ...
}

class Buffer<
    T: type,
    Alignment: ptrsize = reflect(T).alignment
> {
    // ...
}
```

统一形式为：

```text
runtime parameter:  name: Type = expression
generic parameter:  Name: Type = expression
```

generic parameter list `<...>` 本身要求其中所有实参在声明闭合期间已知，因此不在形参类型前重复书写 `comptime`。`Name: comptime Type` 不属于 Ink 语法。

默认值不构成推导。它只在调用点省略具有声明默认值的参数时补全实参；位置调用只能省略连续尾部参数，命名实参则可以显式绑定较后的参数并跳过中间已有默认值的参数。

## 2. 位置调用省略尾部，命名实参可以跳过默认项

具有默认值的参数必须位于所有无默认值的同类参数之后：

```ink
func valid(
    required: i32,
    first: i32 = 1,
    second: i32 = 2
);

func invalid(
    first: i32 = 1,
    required: i32, // 编译错误
);
```

调用者只能从末尾连续省略：

```ink
connect(host);
connect(host, 8080);
connect(host, 8080, custom_timeout);
```

普通调用、构造调用、attribute application 和 decorator application 统一支持 `identifier = expression` 命名实参。全部位置实参和 `...` 展开必须位于命名实参之前；命名实参可以跳过具有默认值的较早参数：

```ink
connect(host, timeout = custom_timeout);
connect(timeout = custom_timeout, host = host);
```

第二个调用仍按源码顺序先求值 `custom_timeout`，再求值 `host`；名称只改变形参绑定，不改变显式实参的求值顺序。未知名称、重复名称、位置实参与命名实参重复绑定同一形参，以及遗漏没有默认值的形参，均为编译错误。

Ink v0 仍不提供命名泛型实参或“跳过当前位置”的匿名占位符。泛型 `<...>` 的默认值和参数包继续使用第 6、7 节的位置绑定规则。

## 3. 普通默认表达式在每次调用时求值

普通函数参数的默认表达式不是在声明时求值一次，而是在每次实际省略该参数的调用发生时求值：

```ink
func log(
    message: StringView,
    timestamp: Timestamp = clock.now()
) {
    // ...
}
```

每次 `log(message)` 都重新调用 `clock.now()`。显式提供 `timestamp` 时不执行默认表达式。

默认表达式可以产生普通运行时副作用或抛出未检查异常。副作用发生在进入目标函数之前；默认表达式抛出时，目标函数、装饰器和函数体都没有开始执行。

## 4. 可以引用之前已经绑定的参数

默认表达式可以引用同一参数列表中位于它之前的参数：

```ink
func read(
    buffer: byte[],
    count: ptrsize = buffer.length
) {
    // ...
}
```

调用：

```ink
read(make_buffer());
```

概念求值顺序为：

```text
evaluate make_buffer() once
→ bind buffer
→ evaluate buffer.length
→ bind count
→ enter read(buffer, count)
```

默认表达式不能引用自身或位于它之后的参数。按照 [`../parser/13-expression-evaluation-order.md`](../parser/13-expression-evaluation-order.md) 的统一规则，调用先从左到右计算所有显式位置和命名实参，再按照形参声明顺序从左到右计算仍未绑定参数的默认表达式。前面的参数绑定在后续默认表达式执行前已经稳定，且不会为了计算默认值重复求值显式实参。

## 5. 名称解析和访问权限

默认表达式在函数声明的词法上下文中解析和进行访问检查，而不是捕获调用位置的同名局部变量：

```ink
const default_port: u16 = comptime 443;

func connect(
    host: StringView,
    port: u16 = default_port
);
```

调用者作用域中另一个名为 `default_port` 的变量不影响该默认值。默认表达式可以使用声明模块正常可访问的私有辅助声明，但不会把这些私有声明暴露给调用者。

非泛型默认表达式中不依赖参数的部分在声明处完成普通类型检查；依赖前置参数或编译期参数的操作在相应闭合调用或实例中继续检查。最终产生的值必须按普通参数绑定规则兼容目标参数类型。

## 6. 编译期参数默认值

编译期默认表达式在泛型实参绑定阶段执行：

```ink
parse<>(text);   // Base = 10
parse<16>(text); // Base = 16

Buffer<i32>;     // Alignment = reflect(i32).alignment
Buffer<i32, 16>; // Alignment = 16
```

即使所有编译期参数都有默认值，调用开放泛型函数仍须写出 `<>`，明确请求使用默认编译期实参。议题 64 的无推导规则继续有效：

```ink
parse(text); // 编译错误：没有显式请求开放泛型实例
```

绑定顺序为：

```text
bind explicit positional generic arguments
→ evaluate omitted trailing defaults from left to right
→ canonicalize all comptime values
→ request closed instance
→ Partial Evaluation
```

默认表达式可以引用之前已经绑定的编译期参数、普通编译期函数、完整反射和 `TargetContext`。文件、环境或其他外部操作继续遵守议题 61 的能力、处理器和依赖追踪规则；显式提供该参数时不执行它的默认表达式。

## 7. 与参数包组合

议题 62 的参数包自身不能声明默认值，空包已经表示零个元素。参数包之前可以存在具有默认值的普通编译期参数：

```ink
class Values<
    Element: type = i32,
    Rest: type...
> {
    // ...
}
```

位置绑定优先填充固定参数：

```text
Values<>           → Element = i32, Rest = []
Values<byte>       → Element = byte, Rest = []
Values<byte, i32>  → Element = byte, Rest = [i32]
```

因为 v0 没有命名泛型实参或泛型占位符，所以不能在使用 `Element` 默认值的同时为 `Rest` 提供非空位置实参。普通调用已经支持的命名实参不进入泛型 `<...>`。需要这种泛型接口时应重新排列 API 或显式写出默认值。

## 8. 默认值不属于函数签名或 ABI

函数的完整参数列表构成实际调用签名；默认值只是源码直接调用的实参补全规则：

```ink
func output(value: i32, radix: u32 = 10);
```

它仍然只有一个二参数函数，不会隐式生成一参数重载。两个完整签名相同、只有默认表达式不同的声明属于重定义。

议题 70 规定，普通默认实参只影响闭合候选能否接收当前实参数量，不进入签名或产生额外候选；编译期默认实参只有在调用者显式写出 `<>` 选择泛型候选后才参与绑定。

函数指针、裸稳定入口和其他只携带函数类型的调用必须提供全部参数：

```ink
const entry: func(i32, u32) = &output;
entry(10, 10); // 必须提供 radix
```

Parser 议题 29 规定函数类型参数只写类型，不写名称或默认表达式；省略 `-> type` 等价于返回 `void`。因此上面的 `func(i32, u32)` 只记录实际二参数调用形状，不记录 `radix` 的默认值。

同理，仅通过普通函数值或函数指针调用时没有可用于 `NamedArgument` 绑定的参数名称，必须使用位置实参并提供完整参数列表。命名实参只适用于能够静态确定声明参数名称的普通源码调用；参数名称是否属于公开源码调用契约由相应 API 和模块规则维护，但不进入函数类型或底层调用 ABI。

默认值不编码进普通调用约定。`extern "C"` 声明即使供 Ink 源码直接调用时带有默认值，C ABI 消费者也看不到该默认值；它不是二进制接口的一部分。

## 9. 虚函数和接口方法

虚函数或接口方法的默认值属于最初声明该槽的调用契约：

```ink
class Base {
    virtual func draw(scale: f32 = 1.0);
}

class Derived : Base {
    override func draw(scale: f32); // 不能重新声明或修改默认值
}
```

覆盖声明不能添加、删除、重写或重新声明继承槽的默认值。通过 `Base`、`Derived` 或继承该槽的其他静态视图调用时，编译器都使用根槽声明的同一默认表达式，再进行正常动态分派。

接口实现和接口默认方法覆盖遵循相同规则。彼此无继承关系的多个接口槽仍是独立调用契约；它们的默认参数冲突以及一个类方法能否同时满足这些槽，按照后续完整接口签名冲突规则处理，不能由实现方法静默选择其中一个默认值。

## 10. 动态反射调用不应用默认值

议题 21 和 57 的同步、异步动态反射调用必须提供描述符完整签名要求的全部运行时实参：

```text
FunctionInfo.call       requires every parameter
FunctionInfo.call_async requires every parameter
```

`FunctionInfo` 不需要为运行时调用保留可执行默认表达式，也不根据缺少的 `DynamicRef` 数量自动运行源码默认值。调用方希望省略参数时，应在普通源码中调用一个包装函数，由包装函数使用直接调用语义应用默认值。

编译期完整反射可以观察参数是否声明默认值及其结构化语义信息；动态 `[reflect]` 描述符不因此获得运行时默认参数 thunk。

## 11. 装饰器与异步调用

默认实参在进入函数稳定入口和装饰器链之前已经补全。同步装饰器总是看到完整参数列表，默认表达式抛出时不进入任何装饰器。

异步函数调用同样先求值全部显式和默认实参，再调用议题 44—56 的任务构造入口：

```text
evaluate explicit arguments
→ evaluate omitted defaults
→ construct lazy Task
→ later drive coroutine body
```

因此默认表达式的副作用和异常发生在任务创建之前，不属于惰性任务体，也不会写入任务的 `ExceptionBox`。默认表达式失败时任务尚未成功构造。

## 12. 热更新与源码分发

默认值在调用点补全，不通过目标函数稳定入口动态查询。改变默认表达式会影响重新编译后的省略调用，但不会改写已经编译到旧调用方中的实参生成代码。

这属于源码调用契约变更，不是函数完整参数 ABI 变更。议题 61 的源码分发规则保证可复用 Ink 调用方能够在更新后重新编译；热更新系统不能只替换被调用函数体就声称所有旧调用点已经采用新默认值。

显式写出全部实参的调用不受默认表达式变化影响。

## 13. 诊断

编译器必须诊断：

- 无默认参数出现在有默认参数之后；
- 位置调用省略非尾部参数；
- 未知或重复的命名实参；
- 位置与命名实参重复绑定同一形参；
- 命名实参后继续出现位置实参或列表展开；
- 省略没有默认值的必需参数；
- 默认表达式引用自身或后续参数；
- 默认表达式结果不能绑定目标类型；
- 覆盖声明试图添加或改变默认值；
- 函数指针或动态反射调用缺少完整实参；
- 省略泛型默认实参却没有写出 `<>`；
- 参数包绑定与默认固定参数不符合位置规则。

## 14. 后续问题

以下内容留给独立议题：

- 多个独立接口槽具有不同默认值时的实现冲突；
- 跨模块 API 参数重命名的源码兼容性分类；
- 未来是否让动态反射显式查询可静态编码的常量默认值；
- 默认值变化在包版本工具中的兼容性分类。
