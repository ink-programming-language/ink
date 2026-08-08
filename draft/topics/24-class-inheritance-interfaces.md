# 议题 24：`class`、单继承与多接口

> 状态：已确认，2026-08-04 确认 `virtual func f();` 抽象语义；2026-08-05 确认 `static func`、尾随 `const`、类与虚函数的 `final`、统一继承类型列表、显式字段和类/接口成员区域；议题 25—31、35、54—58 与 Parser 议题 10、31—33、40 补充
> 确认日期：2026-08-01

## 1. 删除 `struct` 关键字

Ink 不提供 `struct` 关键字。用户定义的普通记录、资源类型、透明包装、泛型聚合和元数据类型统一使用 `class`。

```ink
class Point {
    var x: int;
    var y: int;
}

[noncopyable]
class File {
    // ...
}

[metadata]
class DisplayName {
    var value: string;
}
```

`struct` 不作为保留字，词法上可以作为普通标识符使用。

## 2. `class` 仍然是可内联的值类型

`class` 不隐含垃圾回收、堆分配、对象身份或引用语义。一个类值可以：

- 作为局部变量直接存储；
- 内联成为另一个类的字段；
- 成为数组、元组或泛型容器的元素；
- 在满足议题 02 的复制规则时按值传递；
- 通过 `[noncopyable]` 禁止复制；
- 使用构造函数、析构函数和 `defer` 管理资源。

```ink
class Pixel {
    var r: u8;
    var g: u8;
    var b: u8;
    var a: u8;
}

var pixel: Pixel;          // 可以直接位于栈上
var image: Pixel[1024];    // 元素直接内联存储
```

因此 `class` 只是 Ink 唯一的普通用户定义聚合类型关键字，不等同于 Java、C# 等语言中必须通过引用使用的对象类别。

## 3. 单一具体继承

一个 `class` 最多继承一个具体父类：

```ink
class Entity {
    var id: i64;
}

class Player : Entity {
    var health: i32;
}
```

派生对象包含一个父类子对象以及自己的字段。Ink v0 不支持多个具体父类，也不支持菱形具体继承。

禁止多个具体父类避免：

- 同一对象内存在多条父类子对象路径；
- 基类指针转换需要在多个布局分支间消歧；
- 构造、析构和复制顺序形成菱形问题；
- 反射成员查找需要任意线性化；
- 热更新同时迁移多条具体布局链。

## 4. 禁止按值切片

Ink 不允许通过派生类值隐式或显式产生只包含父类子对象的基类值。

```ink
class Entity {
    var id: i64;
}

class Player : Entity {
    var health: i32;
}

var player: Player;

const entity: Entity = player; // 编译错误：禁止按值切片
```

以下位置同样不能接受派生值代替基类值：

- 按值函数参数；
- 按值返回；
- 已有基类值赋值；
- 基类数组或值容器元素；
- `cast::<Entity>(derived)` 等显式转换。

如果程序确实需要根据派生对象的数据构造一个独立基类值，必须显式调用一个由类型作者声明的普通构造函数，并由该构造函数定义复制哪些数据。这是新对象构造，不是语言提供的切片转换。

## 5. 引用和指针上转型

派生类引用和指针可以上转型为基类引用和指针：

```ink
var player: Player;

const entity_ref: Entity& = player;
const entity_ptr: Entity* = &player;
```

上转型只借用或指向对象中的父类子对象，不复制任何字段，也不改变对象生命周期。

```ink
func inspect(entity: const Entity&);

inspect(player); // 合法：引用上转型
```

值容器不会获得隐式多态：

```ink
Vector::<Entity>  // 只保存完整 Entity 值
Vector::<Entity*> // 可以指向 Entity 或其派生类
```

基类引用或指针向派生类的安全下转型、接口横向转换和失败处理由议题 31 的 `try_cast` 规定。无检查的裸指针地址重解释仍由议题 11 的 `ptrcast` 规定。

## 6. 多接口

