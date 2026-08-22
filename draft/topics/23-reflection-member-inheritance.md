# 议题 23：反射成员唯一性与继承覆盖

> 状态：已确认，议题 24、25、28—30 补充  
> 确认日期：2026-08-01

## 1. 动态反射函数不允许重载

同一个类型中不能有两个同名的 `[reflect]` 函数，即使它们的参数或返回类型不同。

```ink
[reflect]
class Parser {
    [reflect]
    func parse(value: i32) {
        // ...
    }

    [reflect]
    func parse(value: f32) {
        // 编译错误：Parser 已经具有名为 parse 的反射函数
    }
}
```

顶层函数也遵守同样规则：同一模块反射命名空间中只能公开一个同名反射函数。

限制只作用于动态 `[reflect]` 集合。普通 Ink 函数仍可以按照语言的普通重载规则重载；一个重载集合中至多有一个声明可以标记 `[reflect]`。

```ink
class Parser {
    [reflect]
    func parse(value: i32) {
        // 唯一动态反射入口
    }

    func parse(value: f32) {
        // 可以存在，但不进入动态反射表
    }
}
```

编译期完整反射仍然可以查看全部重载声明，包括没有 `[reflect]` 的声明。本规则只取消动态反射按签名选择重载的需求。

## 2. 动态反射属性名称唯一

同一个类型中不能有两个同名的 `[reflect]` 字段或属性。编译器必须在生成动态描述表之前报告重复名称，并指出全部冲突声明。

字段/属性与函数使用不同的反射查询类别：

```ink
type_info.property("value")
type_info.function("value")
```

因此同一个类型可以具有同名反射属性和反射函数；它们不会在各自的查询表中产生歧义。

## 3. 继承链允许同名覆盖

子类型可以声明与父类型同名的反射函数或反射属性。它们不构成同一类型内的重载或重复声明，而是在继承视图中覆盖父成员。

```ink
[reflect]
class Entity {
    [reflect]
    func display_name() -> StringView {
        // ...
    }

    [reflect]
    var id: i64;
}

[reflect]
class Player : Entity {
    [reflect]
    func display_name() -> StringView {
        // 覆盖 Entity.display_name
    }

    [reflect]
    var id: i64; // 在 Player 反射视图中覆盖 Entity.id
}
```

声明本身必须先满足 Ink 类型系统和继承模型对方法覆盖、字段隐藏、布局及可见性的普通规则。反射机制不会使一个原本非法的继承声明变成合法。

## 4. 从子类向父类查找

在子类型的反射视图上按名称查询时，依次检查：

1. 当前类型直接声明的反射成员；
2. 直接父类型；
3. 更上层父类型；
4. 直到继承链结束。

找到第一个同类别、同名称的反射成员后立即停止。

```ink
const player_type_result = reflection.find_type("game.Player");
if (player_type_result.has_value()) {
    const player_type = player_type_result.value();
    const function_result = player_type.function("display_name");
    if (function_result.has_value()) {
        const function = function_result.value();
        print(function.qualified_name); // Player.display_name
    }

    const property_result = player_type.property("id");
    if (property_result.has_value()) {
        const property = property_result.value();
        print(property.qualified_name); // Player.id
    }
}
```

在父类型自己的反射视图上查询仍得到父成员：

```ink
const entity_type_result = reflection.find_type("game.Entity");
if (entity_type_result.has_value()) {
    const entity_type = entity_type_result.value();
    const function_result = entity_type.function("display_name");
    if (function_result.has_value()) {
        const function = function_result.value();
        print(function.qualified_name); // Entity.display_name
    }

    const property_result = entity_type.property("id");
    if (property_result.has_value()) {
        const property = property_result.value();
        print(property.qualified_name); // Entity.id
    }
}
```

## 5. 有效成员视图与直接声明视图

`type_info.functions` 和 `type_info.properties` 返回覆盖完成后的有效成员集合。同一类别中每个名称最多出现一次，子类型成员取代父类型同名成员。

需要工具查看继承来源时，反射 API 同时提供：

```ink
type_info.declared_functions  // 只包含当前类型直接声明的反射函数
type_info.declared_properties // 只包含当前类型直接声明的反射属性
type_info.base                // 显式取得父类型描述
```

每个有效成员描述符必须保留 `declaring_type`，使编辑器和调试器能够显示该成员实际来自哪个类型。

有效成员集合的顺序不用于名称解析。工具需要稳定展示顺序时，可以按照声明来源和源码顺序排序，但程序语义只依赖名称覆盖关系。

## 6. 元数据不自动合并

子类型覆盖一个反射成员时，查找返回子成员的描述符及其自身元数据。父成员上的用户元数据和属性不会自动合并到子成员。

```ink
[reflect]
class Entity {
    [reflect(Category("Base"))]
    func update() {
        // ...
    }
}

[reflect]
class Player : Entity {
    [reflect(Category("Player"))]
    func update() {
        // 查询 Player.update 只得到 Category("Player")
    }
}
```

需要读取父成员元数据时，工具显式沿 `type_info.base` 查询。以后如果某种内建属性需要继承，必须由该属性自己的规范单独声明，不能由反射系统普遍推断。

## 7. 未标记的子成员

动态反射状态不因普通语言覆盖而自动继承。如果子类型声明了同名函数或属性，但没有 `[reflect]`，它不会作为新的动态反射成员进入子类型描述表；名称查找继续取得最近的已反射父成员。

如果该父函数按照 Ink 的普通语言规则进行虚派发，通过父反射描述符调用它时仍应遵守正常虚派发语义。反射成员查找和函数执行时的虚派发是两个独立步骤，具体见议题 25。

子类型需要替换反射元数据、动态调用签名或属性描述时，必须在自己的覆盖声明上显式添加 `[reflect]`。

## 8. 名称与热更新

反射成员身份仍使用议题 22 的字符串名称：

```text
game.Entity.display_name
game.Player.display_name
```

父子成员具有不同的完全限定名称。子类型查询时选择子成员是继承查找规则，不表示它们共享某个隐藏稳定 ID。

重命名子成员等于删除旧子成员并创建新成员。重命名后，如果父类型仍具有旧名称，子类型按旧名称查询可能重新暴露父成员；工具链应在这种情况下提供诊断提示，但行为仍由正常的子到父查找规则决定。

## 9. 多接口冲突

议题 24 已禁止多个具体父类，因此具体类继承链始终唯一。议题 28 规定不同接口的同名反射方法仍具有各自独立的完全限定描述符，并通过各自接口表调用。普通接口默认实现冲突按照议题 29 处理；接口继承图中的查找、菱形合并和子接口覆盖按照议题 30 处理。编译器不能按照接口声明顺序任意选择语义。

## 10. 诊断

同一类型或同一模块出现两个同名反射函数时，编译器错误必须指出：

- 冲突的反射名称；
- 全部参与冲突的声明；
- 普通重载本身仍可保留；
- 需要取消其中一个 `[reflect]`，或为动态入口使用不同的源码函数名。

同名反射属性冲突采用相同诊断原则。继承覆盖合法时不产生重复名称错误。
