# 议题 19：编译期反射、动态反射与自定义元数据

> 状态：已确认，议题 20—26、28、31—35、37、57、61、63、66 与 Parser 议题 15 补充
> 确认日期：2026-08-01

## 1. 两级反射模型

Ink 把反射分成两种能力：

1. 编译期反射可以访问全部结构信息，不要求源码添加反射标记；
2. 动态反射只为显式使用 `[reflect(...)]` 的类型、字段和函数生成运行时信息。

编译期的完整可见性不意味着程序必须携带完整运行时描述表。没有生成动态反射信息的声明，其编译期元数据可以在编译结束后完全消失。

## 2. `reflect` 是内建属性

`reflect` 不采用独立前缀或宏语法，而是内建属性系统中的可变参数属性：

```ink
[reflect]
class EmptyMetadataType {
    // ...
}

[reflect(
    display_name = DisplayName("Player"),
    serializable = Serializable()
)]
class Player {
    // ...
}
```

`[reflect]` 与 `[reflect()]` 含义相同。它们都要求编译器为目标声明生成动态反射描述符；括号内的零个或多个位置或命名实参是附着到该声明的用户元数据值。命名形式使用统一调用语法 `identifier = expression`，例如 `display_name = DisplayName("Player")`；Parser 建立普通 `NamedArgument` 节点，不为反射属性建立专用赋值表达式。

`reflect` 是语言内建属性名，用户不能重新定义其编译器语义。用户可以自由定义传入它的元数据类型。

## 3. 编译期完整反射

在编译期上下文中，`reflect(declaration)` 可以查看声明的完整结构，而不要求目标带有 `[reflect]`：

```ink
comptime {
    const type = reflect(Player);

    print(type.name);
    print(type.size);
    print(type.alignment);
    print(type.visibility);

    for (const field in type.fields) {
        print(field.name);
        print(field.type);
        print(field.offset);
        print(field.attributes);
        print(field.metadata);
    }

    for (const function in type.functions) {
        print(function.name);
        print(function.parameters);
        print(function.return_type);
        print(function.calling_convention);
    }
}
```

编译期反射至少包括：

- 类型种类、名称、可见性和泛型参数；
- 大小、对齐、字段偏移和其他布局信息；
- 字段、函数、构造函数，以及议题 32 定义的枚举分支、载荷和表示信息；
- 参数、返回类型和调用约定；
- 全部内建属性；
- 已附着的用户反射元数据。

议题 66 进一步区分开放 `GenericTypeDecl`、`GenericFunctionDecl` 与闭合 `type`、`FunctionDecl`。开放声明反射可以查询有序泛型参数、参数值类型、参数包和默认值存在性，但没有对象大小，也不能进入运行时动态反射值。

编译期结构反射不自动暴露函数体 AST、局部变量、注释或任意 token stream。议题 63 进一步规定 Ink v0 不提供字符串到代码重解析、token 粘贴、源码 quote/splice 或局部 `eval`；这些能力不能从完整结构反射中推导出来。

议题 61 允许普通编译期代码把反射结果重新用于受控的类型、字段、函数和 InkIR 构造。生成结果仍须重新经过名称绑定、访问检查、类型检查、布局依赖检查和 IR verifier；能够查询结构不等于获得可任意破坏编译器内部 IR 的写权限。

反射可以检查私有声明的结构和可见性，但“能够看到元数据”不自动授予读取或修改私有值的能力。具体规则由 [`20-reflection-access-control.md`](./20-reflection-access-control.md) 规定。

## 4. 显式动态反射

类型、字段和函数分别选择是否生成动态反射信息：

```ink
[reflect(display_name = DisplayName("Player"))]
class Player {
    [reflect(
        display_name = DisplayName("Health"),
        range = Range(min = 0, max = 100),
        save_game = SaveGame()
    )]
    var health: i32;

    [reflect(
        display_name = DisplayName("Take Damage"),
        category = Category("Combat")
    )]
    func take_damage(amount: i32) -> bool {
        // ...
    }

    var secret: i32;
}
```

该声明产生：