一个类可以实现任意多个接口，同时仍然最多只有一个具体父类：

```ink
interface Renderable {
    func render();
}

interface Serializable {
    func serialize(writer: Writer&);
}

class Player : Entity, Renderable, Serializable {
    // ...
}
```

继承列表中最多出现一个 `class`，但可以出现多个 `interface`。编译器根据被引用声明的种类区分具体父类与接口，不依赖列表位置猜测。

类和接口声明的 Parser 统一把继承列表解析为完整 `type` 序列：

```ebnf
inheritance_clause =
    ":", type, { ",", type } ;
```

该列表不接受尾随逗号。Parser 不查询名称所指声明的种类，因此泛型实例 `Base::<T>`、指针 `Base*`、函数类型或多个具体类都可以先形成结构完整的继承项 CST。语义分析随后要求每一项都是允许继承的闭合名义类型，并检查类最多有一个具体父类、其余项为接口、接口只能继承接口、没有重复项或继承环，以及不能继承 `final class`。开放泛型声明本身不是 `type`，也不能直接作为继承项。

接口不构成第二个具体父类子对象。接口引用采用议题 26 定义的两字胖引用；接口默认方法以及接口之间的同名方法冲突由议题 29 规定；接口可以按照议题 30 多继承其他接口。

议题 35 允许 `class` 和 `interface` 使用 `[exception]` 建立可捕获异常类型体系。异常类仍遵守本议题的单一具体继承和多接口规则，但普通类、普通接口和异常接口之间具有额外的实现限制。

## 7. 抽象虚函数

类中的无函数体 `virtual func` 声明表示抽象虚函数：

```ink
class Shape {
    virtual func area() -> f64;
}
```

含有未实现抽象虚函数的类自动成为抽象类，不能直接构造完整对象。派生类只有为全部继承的抽象虚函数提供合法覆盖后，才能成为可实例化的具体类。

Ink v0 不增加 `abstract` 关键字，也不采用 C++ 的 `= 0` 标记。是否抽象直接由 `virtual` 与函数体缺席共同表达。带函数体的 `virtual func` 仍是具有默认实现的普通虚函数。

没有 `virtual` 的普通类成员必须提供语句块函数体：

```ink
class Shape {
    func helper(); // 语义错误
}
```

Parser 议题 31 仍为结尾 `;` 的声明建立完整 CST；语义分析根据声明上下文和修饰符决定它是否为合法抽象函数。Ink v0 不提供类外成员定义，因此不能把普通无函数体成员解释为等待后续定义的前向声明。构造函数和析构函数也不能成为抽象函数，并继续遵守议题 03、06 的强制函数体规则。

## 8. 复制与 `[noncopyable]` 传播

父类是派生类值的组成部分，因此复制能力沿具体继承链传播：

- 父类 `[noncopyable]` 时，所有派生类自动不可复制；
- 父类可复制不保证派生类可复制，派生字段和自身属性仍参与判断；
- 派生类不能通过自定义构造或接口实现恢复被父类禁止的隐式复制能力；
- 复制派生类时复制完整派生对象，不存在只复制父类部分的隐式路径。

```ink
[noncopyable]
class Resource {
    // ...
}

class Socket : Resource {
    // Socket 自动不可复制
}
```

## 9. 构造、析构与布局

具体继承意味着父类子对象必须先于派生类自身状态完成初始化，并在派生类自身状态清理之后销毁。Parser 议题 38 采用 C++ 风格构造初始化列表传递基类构造参数：

```ink
func Player(id: i64, name: String)
    : Entity(id), name(name)
{}
```

直接具体基类先初始化，本类字段再按声明顺序初始化，最后执行构造函数体；初始化项的源码顺序不改变该顺序。通过基类进行动态销毁继续遵守议题 27。

没有虚函数的类不因为 `class`、具体继承或实现接口而自动增加对象内的 vptr、类型描述符指针或其他隐藏字段。实际使用 `try_cast` 时可以按照议题 31 为 vtable 或接口表生成对象外的最小类型描述符，但不改变类对象布局。虚类对象和接口引用的具体成本由议题 26 规定。

