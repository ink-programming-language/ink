# Parser 议题 40：Class 类型表达式

> 状态：已确认，复用普通 class 的完整语法，只在表达式上下文把类名改为可选
> 确认日期：2026-08-06

## 1. 表面形式

`class` 可以在普通表达式位置产生一个编译期类型值：

```ink
func make_storage<T: type>() -> type {
    return class {
        var value: T;
    };
}
```

可选的局部类名用于类体内部的递归引用：

```ink
func make_node<T: type>() -> type {
    return class Node {
        var value: T;
        var next: Node*;
    };
}
```

`Node` 不进入外层函数或 module 作用域。表达式结果本身是 `type` 值，调用者可以返回、保存或传递它。

## 2. 复用普通 class 文法

命名 class 声明和 class 类型表达式共享同一个定义尾部：

```ebnf
class_declaration =
    type_declaration_prefix,
    "class", identifier,
    class_definition_tail ;

class_type_expression =
    type_declaration_prefix,
    "class", [ identifier ],
    class_definition_tail ;

class_definition_tail =
    [ generic_parameter_clause ],
    [ inheritance_clause ],
    class_member_block ;
```

因此两种入口使用相同的：

- attribute 与类型修饰符前缀；
- 可选泛型参数子句；
- 可选继承列表；
- 字段、函数、嵌套类型和 `comptime` 成员区域；
- 成员 Parser、CST 子结构和错误恢复。

唯一的语法差别是名称要求：声明上下文中的 `class_declaration` 必须有 `identifier`，表达式上下文中的 `class_type_expression` 可以没有名称。

## 3. 接入表达式

class 类型表达式属于结构化基础表达式：

```ebnf
structured_expression =
      match_expression
    | class_type_expression ;
```

Parser 议题 35 的聚合初始化已经改为普通表达式专用 postfix 后缀，不再占用 `structured_expression` 分支。class 类型表达式本身仍是可后缀基础表达式，因此可以直接作为聚合目标：`class { var value: i32; } { value: 1 }`。

它可以出现在任何普通表达式入口：

```ink
const Generated: type = class { var value: i32; };
consume_type(class : Base { var value: i32; });
return class Local { var next: Local*; };
```

外层绑定、实参或 `return` 提供自己的结束符。class 成员块的 `}` 结束类型表达式自身；外层是否还需要 `;`、`,` 或 `)` 由原有产生式决定。

## 4. 上下文保持声明与表达式分离

module 和类型成员的 declaration item Parser 继续进入 `class_declaration`，所以类名必需：

```ink
class Named {} // 声明合法
class {}       // 顶层声明非法：缺少名称
```

表达式 Parser 才进入 `class_type_expression`：

```ink
const T: type = class {}; // 表达式合法
```

Parser 不把一个缺名的顶层 class 声明自动改造成被丢弃的表达式，也不让 class 类型表达式在外层隐式建立声明名称。

## 5. 前缀与数组表达式消歧

无前缀形式从硬关键字 `class` 直接识别。以 `public`、`protected`、`private` 或 `final` 开始的形式可以先收集普通 `type_modifier_sequence`，再要求后续出现 `class`。

`type_annotation_sequence` 的 `[` 与数组表达式共享起始 Token。表达式 Parser 使用局部 checkpoint：只有完整解析零组或多组 `attribute_list`、随后可选类型修饰符并看到 `class` 时，才提交为 `class_type_expression`；否则回滚并按普通数组表达式解析。该试探只查看 Token 结构，不查询 attribute 名称或类型信息。

## 6. Parser 与语义边界

由于类型表达式完整复用普通 class 文法，Parser 也保留泛型参数、访问修饰符、`final`、继承列表及其任意语法组合。某个组合是否适用于局部或匿名类型、是否允许建立开放泛型声明、继承项是否合法以及表达式是否最终产生闭合 `type`，均由后续语义分析决定。

议题 67 当前要求结构化类型生成最终产生闭合类型，并禁止在声明表达式中动态创建新的开放泛型形参列表。该限制不需要把普通 class 的泛型子句从 Parser 中复制删除；语义阶段可以根据 `ClassTypeExpression` 上下文拒绝不允许的开放结果。

局部名称的作用域只包含当前 class 定义，且只用于类型内部引用。名称重复、无限递归按值字段、对象布局、名义类型身份和实例缓存同样属于语义规则。

## 7. CST

CST 使用独立外层节点并复用相同内部结构：

```text
ClassDeclaration
├─ TypeDeclarationPrefix
├─ Keyword(class)
├─ Identifier
└─ ClassDefinitionTail

ClassTypeExpression
├─ TypeDeclarationPrefix
├─ Keyword(class)
├─ Identifier?
└─ ClassDefinitionTail
```

这样工具可以区分“向外层作用域声明类”和“产生一个类型值”，同时对泛型、继承和成员块使用同一套遍历与格式化逻辑。

## 8. 结论

Ink 的 class 类型表达式不是第二套匿名类语法。它完整复用普通 class 的声明前缀、泛型、继承和成员块，只在表达式上下文把类名从必需改为可选。无名形式直接产生类型值；具名形式提供只在类体内部可见的递归名称。声明上下文仍强制类名存在，Parser 不根据名称或类型语义混淆两个入口。