- `Player` 的运行时 `TypeInfo`；
- `health` 的 `PropertyInfo` 和动态访问适配器；
- `take_damage` 的 `FunctionInfo` 和动态调用适配器；
- 对应声明上的强类型元数据；
- 不包含 `secret` 的动态成员信息。

成员使用 `[reflect]` 时，其所属类型也必须使用 `[reflect]`，否则是编译错误。顶层函数可以独立使用 `[reflect]`。

普通运行时代码对未标记声明请求动态描述符时必须产生编译错误，而不是静默生成隐藏的运行时元数据。编译期 `reflect(...)` 不受此限制。

## 5. 用户定义元数据类型

用户通过内建 `[metadata]` 属性声明可以附着到 `[reflect(...)]` 的强类型元数据：

```ink
[metadata]
class DisplayName {
    var value: string;
}

[metadata]
class Range {
    var min: i64;
    var max: i64;
}

[metadata]
class Category {
    var name: string;
}

[metadata]
class SaveGame {}
```

`[metadata]` 自身是语言内建属性；`DisplayName`、`Range` 等是普通用户类型。这不允许用户发明新的编译器属性，也不会让编译器把这些类型解释为布局、优化或类型检查指令。

`[reflect(...)]` 的每个位置或命名实参值必须是在编译期完成构造并能静态编码的 `[metadata]` 值。命名实参的名称与值一起保留在结构化元数据中；重复名称和同一声明不允许的名称由 `reflect` 属性语义检查。允许的值组成部分包括：

- 布尔、整数、浮点数和静态字符串；
- 枚举值；
- 类型、函数和其他声明的完全限定名称；
- 其他元数据值；
- 上述值构成的定长数组。

元数据构造上下文要求这些字段值在编译期已知；字段类型仍写作普通 `string`。静态字符串在动态描述表中编码为指向模块静态只读数据的字符串视图，不要求引入核心语言内建的拥有型字符串。

元数据不能包含：

- 指向运行时对象的裸指针或引用；
- 文件、线程、设备和其他运行时句柄；
- 需要运行时析构的对象；
- 依赖特定加载地址才有意义的数据；
- 循环对象图。

同一个 `[reflect(...)]` 列表默认不能出现两个相同元数据类型的值；需要表达多个同类项目时，应在一个元数据值中使用定长数组。这样 `metadata.get[T]()` 始终没有歧义。

## 6. 动态反射查询

带有 `[reflect]` 的类型进入运行时反射注册表，并按照议题 22 使用完全限定名称查找：

```ink
if (match .some(type) = reflection.find_type("game.Player")) {
    print(type.name);
    print(type.size);
    print(type.alignment);

    for (const property in type.properties) {
        print(property.name);
        print(property.type_name);
    }
}
```

自定义元数据按其类型查询：

```ink
if (match .some(health) = type.property("health")) {
    if (match .some(range) = health.metadata.get[Range]()) {
        print(range.min);
        print(range.max);
    }

    if (health.metadata.has[SaveGame]()) {
        save_property(health);
    }
}
```

强类型查询由编译器取得元数据类型的完全限定名称，并在运行时按照该名称匹配。运行时仍可枚举未知元数据，并通过其通用描述符供编辑器、调试器和插件显示。

## 7. 动态字段访问

被反射字段可以通过编译器生成的适配器进行类型检查后的动态访问：

```ink
if (match .some(property) = type.property("health")) {
    const health = property.get[i32](&player);
    property.set[i32](&player, 80);
}
```

运行时必须检查：

- 接收对象与字段所属类型兼容；
- 请求的字段类型正确；
- 写入操作满足字段可变性；
- 对象和描述符仍属于有效模块版本。

不匹配必须抛出明确的反射异常，不能因为反射适配器中的错误类型、错误偏移或错误对象产生 UB。异常按照议题 34 自动传播。对不可复制字段取得值、借用字段以及字段访问控制的具体 API 留给后续议题细化。

字段按值读取、借用、写入以及内部类型擦除表示由 [`21-dynamic-reflection-value-abi.md`](./21-dynamic-reflection-value-abi.md) 规定。

## 8. 动态函数调用