虚函数可以标记 `[reflect]`，其动态反射调用仍执行正常虚派发；描述符选择、覆盖元数据和适配器规则由议题 25 规定。vptr、vtable 和接口胖引用 ABI 由议题 26 规定，动态销毁由议题 27 规定。

## 10. 反射继承

具体父类链唯一，因此议题 23 的反射查找始终按以下顺序进行：

```text
当前类 → 直接父类 → 更上层父类
```

子类同名反射函数或属性覆盖父类成员。类实现多个接口时，接口反射描述符、实现关系和分派规则由议题 28 规定；接口继承产生的有效成员查找由议题 30 规定。不同来源接口的同名方法不会被隐式合并成一个运行时反射描述符。

## 11. 迁移规则

此前草案中的 `struct` 示例全部改为 `class`。这只是类型声明语法统一，不把这些类型改为隐式引用对象，也不改变已经确认的：

- 值复制与 `[noncopyable]`；
- RAII 和析构函数；
- 数组和字段内联布局；
- `[transparent]` 包装；
- `[metadata]` 元数据类型；
- `[reflect]` 动态反射选择。

## 12. 异步成员函数不复制接收对象

议题 54 规定 `class` 的异步成员函数保留普通声明和调用语法。调用 `object.method_async()` 时，任务帧保存对象地址作为非拥有原始 `this` 指针；它不会因为 `class` 是可内联值类型就复制完整对象，也不会延长对象生命周期。

这适用于可复制类和 `[noncopyable]` 类。接收对象必须保持在原地址并存活到异步成员任务最终结束；悬空访问和未经同步的冲突访问遵守议题 01、04 的普通原始指针规则。

议题 55 规定类异步虚函数在任务创建时完成一次 vtable 分派，并直接构造最终覆盖的惰性任务。议题 56 对接口调用采用相同的创建期分派原则：类实现任务保存调整后的原始 `this`，接口默认任务保存规范化到默认方法声明接口的胖接收者。议题 57 规定异步反射适配器只验证并进入这些普通任务构造路径，不推迟接收者分派到第一次 `await`。

## 13. 静态成员函数

类体内的普通函数声明自动拥有隐式 `this` 接收者。需要声明不依赖类实例的类型级函数时，使用 `static` 修饰符：

```ink
class Counter {
    var value: i32;

    static func add(left: i32, right: i32) -> i32 {
        return left + right;
    }

    func reset() {
        this->value = 0;
    }
}

const result = Counter.add(1, 2);
```

`static func` 没有隐式 `this`，函数体内出现 `this` 时产生语义错误。`static` 与 `virtual`、`override`、构造函数或析构函数组合也产生语义错误；通用 Parser 仍保存源码中的有序修饰符和完整函数名称，由语义分析根据声明上下文检查这些组合。

`static` 位于函数声明的修饰符序列中，并与其他函数修饰符一样出现在 `func` 之前。属性和装饰器仍必须整体位于全部函数修饰符之前。

Ink v0 只给类成员函数定义 `static` 语义。模块级自由函数使用 `private` 控制模块外可见性，不能用 `static` 表示 C++ 翻译单元内部链接；接口也不声明静态要求或静态默认方法：

```ink
static func helper(); // 语义错误：模块级函数不使用 static

interface Factory {
    static func create(); // 语义错误：v0 不支持静态接口成员
}
```

为了保持通用函数声明 Parser 简单，这些写法仍可建立包含 `static` 修饰符的完整 CST；是否位于类成员列表由语义分析检查。类外的其他声明区域同样不能赋予 `static` 另一套链接或存储期含义。

## 14. 只读成员函数

非静态成员函数默认拥有可写的隐式接收者。需要承诺不通过当前接收者修改对象时，在参数列表之后、可选返回子句之前写尾随 `const`：

```ink
class Buffer {
    var data: byte*;
    var length: ptrsize;

    func size() const -> ptrsize {
        return this->length;
    }

    func clear() {
        this->length = 0;
    }
}
```

