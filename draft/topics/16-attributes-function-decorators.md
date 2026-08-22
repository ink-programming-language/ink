# 议题 16：内建属性与函数装饰器

> 状态：已确认，decorator 是编译期实例化的强类型签名保持包装模板，可跳过或重复调用 continuation；议题 19、58、59、65 与 Parser 议题 15、31、36、37 补充
> 确认日期：2026-08-01

## 1. 属性与装饰器是两种机制

Ink 严格区分属性与装饰器：

```ink
[fast_math, assume_finite]       // 内建属性
@trace(name = "kernel")          // 用户定义装饰器
func kernel() {
    // ...
}
```

- 属性描述声明的编译器语义与元数据；
- 装饰器在编译期生成围绕函数体的代码；
- 同步装饰器生成的代码在同步函数调用中运行；议题 58 的异步装饰器代码在惰性任务被驱动后运行；
- `[...]` 只用于属性；
- `@name(...)` 只用于装饰器。

## 2. 属性是封闭的内建集合

Ink 不允许用户声明自定义属性。所有合法属性都由语言规范、目标规范或工具链规范定义。

```ink
[noncopyable]
class File {
    // ...
}

[transparent]
class Handle {
    var value: ptrsize;
}
```

无法解析的属性名称、错误拼写和错误参数必须产生编译错误，不能静默忽略。

每个内建属性必须分别规定：

- 允许附着的语法目标；
- 参数类型和默认值；
- 是否允许重复；
- 是否影响类型检查、布局、ABI、运行语义、优化或诊断；
- 是否传播到组成类型、嵌套作用域或生成代码；
- 在编译期和运行时反射中的表示。

## 3. 属性列表语法

多个属性写在同一个方括号列表中：

```ink
[fast_math, assume_finite]
func calculate() {
    // ...
}
```

带参数的属性使用调用形式：

```ink
[align(16), section("network")]
class Packet {
    // ...
}
```

属性参数必须是编译器能够在编译期求值和验证的值。

属性应用和装饰器应用统一复用普通函数调用的实参规则：支持位置实参、前置 `...expression` 列表展开和 `identifier = expression` 命名实参，并要求全部位置实参和展开位于命名实参之前。例如：

```ink
[align(16), section(name = "network")]
@trace(name = "calculate", level = 2)
func calculate() {}
```

Parser 议题 15、31 对普通调用、属性和装饰器建立相同的 `PositionalArgument`、`ListExpansion` 与 `NamedArgument` CST 节点。具体内建属性或装饰器是否接受某个名称、是否允许重复绑定，由语义分析检查。attribute/decorator application 本身就是编译期上下文，全部实参必须在该处形成编译期值；装饰器形参仍写作普通 `name: Type`，不增加 `name: comptime Type`。

属性列表是无序集合：

```ink
[fast_math, assume_finite]
[assume_finite, fast_math]
```

两者含义相同。除非某个内建属性明确声明可重复，否则重复出现同一属性是编译错误。

## 4. 属性附着位置

Ink v0 只允许属性列表出现在具有明确 declaration-prefix 语法的声明之前，例如类型声明、函数声明和字段声明。具体属性能否附着某种声明，仍由该属性自己的规范决定。

```ink
[noncopyable]
class Resource {
    [align(16)]
    var storage: byte[16];
}

[fast_math]
func calculate(value: f32) -> f32 {
    return value * 0.5f32;
}
```

Parser 议题 31 要求函数的全部 attribute list 位于所有 decorator application 和函数修饰符组成的声明前缀中；进入参数列表以后不再提供 attribute 附着位置。Parser 议题 33 对类型和字段声明采用同样不可回退的 annotation/modifier 阶段。Ink v0 同样不支持返回位置、语句块或单个表达式 attribute：

```ink
func invalid_parameter([attribute] value: i32); // 语法错误
func invalid_return() -> [attribute] Result;     // 语法错误
[attribute] { work(); }                          // 语法错误
```

这不为未来版本保留一个可被 Parser 静默接受的空语法槽。以后确实需要参数级 `[noalias]`、返回位置或 block attribute 时，必须由独立议题增加准确产生式、CST 和语义规则。

类和接口声明只接受零个或多个 attribute list，不接受函数专用 decorator；全部 attribute list 必须位于访问修饰符和 `final` 等类型修饰符之前。一旦类型声明 Parser 进入 modifier 阶段，就不能在同一声明前缀中再次消费 attribute list。具体 attribute 是否允许附着类或接口，仍由语义分析检查。

