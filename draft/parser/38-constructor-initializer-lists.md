# Parser 议题 38：构造初始化列表

> 状态：已确认，采用 C++ 风格的冒号初始化列表
> 确认日期：2026-08-06

## 1. 表面形式

构造函数可以在参数子句之后、函数体之前使用冒号初始化列表，直接初始化具体基类子对象和本类字段：

```ink
class Entity {
    var id: i64;

    func Entity(id: i64)
        : id(id)
    {}
}

class Player : Entity {
    var name: String;

    func Player(id: i64, name: String)
        : Entity(id), name(name)
    {}
}
```

初始化项使用与普通调用相同的圆括号实参结构。Ink 不为构造初始化列表增加 `base`、`super`、`constructor` 或其他关键字。

## 2. EBNF

```ebnf
function_body =
      function_definition
    | ";" ;

function_definition =
    [ constructor_initializer_clause ],
    statement_block ;

constructor_initializer_clause =
    ":", constructor_initializer,
    { ",", constructor_initializer } ;

constructor_initializer =
    constructor_initializer_target,
    call_suffix ;

constructor_initializer_target =
    identifier,
    { constructor_initializer_target_suffix } ;

constructor_initializer_target_suffix =
      ".", identifier
    | generic_argument_suffix ;
```

`call_suffix` 复用 Parser 议题 15、37 的完整普通调用实参规则，因此支持位置实参、`...expression` 列表展开、命名实参以及独占实参序列的裸 `...`。某个初始化目标是否允许全参数转发由后续语义检查决定。`generic_argument_suffix` 复用议题 16，使 `Base<T>(arguments)`、`module.Base<T>(arguments)` 等限定基类名称保持普通泛型拼写。

冒号后至少存在一个初始化项，列表不允许尾随逗号。初始化列表只能伴随真实 `statement_block`；以 `;` 结束的无函数体声明不能携带初始化列表。

## 3. 与通用函数声明的连接

Parser 议题 31 不通过所属类名称预先判断某个 `func Identifier(...)` 是否为构造函数。因此通用函数定义 Parser 在参数、可选接收者限定和可选返回子句之后，按 `:` 识别可选初始化列表，再解析必需的语句块。

```ink
func Player(id: i64) : Entity(id) {}
```

Parser 只建立结构完整的 `ConstructorInitializerClause`。该函数是否确实是构造函数、目标是否为直接具体基类或本类字段，均在名称绑定和构造语义阶段确定。普通函数、析构函数、接口方法或 decorator 声明即使具有同样 Token 形状，也不能因此获得构造初始化语义。

## 4. 初始化目标

语义上，一个初始化项只能命名：

- 当前类的直接具体基类；
- 当前类直接声明的实例字段。

接口、间接基类、继承字段、静态函数、普通方法和其他名称不能成为初始化目标。Parser 不查询这些类别；限定名称与泛型后缀只形成中性目标语法。

字段项直接初始化最终字段存储，不先默认构造再执行赋值，因此可以初始化 `const` 字段和不可复制字段：

```ink
class ResourceOwner : OwnerBase {
    const handle: Handle;

    func ResourceOwner(path: String)
        : OwnerBase(path), handle(open_handle(path))
    {}
}
```

## 5. 初始化顺序

初始化顺序参考 C++，不由初始化项的源码排列决定：

1. 直接具体基类子对象；
2. 本类字段，按字段声明顺序；
3. 构造函数体。

初始化列表只把参数表达式关联到目标，不重新排列对象布局或生命周期顺序。为避免误读，格式化器和诊断应建议源码也按照“基类、字段声明顺序”排列。

未列出的基类或字段继续使用已经确定的声明初始化器或默认初始化规则。重复目标、未知目标、缺少必需初始化以及初始化表达式的类型或异常行为属于构造语义检查，不改变 Parser 结构。

## 6. 结论

Ink 使用 C++ 风格的 `: Base(arguments), field(arguments)` 构造初始化列表。列表位于完整函数声明头之后、语句块之前，复用普通调用实参与泛型限定名称语法，不引入新的关键字。Parser 中性保存初始化目标和实参；语义阶段只允许直接具体基类与本类实例字段，并按照基类优先、字段声明顺序执行直接初始化。