普通 `func clear()` 的隐式接收者具有 `Buffer*` 语义；`func size() const` 的隐式接收者具有 `const Buffer*` 语义。只读成员函数可以由可写或只读对象调用；非 `const` 成员函数不能由只读对象调用。

尾随 `const` 是成员接收者限定符，不属于 `func` 前面的 `function_modifier`，也不是 attribute：

```ebnf
member_receiver_qualifier = "const" ;
```

它属于成员函数签名并参与重载、接口契约和虚函数覆盖。因此可以声明接收者限定不同的两个重载；可写对象同时适配二者时优先选择非 `const` 重载，只读对象只能选择 `const` 重载：

```ink
func data() -> byte*;
func data() const -> const byte*;
```

通用 Parser 在参数列表后保存可选 `member_receiver_qualifier`，不查询声明是否位于类或接口中。自由函数、`static func`、构造函数或析构函数带尾随 `const` 都可以形成完整 CST，但必须由语义分析报告非法组合。

普通函数“不修改任何外部状态”属于独立的纯度或副作用契约，不由尾随 `const` 表达。未来需要该能力时应使用类似 `[pure]` 的属性独立设计；这不改变 attribute 默认不属于普通函数签名的规则。

## 15. 类与接口声明的访问修饰符

类和接口声明都可以带一个访问修饰符：

```ink
public class User {}
private class InternalCache {}

public interface Reader {}
private interface InternalReader {}
```

模块顶层声明只允许 `public` 和 `private`；`protected` 只在具有外层类型的成员访问上下文中有意义。通用类型声明 Parser 仍可把三个硬关键字都保存为 `access_modifier`，所以顶层 `protected class Hidden {}` 可以形成完整 CST，随后由语义分析报告上下文错误。重复访问修饰符和互相冲突的访问修饰符也由语义分析检查。

声明 attribute list 仍位于访问修饰符之前：

```ink
[reflect]
private class InternalModel {}
```

类型声明前缀采用两个不可回退的阶段：先收集零个或多个 attribute list，再收集零个或多个类型修饰符：

```ebnf
type_declaration_prefix =
    type_annotation_sequence,
    type_modifier_sequence ;

type_annotation_sequence =
    { attribute_list } ;

type_modifier_sequence =
    { type_modifier } ;

type_modifier =
      access_modifier
    | "final" ;
```

类型声明不接受函数专用 decorator。进入 `type_modifier_sequence` 后不能再回到 attribute 阶段，因此 `private [reflect] class Service {}` 是语法错误。修饰符内部按源码顺序保存；格式化器使用 `access final class` 的规范顺序。`final interface`、重复 `final` 或多个访问修饰符仍可形成完整 CST，其合法性由语义分析检查。

访问修饰符在类和接口声明中是可选语法。Parser 只保存它是否出现及其源码形状；省略时采用哪一种默认可见性完全属于名称绑定和访问控制语义，不由语法分析决定。

## 16. 命名类与接口必须具有成员块

命名类和接口声明必须直接提供完整成员块，空成员块合法：

```ink
class Empty {}
interface Marker {}
```

Ink v0 不提供 C++ 式类型前向声明，因此 `class Node;` 和 `interface Reader;` 都是语法错误。模块名称绑定不依赖声明的文本顺序，递归指针或引用也可以直接引用当前类型，无需先引入不完整声明：

```ink
class Node {
    var next: Node*;
}
```

声明骨架为：

```ebnf
class_declaration =
    type_declaration_prefix,
    "class", identifier,
    [ generic_parameter_clause ],
    [ inheritance_clause ],
    class_member_block ;

interface_declaration =
    type_declaration_prefix,
    "interface", identifier,
    [ generic_parameter_clause ],
    [ inheritance_clause ],
    interface_member_block ;
```

