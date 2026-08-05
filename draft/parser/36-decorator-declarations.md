# Parser 议题 36：装饰器声明

> 状态：已确认，只允许顶层声明；Parser 议题 37 补充隐式 continuation 的裸 `...` 全参数转发
> 确认日期：2026-08-05

## 1. 复用函数声明骨架

装饰器声明复用 Parser 议题 31 的函数声明结构，只把声明关键字 `func` 替换为 `decorator`：

```ebnf
decorator_declaration =
    decorator_attribute_sequence,
    function_modifier_sequence,
    "decorator",
    function_name,
    [ generic_parameter_clause ],
    function_parameter_clause,
    [ member_receiver_qualifier ],
    [ return_clause ],
    function_body ;
```

`function_modifier_sequence`、`function_name`、`generic_parameter_clause`、`function_parameter_clause`、`member_receiver_qualifier`、`return_clause` 和 `function_body` 全部直接复用函数声明的既有产生式。

```ink
decorator trace(name: string) {
    log("enter", name);
    const result = function(...);
    log("leave", name);
    return result;
}
```

装饰器的形参都是普通 `name: type` 形参。隐式 continuation `function` 不写入形参列表。

## 2. 声明前缀差异

装饰器声明自身只接受 attribute list，不接受 decorator application：

```ebnf
decorator_attribute_sequence =
    { attribute_list } ;
```

attribute 必须全部位于函数修饰符之前，与函数声明的阶段顺序一致：

```ink
[reflect]
public async decorator trace(name: string) {
    return await function(...);
}
```

下列 meta-decorator 形式不属于 `decorator_declaration`：

```ink
@measure
decorator trace() {
    return function(...);
}
```

这与普通函数声明的 `function_annotation_sequence` 不同；后者仍可交错包含 attribute list 和 decorator application。

## 3. 同步与异步形式

`async` 继续作为现有 `function_modifier` 出现在 `decorator` 之前：

```ink
decorator sync_trace() {
    return function(...);
}

async decorator async_trace() {
    return await function(...);
}
```

不增加 `async_decorator` 复合关键字或第二套声明产生式。

## 4. 其他声明组成部分

由于其余结构完全复用函数声明，装饰器在语法上具有相同的：

- 可选泛型参数子句；
- 必需的普通运行时参数括号；
- 可选尾随 `const`；
- 可选返回子句；
- block 或分号形式的 body。

这些结构的组合是否适用于某个装饰器，不改变本节已经确定的 Parser 形状。

## 5. 声明位置

`decorator_declaration` 只进入 Parser 议题 05 的 `top_level_declaration`：

```ebnf
top_level_declaration =
      binding_declaration
    | function_declaration
    | decorator_declaration
    | class_declaration
    | interface_declaration
    | enum_declaration ;
```

类和接口成员可以在普通函数声明前使用 decorator application，但其成员项不接受 decorator declaration：

```ink
decorator trace() {
    return function(...);
}

class Service {
    @trace
    func run() {}
}
```

下列成员声明不属于类成员文法：

```ink
class Service {
    decorator trace() {
        return function(...);
    }
}
```

## 6. 结论

装饰器声明是把函数声明关键字 `func` 替换为 `decorator` 的同形声明。语法前缀只接受 attribute，不接受另一个 decorator application；`function` continuation 隐式存在于装饰器体内，不占普通形参位置。装饰器只能在顶层声明，类和接口成员只允许应用已经可见的 decorator。
