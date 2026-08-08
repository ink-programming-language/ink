# Parser 议题 10：`var`、`const` 与元组解构声明

> 状态：已确认，2026-08-04 移除 `let` 声明语法、分离编译期求值并接入元组解构；2026-08-08 确认 `let` 仍为硬关键字；Parser 议题 25 复用 `var`/`const` 作为普通 `for` 绑定模式，议题 29 统一类型内部 `const`，议题 30 允许初始化表达式中的前置类型限定；2026-08-05 确认字段同样必须显式写 `var` 或 `const`，完整字段产生式见议题 33；2026-08-06 由议题 39 为 module 顶层绑定增加访问修饰 wrapper
> 确认日期：2026-08-04

## 1. 基本文法

普通绑定声明分为两种：

```ebnf
binding_declaration =
    var_declaration
  | const_declaration ;

var_declaration =
    "var",
    ( identifier,
      ( ":", type, [ "=", expression ]
      | "=", expression )
    | tuple_pattern, "=", expression ),
    ";" ;

const_declaration =
    "const",
    ( identifier, [ ":", type ]
    | tuple_pattern ),
    "=", expression, ";" ;
```

`tuple_pattern` 由议题 23 定义。两种声明都按照 Parser 议题 08 以显式 `;` 结束；名称形式建立一个绑定，元组形式可以在一次不可反驳解构中建立多个绑定。

## 2. 声明关键字不可省略

每个普通名称绑定都必须以 `var` 或 `const` 开头：

```ink
const size = calculate_size();
var count: i32 = 0;
```

类似 C++ 的裸类型声明形状不属于 Ink 的普通绑定文法：

```ink
Count: int = 3; // 非法：缺少 var 或 const
```

这使 Parser 能从第一个 Token 直接识别声明，并避免名称表达式、类型标注和标签式语法之间的歧义。`let` 是保留的硬关键字，但 Ink v0 不提供 `let` 声明产生式，因此它不能用于声明，也不能匹配普通 `identifier`。

Parser 议题 25 的普通 `for` 绑定同样必须在 pattern 前显式写 `var` 或 `const`：`for (const value in values) { ... }`。该循环头没有 `=` 初始化器和结尾分号，因此不是本节的 `binding_declaration`，但两种关键字保持相同的顶层绑定含义。

函数参数和泛型参数由各自声明产生式定界，仍使用 `name: type` 形状；它们不会被当成普通绑定声明。字段虽然由独立的成员声明产生式解析，但同样必须显式写 `var` 或 `const`：

```ink
class Point {
    var x: i32;
    const origin_id: i64 = 0;
}
```

旧的裸字段 `x: i32;` 是语法错误。字段声明还必须显式标注类型，不允许写成 `var x = 1;` 或 `const x = 1;`。字段产生式允许 `var` 和 `const` 字段都省略初始化器；初始化是否充分由构造和类型语义检查。字段不直接继承本议题中局部与 module 绑定的全部形状。

## 3. `var`

`var` 建立运行时可修改绑定，并且必须提供类型或初始化表达式：

```ink
var count: i32 = 0;
var result = calculate();
var buffer: Buffer;
```

`var buffer: Buffer;` 不建立未初始化存储，而是按照 `Buffer` 的普通默认初始化规则初始化最终存储。如果该类型不能合法默认初始化，则声明非法。

以下形式非法：

```ink
var value;
```

提供初始化表达式时，该表达式只求值一次，并按照既有构造、复制和原地初始化规则建立绑定值。

## 4. `const`

`const` 建立顶层不可重新赋值的绑定，初始化表达式必需：

```ink
const default_port: u16 = 443;
const port: u16 = read_port();
const result = calculate();
```

初始化表达式可以在运行时求值；`const` 本身不要求编译期求值。类型标注可以省略，此时从初始化表达式推导绑定类型。议题 64 禁止的是泛型实参推导，不禁止普通局部值的类型推导。