`generic_parameter_clause` 与函数声明复用统一规则；接口同样可以泛型化。`inheritance_clause` 使用本议题已经确认的完整 `type` 列表。`class_member_block` 和 `interface_member_block` 的成员集合由对应 Parser 声明议题继续展开。

Parser 议题 40 在表达式上下文复用同一 class 前缀、泛型参数、继承列表和成员块，只把类名改为可选；命名声明上下文中的 `identifier` 仍然必需。

## 17. 字段必须显式写 `var` 或 `const`

类字段与局部、module 绑定和 `for` 绑定一样，必须显式声明自身可变性：

```ink
class Point {
    var x: int;
    var y: int;
}

class Config {
    private const max_count: int = 1024;
}
```

旧的裸字段 `x: int;` 不再属于字段声明语法。`var` 表示字段可以通过可写对象路径重新赋值，`const` 表示字段本身不可重新赋值；两者不递归改变字段类型内部指针或引用目标的访问能力。字段声明 Parser 可以从 `var` 或 `const` Token 确定成员种类，不需要把任意 `identifier :` 前缀猜测为字段。

字段必须显式写出类型标注，不允许根据字段初始化器推导布局类型：

```ebnf
field_core =
    field_binding_mode,
    identifier,
    ":", type,
    [ field_initializer ] ;

field_binding_mode =
      "var"
    | "const" ;

field_initializer =
    "=", expression ;
```

因此 `var value: i32;` 和 `const maximum: i32 = 100;` 具有合法字段核心，而 `var value = 0;`、`const maximum = 100;` 都是语法错误。类型标注使类型布局、ABI 和反射声明不依赖初始化表达式的隐式推导结果。

`field_initializer` 对 `var` 和 `const` 都是可选语法，所以 `var first: i32;`、`var second: i32 = 2;`、`const third: i32;` 和 `const fourth: i32 = 4;` 都能形成完整 CST。Parser 不判断无初始化器字段是否合法；语义分析负责默认初始化、构造函数返回前的完全初始化、`const` 字段恰好初始化一次，以及声明初始化器与构造过程之间的冲突。

完整字段声明同样采用 annotation 与 modifier 不回退的两阶段前缀：

```ebnf
field_declaration =
    field_annotation_sequence,
    field_modifier_sequence,
    field_core,
    ";" ;

field_annotation_sequence =
    { attribute_list } ;

field_modifier_sequence =
    { access_modifier } ;
```

字段不接受 decorator；`static`、`final`、`virtual` 等也不属于字段修饰符。全部 attribute list 必须位于访问修饰符之前，访问修饰符又必须位于 `var` 或 `const` 之前。因此 `[reflect] private var value: i32;` 合法，`private [reflect] var value: i32;` 和 `var public value: i32;` 是语法错误。多个访问修饰符可以先形成 CST，随后由语义分析报告重复或冲突；省略访问修饰符的默认含义也不由 Parser 决定。

## 18. `final` 类

`final class` 禁止当前类继续成为其他具体类的父类：

```ink
final class Leaf : Base, Renderable {
    override final func update() {
        // 覆盖 Base 的虚槽并封闭它
    }

    override func render() {
        // 实现 Renderable 接口
        // ...
    }
}
```

`final` 不妨碍该类继承一个已有具体父类、实现接口、声明虚函数，或通过父类和接口引用参与动态分派；它只禁止出现 `class Derived : Leaf`。把 `final` 类写入具体继承列表可以形成完整声明 CST，但语义分析必须拒绝该继承关系。

类级 `final` 位于 `class` 之前。声明上的 attribute list 仍整体位于声明修饰符之前，例如：

```ink
[reflect]
final class Service {
    // ...
}
```

`final interface` 不属于 Ink v0 的声明形式；接口扩展关系继续由议题 30 的接口继承规则控制。改变一个公开类是否为 `final` 会改变合法继承集合和优化假设，属于源码与 ABI 兼容性变化，不能由热更新静默切换。

## 19. 类和接口允许嵌套命名类型

类和接口成员块都允许嵌套声明命名 `class`、`interface`，并在枚举声明语法确定后同样允许嵌套 `enum`：

