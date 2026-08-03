# Parser 议题 10：`let`、`var` 与 `const` 绑定声明

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 基本文法

普通绑定声明分为三种：

```ebnf
binding declaration =
    let declaration
  | var declaration
  | const declaration ;

let declaration =
    "let", identifier, [ ":", type ], "=", expression, ";" ;

var declaration =
    "var", identifier,
    ( ":", type, [ "=", expression ]
    | "=", expression ),
    ";" ;

const declaration =
    "const", identifier, [ ":", type ], "=", expression, ";" ;
```

三种声明都准确建立一个普通名称绑定，并按照 Parser 议题 08 以显式 `;` 结束。

## 2. `let`

`let` 建立运行时不可重新赋值的绑定，初始化表达式必需：

```ink
let size = calculate_size();
let port: u16 = read_port();
```

类型标注可以省略；省略时从初始化表达式推导绑定类型。议题 64 禁止的是泛型实参推导，不禁止已经广泛用于既有 Ink 示例的局部值类型推导。

以下形式非法：

```ink
let value: Data;
let value;
```

Ink 不提供“先声明一个未初始化的 `let`，以后恰好赋值一次”的普通绑定状态。

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

`const` 声明建立编译期常量，初始化表达式必需且必须在编译期求值得到结果：

```ink
const default_port: u16 = 443;
const capacity = calculate_capacity();
```

第二个例子仅在 `calculate_capacity()` 能在当前编译期环境中成功执行时合法。类型标注可以省略并从编译期结果推导。

`const` 可以出现在 module 声明区或普通局部声明区；其值的存储、地址取得和目标文件表示由常量与布局议题确定，不改变本节语法。

## 5. 绑定可变性

`let` 和 `const` 不能作为普通重新赋值的目标；`var` 可以。绑定可变性只约束直接保存的值，不递归改变指针或引用目标已有的访问能力：

```ink
let pointer: Data* = get_data();

pointer->update(); // Data* 允许修改目标
pointer = other;   // 非法：pointer 绑定不可重新赋值
```

类似地，`let` 保存的 `T&` 或 `const T&` 继续分别遵守引用类型自身的可写或只读能力。绑定不可重新赋值不等同于递归深度不可变。

## 6. `const` 声明与类型限定

声明位置的 `const` 和类型内部的 `const` 由语法上下文区分：

```ink
const limit: i32 = 100;       // const declaration
let view: const Data* = ptr;   // const type qualifier
```

Parser 在 `binding declaration` 起始位置把 `const` 解析为常量声明；在冒号后的类型语法及其他类型位置把它解析为类型限定符。Tokenizer 仍只产生同一种 `Keyword(const)`。

## 7. 一个声明只绑定一个名称

普通绑定声明不接受逗号分隔的多名称、多初始化器形式：

```ink
let a, b = 1, 2;
var x, y: i32;
const left, right = 10, 20;
```

这些形式都不符合 EBNF。应写成独立声明：

```ink
let a = 1;
let b = 2;
```

每个声明因此具有独立的初始化时点、名称可见起点、失败清理和源码位置，不需要定义左右数量匹配或并行绑定规则。

## 8. 解构不属于普通声明

本议题只接受单个 `identifier`，不接受元组或枚举模式作为普通声明左侧：

```ink
let (a, b) = pair;
```

该形式是否增加，以及解构时如何处理借用、复制、不可复制值和临时对象，将由未来独立解构议题确定。当前已确认的 `if let` 与 `match` 模式绑定是各自控制结构的专用语法，不会让普通 `let declaration` 自动接受任意模式。

## 9. Module 与局部声明

相同绑定产生式可以在允许绑定的 module 声明区和普通 `StatementBlock` 中使用。Parser 根据父 CST 上下文建立不同节点：

```text
ModuleBindingDeclaration
LocalBindingDeclaration
```

作用域、访问权限、module 初始化顺序和局部 RAII 生命周期由对应语义规则处理；声明的 Token 顺序和基础语法保持一致。

## 10. CST 与恢复

CST 保留关键字、名称、可选类型标注、初始化器和结尾分号。缺少名称、类型、`=`、表达式或分号时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode` 恢复，不能把后续逗号解释为额外普通绑定。

## 11. 确认结论

Ink 使用 `let` 表示必须初始化的运行时不可重新赋值绑定，`var` 表示具有明确类型或初始化器的运行时可变绑定，`const` 表示必须在编译期求值的常量绑定。普通声明一次只绑定一个 Identifier，不支持 `let a, b = 1, 2`，解构绑定留给独立议题。