被反射函数具有编译器生成的类型擦除调用适配器：

```ink
if (match .some(function) = type.function("take_damage")) {
    const damaged = function.call[bool](&player, 10);
}
```

适配器必须检查接收对象、参数数量、参数类型、返回类型和调用约定。反射调用失败抛出明确异常，不以类型不匹配作为 UB。

适配器可以使用调用者提供的栈上动态值数组，不要求为每次调用进行堆分配。不可复制参数和引用参数的适配遵守议题 21；异步函数使用议题 57 的独立 `call_async[R]` 和 `DynamicTaskOut`。可变参数、生成器以及尚未确定的特殊调用边界留给后续议题。

普通同步函数的动态参数与返回值 ABI 由 [`21-dynamic-reflection-value-abi.md`](./21-dynamic-reflection-value-abi.md) 规定。

反射虚函数的描述符选择、元数据来源和最终虚派发规则由 [`25-virtual-functions-reflection.md`](./25-virtual-functions-reflection.md) 规定。

反射接口、接口方法描述符、实现关系与接口表调用规则由 [`28-interface-reflection.md`](./28-interface-reflection.md) 规定。

## 9. 对象布局

`[reflect]` 默认只生成声明级描述符、成员适配器和注册表项，不给每个对象添加隐藏类型指针或对象头。

因此不能从裸 `void*`、无类型内存地址或已经丢失类型信息的指针中恢复动态类型。需要携带动态类型的接口对象、类型擦除容器或未来的反射对象基类，必须在自身 ABI 中显式保存完全限定类型名称或进程内描述符句柄。

## 10. 热更新

动态反射注册使用议题 17 的事务式模块注册：

- 描述符和成员适配器属于具体模块版本；
- 完全限定名称标识类型、字段和函数；
- 函数适配器通过热更新稳定入口调用目标函数；
- 新描述符、函数实现槽和相关注册项在同一事务中发布；
- 卸载旧版本不会删除新版本的同一逻辑描述符。

运行时反射句柄不能是会在模块卸载后悬空的无所有权裸指针。实现可以使用完全限定名称重新解析当前版本，或者让描述符借用在其有效期内固定对应模块版本；具体 ABI 留给反射运行时实现议题确定。

## 11. 成本模型与裁剪

编译期反射不会迫使程序保留运行时元数据。

每个 `[reflect]` 声明可能产生以下运行时成本：

- 完全限定名称、类型和布局描述；
- 用户元数据的静态编码；
- 字段访问或函数调用适配器；
- 模块加载时的事务式注册项；
- 动态查询和动态调用本身的检查成本。

没有 `[reflect]` 的声明不生成上述完整动态反射信息。议题 31 为 `try_cast`、议题 35 为异常匹配分别定义的最小类型描述符不包含成员列表、用户元数据或动态调用适配器，不属于这里的完整反射表。链接器可以删除不可达模块版本的全部反射表。`[reflect]` 不改变普通直接字段访问和普通直接函数调用的性能。

议题 37 的 `ExceptionView.reflection()` 只在动态异常类具有 `[reflect]` 信息时返回与该异常记录模块版本相匹配的 `TypeInfo`；否则返回 `none`。它不能仅按名称查找并把旧异常载荷交给不兼容的新版本反射描述符。

## 12. 与内建属性和装饰器的关系

- `[reflect(...)]` 和 `[metadata]` 是内建属性；
- 用户元数据是数据，不是编译器语义属性；
- 编译期反射可以查看所有内建属性和用户元数据；
- 动态反射只能查看已经生成到动态描述符中的信息；
- 函数装饰器不是属性，默认不作为用户元数据出现在反射表中；
- 装饰器可以显式生成反射注册项或把元数据附着到其目标，但生成结果仍须通过正常的类型检查和反射规则。

## 13. 后续问题

以下内容留给独立议题：

- 反射描述符、句柄和动态值容器的精确二进制布局；
- 安全动态类型查询由议题 31 规定；完整反射描述符的精确二进制布局仍待定；
- 生成器、异步生成器和可变参数函数的动态调用；
- 元数据类型的版本兼容和插件边界。