```ink
class Tree {
    private class Node {
        var value: i32;
        var next: Node*;
    }

    public interface Visitor {
        func visit(node: const Node&);
    }
}

interface Container {
    interface Iterator {
        func next() -> bool;
    }
}
```

嵌套类型复用完整类型声明语法，包括 attribute、可选访问修饰符、泛型参数、继承列表和强制成员块。嵌套类型不捕获外层对象，没有隐式外层 `this`，也不在外层对象布局中增加字段；这一点与 C++ 嵌套类型一致。现有成员后缀可以把限定名称写为 `Tree.Node`，Parser 不查询成员是否为类型，语义分析再检查名称绑定和访问权限。

因此 `protected class Nested {}` 可以在类型成员上下文中形成有意义的声明。接口中嵌套的具体类仍只是位于接口命名空间下的独立类型声明，不是静态接口字段，也不会使接口获得实例布局。

## 20. 不支持访问分区标签

类和接口成员访问只使用逐声明的可选访问修饰符，不支持 C++ 式持续影响后续成员的 `public:`、`protected:` 或 `private:` 分区：

```ink
class Account {
    private var balance: i64;

    public func get_balance() const -> i64 {
        return this->balance;
    }
}
```

`public`、`protected`、`private` 出现在成员声明前缀中时，后面必须继续形成字段、函数或嵌套类型等具体声明；紧随 `:` 是语法错误。逐声明修饰符不会建立跨成员的 Parser 状态，也不会跨 `comptime` declaration block 传播一个隐含访问模式。省略修饰符的默认访问仍由语义分析决定。

## 21. 类成员块的封闭成员集合

类成员块允许字段、函数、嵌套命名类型以及在类声明区域执行的 `comptime` 项：

```ebnf
class_member_block =
    "{", { class_member_item }, "}" ;

class_member_item =
      field_declaration
    | function_declaration
    | class_declaration
    | interface_declaration
    | enum_declaration
    | class_member_comptime_item ;
```

`enum_declaration` 的内部产生式由枚举 Parser 议题定义。`class_member_comptime_item` 是 Parser 议题 32 的统一 `ComptimeRegionControl` 对 `RegionKind::ClassMember` 的标准 EBNF 适配名称，不建立类专用 `comptime` 语义；block、`if`、`match`、`for` 和 `while` 都使用同一控制 Parser，并把每个 body 递归解析为 `class_member_block`。

成员块不允许 `import`、module 或局部 `binding_declaration`、普通表达式或控制流语句、decorator 声明以及单独的空 `;`。decorator 只能在 module 顶层声明；成员函数仍可通过声明前缀应用已经可见的 decorator。成员位置以 `var` 或 `const` 开始的声明始终进入 `field_declaration`，不会建立局部或 module 绑定。

## 22. 接口成员区域复用完整声明形状

接口成员 Parser 广泛接受与类成员相同的声明形状：

```ebnf
interface_member_block =
    "{", { interface_member_item }, "}" ;

interface_member_item =
      field_declaration
    | function_declaration
    | class_declaration
    | interface_declaration
    | enum_declaration
    | interface_member_comptime_item ;
```

`interface_member_comptime_item` 是 Parser 议题 32 的统一 `ComptimeRegionControl` 对 `RegionKind::InterfaceMember` 的标准 EBNF 适配名称。它与 class、module 和 statement 区域共享控制 Parser、CST 和 Partial Evaluation，只把 body 解析为 `interface_member_block`。

因此 `var state: i32;`、`static func create();` 或与接口同名的函数都可以先形成完整接口成员 CST。语义分析随后拒绝接口实例字段、v0 不支持的静态接口函数、接口构造/析构、非法访问级别和其他不符合接口契约的成员。`comptime` 未选中分支仍必须语法正确，但其中未激活声明不进入这些语义检查。

## 23. 后续问题

以下内容留给独立议题：

- 热更新时继承布局的兼容性检查。