以下形式非法：

```ink
const value: Data;
const value;
```

Ink 不提供“先声明一个未初始化的不可变绑定，以后恰好赋值一次”的普通绑定状态。

## 5. `comptime` 与绑定可变性正交

是否可重新赋值由 `var` 或 `const` 决定；是否强制在编译期求值由初始化表达式中的 `comptime` 决定：

```ink
const size = calculate_size();              // 初始化可以在运行时执行
const capacity = comptime calculate_size(); // 必须在编译期成功求值
var table = comptime make_table();           // 编译期求值后建立可变绑定
```

第三种形式要求编译期结果可以作为普通运行时值建立 `table`；之后 `table` 仍然可以重新赋值。`const` 可以出现在 module 声明区或普通局部声明区，其值的存储、地址取得和目标文件表示由常量与布局议题确定，不改变本节语法。

## 6. 顶层绑定与目标访问能力

`const` 绑定不能作为普通重新赋值的目标；`var` 绑定可以。绑定可变性只约束直接保存的值，不递归改变指针或引用目标已有的访问能力：

```ink
const pointer: Data* = get_data();

pointer->update(); // Data* 允许修改目标
pointer = other;   // 非法：pointer 绑定不可重新赋值
```

`var`、`const` 是声明方式，`&`、`*` 和类型内部的 `const` 是类型构造，两层不能互相替代。`var data: Data` 建立拥有自身 `Data` 值的可变绑定；`const alias: Data&` 建立不能重新指向、但允许写目标的非拥有引用。引用类型自身“不能重新指向”的规则不会被外层声明关键字取消。

声明起始位置的 `const` 与类型内部的 `const` 由语法上下文区分，并表示两项独立能力：

```ink
var data: Data = make_data();                       // 拥有 Data，绑定可修改
const writable_pointer: Data* = get_data();         // 绑定不可重写，目标可写
var readonly_pointer: const Data* = get_data();     // 绑定可重写，目标只读
const readonly_view: const Data* = get_data();      // 绑定不可重写，目标只读
const writable_alias: Data& = data;                 // 引用绑定固定，目标可写
const readonly_alias: const Data& = data;           // 引用绑定固定，目标只读
```

Parser 在 `binding_declaration` 起始位置把 `const` 解析为不可变绑定声明；在冒号后议题 29 的统一 `type` 及其他明确类型位置把它解析为前置目标类型限定符。议题 30 还允许同一限定符出现在初始化器、实参和 `return` 等普通表达式入口中：`const PointerType: type = const Data*;` 的第一个 `const` 建立绑定，第二个 `const` 限定 `Data`。Tokenizer 仍只产生同一种 `Keyword(Const)`。绑定不可重新赋值不等同于递归深度不可变。

## 7. 不支持逗号多声明

普通绑定声明不接受逗号分隔的多名称、多初始化器形式：

```ink
const a, b = 1, 2;
var x, y: i32;
const left, right = 10, 20;
```

这些形式都不符合 EBNF。应写成独立声明：

```ink
const a = 1;
const b = 2;
```

每个独立名称声明因此具有自己的初始化时点、名称可见起点、失败清理和源码位置，不需要定义逗号两侧的数量匹配或并行绑定规则。

圆括号元组模式不是逗号多声明。它只有一个右侧表达式，并按第 8 节的规则解构该表达式产生的一个元组值：

```ink
const (a, b) = pair;
```

## 8. 元组解构声明

普通绑定声明可以在关键字后使用议题 23 的 `tuple_pattern`：

```ink
const (left, right) = pair;          // 两个绑定都不可重新赋值
var (x, y) = point;                  // 两个绑定都可修改
const (header, (width, height)) = value;
const (first, _) = pair;             // 第二个位置不建立绑定
```

规则为：