字段声明遵守同一前缀边界：零个或多个 attribute list 全部位于可选访问修饰符以及 `var`/`const` 字段核心之前。字段不接受 decorator；进入访问修饰符阶段后也不能再次消费 attribute list。具体 attribute 是否允许附着字段以及是否与字段类型、布局或可变性兼容，由语义分析检查。

属性列表是外层语法目标的元数据，不自动成为 Parser 议题 29 的 `type` 前缀或后缀，也不因为附着在函数声明上就进入普通重载签名或函数值类型。

## 5. 属性元数据与反射

编译器把解析和规范化后的属性保存为声明元数据，而不是保存在每个运行时对象实例中。

编译期反射可以读取声明上的全部属性及其参数。运行时反射系统也必须能够读取被保留声明的属性元数据。

属性元数据具有编译器定义的内建属性种类和结构化参数。属性顺序不进入反射语义。

运行时元数据不应给每个对象增加隐藏字段；它属于类型、函数或模块共享的描述表。编译期完整反射、运行时元数据的显式生成和裁剪规则由 [`19-reflection-runtime-metadata.md`](./19-reflection-runtime-metadata.md) 规定。

## 6. 装饰器的定位

装饰器是用户可定义的、函数专用、签名保持的静态包装模板。它在每个应用点取得目标函数的准确静态签名并完成实例化，不建立 Python 式运行时函数替换、包装对象、堆分配或动态分派。

从类型关系看，一个 decorator 在编译期执行：

```text
Function::<Signature> → Function::<Signature>
```

输入和输出具有同一参数、返回类型、receiver、调用约定和 ABI。Decorator 可以改变调用过程和正常结果值，但不能改变目标函数的公开签名。

装饰器声明中的普通形参是应用时提供的编译期配置参数。`@trace(name = "network")` 的实参在编译期求值；配置值可以作为常量残留到最终包装代码中，不形成运行时 decorator 实例。

议题 65 的默认实参在进入装饰器链之前已经求值并补全。装饰器始终接收完整参数列表；默认表达式的副作用发生在进入任何装饰器之前。装饰器不能改变目标函数的默认参数契约。

```ink
decorator trace(name: string) {
    log("enter", name);

    const result = function(...);

    log("leave", name);
    return result;
}
```

编译器针对具体目标签名实例化并检查 decorator body，方式类似 C++ 函数模板：不依赖目标签名的错误可以在声明处报告，依赖目标参数或结果类型的操作在应用点检查。装饰器体中的 `comptime` 部分在实例化阶段执行，残留的普通 Ink 语句在目标函数调用时执行。

装饰器声明只允许出现在 module 顶层。类和接口成员函数可以应用顶层可见的装饰器，但成员块不能嵌套声明装饰器。

议题 58 使用显式 `async decorator` 装饰异步函数。其生成代码属于目标 coroutine 的执行体，不属于同步任务构造入口；特殊 continuation 写成 `await function(...)`，但降低为同一个状态机内的 region，不创建第二个任务。

议题 59 确认 Ink v0 不提供任务构造装饰器。需要在惰性任务建立前后运行用户代码时，显式编写同步函数返回 `Task::<T>`，并可对该同步任务工厂应用本议题的普通 `decorator`。

装饰器不是：

- 自定义属性；
- Python 式运行时函数重新绑定；
- 运行时 AST 修改；
- 可以任意替换 token stream 的无类型宏。

## 7. 特殊 `function` continuation

装饰器声明的普通形参全部复用函数形参语法。编译器提供的特殊 `function` continuation 隐式存在于装饰器体内，不占用第一个形参位置。

同步装饰器中的 `function(...)` 表示进入下一层同步装饰器或原始函数体。议题 58 的异步装饰器使用 `await function(...)` 进入下一层异步 region；`...` 默认转发被装饰函数的全部当前参数。

`function`：

- 不是普通运行时函数指针；
- 不能保存到变量、字段或全局对象；
- 不能逃逸、返回或被闭包捕获；
- 不能传给普通运行时函数；
- 只能在当前装饰器生成的包装体内调用；
- 在每个应用点取得下一层函数的准确静态签名；
- 调用降低为静态 continuation 进入，不要求运行时函数对象或动态分派；
- 保持被装饰函数的参数、返回类型、调用约定和 ABI。

签名特定的装饰器可以显式传入修改后的参数，或者修改 `function(...)` 返回的结果，但最终公开签名不能改变。

## 8. Continuation 可以调用零次、一次或多次

Decorator 不规定每条正常路径的 continuation 调用次数。它可以：

- 在缓存命中或显式短路时调用零次；
- 对普通 tracing、计时和同步包装调用一次；
- 为重试、候选执行或其他显式控制流调用多次。

每次 continuation 进入都是一次正常的静态函数调用。参数的复制、移动、借用和生命周期继续接受普通类型检查；重复转发已经被消费的不可复制参数并不会因位于 decorator 中而合法。每条正常返回路径也必须按照目标函数的准确返回类型产生结果。

`function` 仍不能保存、逃逸、被闭包捕获、跨线程传递或交给另一个任务。允许改变调用次数不等于把 continuation 变成一等函数值。

异步 decorator 同样可以零次、一次或多次执行 `await function(...)`。每次执行都重新进入下一层异步 continuation；普通所有权、取消和 coroutine 状态检查继续适用。

## 9. 多层装饰器

装饰器按照 Python 的堆叠顺序从内向外应用：

```ink
@outer
@inner
func target() {
    // 原始函数体
}
```

运行顺序为：

```text
outer.before
    inner.before
        original body
    inner.after
outer.after
```

最靠近函数声明的装饰器最先包裹原始函数体。每层的 `function(...)` 指向下一层，不会递归调用当前装饰层。

某一外层 decorator 跳过或重复调用 continuation 时，会相应跳过或重复执行其内侧 decorator 和最终函数体；静态堆叠顺序本身不改变。

## 10. continuation region 降低

编译器不要求为每个装饰层生成真实的运行时包装函数调用。装饰器展开在 Ink HIR/MIR 中形成嵌套 continuation region。

概念上：

```ink
const result = region {
    // 原始函数体
    // 原始 return value 被改写为 region yield value
};
```

原函数体中的 `return` 结束当前 continuation region，并把返回值交给装饰器后置代码，而不是跳过后置代码直接退出整个包装函数。

一次 `function(...)` 调用建立一次下一层 region 进入。Decorator 调用 continuation 多次时，每次都创建独立的动态执行实例，但仍共享编译期确定的代码、签名和 ABI，不产生通用运行时包装对象。

这样可以：

- 避免参数的额外复制；
- 避免装饰层产生闭包或堆分配；
- 避免依赖优化器内联来消除包装调用；
- 保留原函数局部变量、析构和 `defer` 作用域；
- 让装饰器普通后置代码只处理正常返回；
- 让装饰器通过 `defer` 覆盖所有适用的退出路径。

## 11. 热更新

支持热更新的被装饰函数使用稳定入口和可原子替换的实现槽：

```text
stable function entry
        ↓
hot-reload slot
        ↓
outer decorator region
        ↓
inner decorator region
        ↓
original function body
```

热更新版本重新进行装饰器展开和 AOT 编译，然后把实现槽从旧的完整装饰实现原子替换为新的完整装饰实现。

异步目标的稳定入口是完整装饰后任务的构造入口。任务创建时固定具体装饰器展开、帧布局和 resume/destroy 入口；以后驱动旧任务继续执行旧装饰器和旧函数体。

- 新调用进入新版本；
- 已经执行中的旧调用可以完成旧版本；
- 取得函数地址时获得稳定入口；
- 原始函数体地址不能逃逸；
- 源码中的递归函数调用经过稳定入口和装饰器；
- 特殊 `function(...)` continuation 直接进入下一层，避免无限递归。

热更新函数的普通调用不能被永久内联或绕过稳定入口，除非后续热更新协议提供版本检查、去优化或可修补调用点。

装饰器产生的强类型模块注册记录及其热更新切换遵循 [`17-module-lifecycle-decorator-registration.md`](./17-module-lifecycle-decorator-registration.md)。Decorator 不生成用户模块加载或卸载代码。

## 12. 诊断与工具

装饰器生成代码必须重新通过正常的名称解析、类型检查和控制流检查。

编译器必须保留展开来源映射，使生成代码中的诊断能够同时指出：

- 被装饰函数；
- 对应的装饰器应用；
- 装饰器定义中的生成位置。

工具链应能显示装饰后的 HIR/MIR，帮助检查实际插入的运行时代码和装饰器堆叠顺序。

## 13. 后续问题

以下内容留给独立议题：

- 装饰器参数的完整类型兼容与转换规则；
- 装饰器如何访问异构参数包；
- 异步函数 continuation 由议题 58 规定，生成器和 `never` 返回函数仍待定；
- 装饰器生成的静态状态如何参与热更新迁移。