- 外层 `var` 或 `const` 统一作用于模式中建立的全部名称；pattern 内不重复写声明关键字；
- 右侧初始化表达式必需并且只求值一次；
- 右侧求值完成后，绑定按 pattern 源码顺序从左到右、对嵌套元组深度优先初始化；
- 整个声明成功后全部新名称才同时进入作用域，初始化过程不能观察到一组部分可见的绑定；
- 初始化中途失败时，只清理已经成功建立的绑定，并按其建立逆序清理；
- `_` 不建立绑定，嵌套元组继续递归使用同一规则；
- 同一模式中重复名称、元组形状或元素数量不匹配属于语义错误；
- v0 不在元组模式后接受整体或逐元素类型标注，各绑定类型从对应元素推导；
- 枚举 `variant_pattern` 是可反驳模式，不能用于普通声明。

因此以下形式非法：

```ink
const (left, var right) = pair;       // pattern 内不能重复声明关键字
const (left: i32, right: i32) = pair; // v0 没有逐元素类型标注
const .some(value) = optional;        // 可反驳模式只能用于匹配结构
```

Parser 只识别不可反驳的语法形状。元素按值初始化、引用元素、不可复制值和临时对象遵守议题 69 的普通元组解构语义；模式本身不隐式把值元素改成借用。

## 9. Module 与局部声明

`binding_declaration` 仍是 module 与局部声明共用的核心形状。module 顶层在它外面增加议题 39 的访问修饰 wrapper：

```ebnf
top_level_binding_declaration =
    top_level_binding_modifier_sequence,
    binding_declaration ;

top_level_binding_modifier_sequence =
    { access_modifier } ;
```

因此以下 module 顶层声明都符合语法：

```ink
public const api_version = 1;
private var cache: Cache;
const default_visibility = 0;
```

零个访问修饰符合法，省略时采用什么默认可见性不由 Parser 决定。`access_modifier` 复用议题 31 的 `public`、`protected`、`private` 产生式，以保持各类声明的前缀解析一致；module 顶层不允许 `protected`，重复或冲突修饰符也非法，但这些约束均在语义阶段检查。

普通 `StatementBlock` 继续直接使用无访问前缀的绑定核心：

```ink
func update() {
    var count = 0;
    private var hidden = 0; // 非法：局部绑定没有访问前缀
}
```

Parser 根据父 CST 上下文建立不同节点：

```text
TopLevelBindingDeclaration
├─ AccessModifier*
└─ NamedBindingDeclaration | TupleDestructuringDeclaration

LocalBindingDeclaration
└─ NamedBindingDeclaration | TupleDestructuringDeclaration
```

访问前缀只属于 `TopLevelBindingDeclaration` wrapper，不改变内部 `var`、`const`、类型标注、初始化器或元组模式的 Token 顺序。作用域、具体访问权限、module 初始化顺序和局部 RAII 生命周期由对应语义规则处理。

## 10. CST 与恢复

CST 分别建立 `NamedBindingDeclaration` 或 `TupleDestructuringDeclaration`，并保留关键字、名称或完整 `TuplePattern`、可选类型标注、初始化器和结尾分号。缺少名称、模式元素、逗号、右括号、类型、`=`、表达式或分号时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode` 恢复，不能把元组模式外的后续逗号解释为额外普通绑定。

## 11. 确认结论

Ink 的普通绑定声明必须以 `var` 或 `const` 开头，`Count: int = 3;` 之类的裸声明非法。module 顶层可以在绑定核心前增加访问修饰符，局部绑定不能增加该前缀；省略时的默认可见性属于语义规则。普通 `for` 的每轮绑定和类字段也显式复用这两个关键字。`var` 表示可变绑定，`const` 表示不可重新赋值绑定；二者都可以接收运行时或编译期结果，`comptime` 独立负责强制编译期求值。普通声明的关键字后可以是单个 Identifier，也可以是不可反驳的 `tuple_pattern`；后者使用一个只求值一次的初始化器同时建立全部名称。
