# Parser 议题 26：`return` 与 `defer` 语句

> 状态：已确认，2026-08-05 确认省略函数返回类型固定为 `void`
> 确认日期：2026-08-04

## 1. 基本产生式

Ink 的函数返回与延迟清理使用以下 EBNF：

```ebnf
return_statement =
    "return", [ expression ], ";" ;

defer_statement =
    "defer",
    ( expression, ";"
    | statement_block ) ;
```

`return_statement` 和表达式式 `defer` 以真实分号结束；block 式 `defer` 由 `statement_block` 的 `}` 自行结束，不再写分号。`expression` 使用已经确认的完整表达式文法，`statement_block` 由议题 09 定义；换行与注释仍然只是 Trivia。

## 2. `return` 的两种形状

无结果返回和有结果返回分别写为：

```ink
return;
return value;
```

`return` 不要求在结果表达式外增加括号，普通括号表达式仍然合法：

```ink
return calculate();
return (calculate());
```

Parser 只根据 `return` 后的下一个显著 Token 是否为 `;` 判断可选表达式是否存在。换行不能隐式结束语句：

```ink
return
    calculate();
```

上例是一个有结果 `return_statement`。

函数声明省略 `-> type` 时，语义上固定规范化为 `-> void`，不会扫描函数体并根据 `return` 推导结果类型：

```ink
func log() {
    return;       // 合法，声明结果为 void
}

func calculate() {
    return 42;    // 语义错误，不会推导为 i32
}

func calculate() -> i32 {
    return 42;    // 合法
}
```

Parser 对三处 `return` 仍只按本节统一产生式建立 CST。声明是否省略结果、`return;` 或 `return expression;` 是否符合结果契约，均由语义阶段检查。构造函数和析构函数继续由生命周期语义禁止显式结果类型以及有值 `return`。

## 3. 元组和完整表达式边界

返回多个组成值时必须先构造一个元组表达式：

```ink
return (left, right);
return (value,);
return ();
```

逗号不是普通表达式运算符，因此以下形式非法：

```ink
return left, right; // 非法：元组缺少圆括号
```

`if_expression` 和其他完整表达式可以直接作为返回结果：

```ink
return if (ready) value else fallback;
```

结果表达式自身以 `}` 结束时也不能省略外层 `return_statement` 的分号。

## 4. 返回上下文和清理顺序

Parser 在允许 `statement` 的位置统一建立 `ReturnStatement`；当前是否位于可以返回的函数、装饰器或其他可执行体内，以及有无结果是否符合声明结果类型，属于后续语义检查。

`return expression;` 的表达式只求值一次。返回结果成功构造或稳定到调用方结果位置后，才按照议题 03 清理被离开的局部作用域：

```text
求值返回表达式
→ 成功建立返回结果
→ 逆序执行局部析构和 defer
→ 把控制权交还调用方
```

返回表达式触发 trap 时不会完成返回。直接构造到返回位置、命名值复制和不可复制类型限制继续遵守议题 02，不因 `return` 语法增加隐藏移动。

`return;` 与 `return ();` 不等价：前者没有结果表达式，后者返回议题 69 的空元组值。语义检查分别要求相应的 `void` 或 `()` 结果契约。

## 5. `defer` 的表达式与 block 形态

单项延迟清理使用表达式形式：

```ink
defer file.close();
defer log("leaving scope");
```

需要多条语句、局部声明、赋值或局部控制流时使用 block 形式：

```ink
defer {
    const path = file.path();
    file.close();
    log("closed", path);
}
```

block 形式的结束 `}` 自行结束 `defer_statement`，后面不能再写分号：

```ink
defer {
    cleanup();
}; // 非法：多余分号不是空语句
```

`defer;` 仍然非法，因为它既没有表达式，也没有 `statement_block`。

`return`、`break` 和 `continue` 是语句而不是表达式，不能直接作为表达式式 `defer` 的 body：

```ink
defer return;      // 非法
defer break;       // 非法
```

议题 11 的赋值也是专用语句，因此 `defer target = value;` 不符合表达式形式；需要赋值时写入 block：

```ink
defer {
    target = value;
}
```

## 6. 注册时不执行延迟内容

执行到任一 `defer_statement` 时只在当前作用域注册清理动作。整个表达式或 block 在该作用域实际清理时才执行；注册时不预先求值接收者、实参或其他子表达式，也不为它们建立隐藏副本：

```ink
var message = "start";
defer {
    log(message);
}
message = "finish";
```

离开作用域时上例读取当前的 `message`，因此记录 `"finish"`。需要快照时必须显式建立单独绑定：

```ink
const saved_message = message;
defer log(saved_message);
```

名称仍在 `defer` 源码位置完成普通静态解析；只是对相应绑定和值的实际访问延迟到清理动作执行时。因此 defer body 只能引用该语句处已经可见的外部名称，不能向后引用随后才声明的局部对象：

```ink
defer use(resource);        // 非法：resource 尚不可见
var resource = acquire();
```

这一普通可见性规则也保证 defer body 不会在对象已经提前析构后才通过隐藏捕获访问它。`defer` 不是运行时分配的闭包；该规则避免隐式复制不可复制资源，也与 C++ 风格引用和显式值复制保持一致。

## 7. 作用域、顺序和临时对象

每条已经由控制流到达的 `defer_statement` 在当前词法作用域注册一个动作。它与成功构造的局部对象共享议题 03 的后进先出清理栈：

```ink
defer log("A");
defer log("B");
```

离开作用域时先记录 `"B"`，再记录 `"A"`。不同 defer 动作按注册逆序执行；同一个 defer block 内部则按照普通语句的源码顺序执行：

```ink
defer {
    log("first");
    log("second");
}
```

上例依次记录 `"first"`、`"second"`。defer block 建立普通嵌套词法作用域；其中声明的局部对象在清理动作执行时构造，并在该 block 结束时逆序清理。嵌套 `defer` 若出现在 defer block 内，则在执行到它时注册到该 block 自己的清理栈。

从未执行到的外层 `defer` 不注册；循环每次迭代和嵌套 block 各自遵守对应作用域边界。

表达式形式产生的普通结果被丢弃。它在清理时形成一个完整表达式，其临时对象在该动作结束时逆序销毁，然后才继续执行清理栈中的下一个动作。

## 8. 清理动作的语义限制

`defer` 动作必须在清理过程中完整执行，不能暂停或把控制流转移到 body 之外。清理期间触发 trap 则立即终止，不保证继续执行剩余清理动作。

defer body 不能把控制流转移到该 body 之外：

- `return` 始终非法；
- `break` 或 `continue` 只能指向 defer block 内部声明的循环，不能指向注册点外层的循环；
- `await`、yield 或其他可能暂停清理过程的操作非法。

这些是 defer body 的语义检查。Parser 仍为 block 中出现的真实语句建立完整 CST，再由控制流检查报告非法外跳；它不根据调用目标或纯度改变 `DeferStatement` 的解析形状。

## 9. 两种结构都不是表达式

`return_statement` 和 `defer_statement` 都不能作为初始化器、实参或其他表达式的操作数：

```ink
const value = return 1;       // 非法
consume(defer cleanup());     // 非法
```

表达式式 `defer_statement` 保存一个 `expression` 子节点，但它不是普通 `expression_statement`：普通表达式语句立即求值，`defer` 表达式到作用域清理时才求值。block 式 `defer_statement` 保存一个 `statement_block`，同样整体延迟执行。只有表达式形式保留结尾分号；block 形式由 `}` 结束。

## 10. CST 形状与确定性解析

CST 至少使用以下节点：

```text
ReturnStatement
DeferStatement
```

`ReturnStatement` 按源码顺序保存 `return`、可选表达式、`;` 和全部 Trivia。`DeferStatement` 保存 `defer`，随后保存表达式与 `;`，或者保存完整 `StatementBlock`，并保留全部 Trivia。

`return` 与 `defer` 都是硬关键字，因此语句起始 Token 可以确定节点种类。`defer` 后的下一个显著 Token 是 `{` 时确定进入 block 形式，否则进入表达式形式；普通表达式不能以裸 `{` 开始，因此不需要回溯。Parser 也不能把 `defer expression;` 降格为带前缀的普通表达式语句。

## 11. 错误恢复

`return` 或表达式式 `defer` 缺少分号时继续使用议题 08 的零宽度 `MissingToken(';')` 恢复。`return` 后立即遇到 `;` 表示合法的无结果返回，而 `defer` 后立即遇到 `;` 必须建立缺失 body 节点：

```ink
return; // 合法
defer;  // 非法：缺少 expression 或 statement_block
```

block 形式缺少结束 `}` 时按照议题 09 插入零宽度 `MissingToken('}')`；完整 block 后的额外 `;` 必须保留为非法空语句 Token。`return left, right;` 中真实的逗号必须保存在 `ErrorNode` 或交还外层同步逻辑，不能静默改写成元组。

## 12. 确认结论

Ink 使用 `return;` 和 `return expression;` 表示函数返回，结果表达式只求值一次，成功建立返回结果后再执行离开路径上的逆序清理。函数声明省略结果类型固定表示 `void`，不从函数体推导。元组结果必须显式写圆括号。延迟清理同时支持 `defer expression;` 和 `defer { ... }`；整个表达式或 block 在作用域清理时执行，注册时不建立接收者或实参快照。defer block 可以容纳多条普通语句，但不得让返回、外层循环跳转或暂停越过清理边界。`return`、表达式式 `defer` 和 block 式 `defer` 都拥有确定的 CST 与错误恢复规则。
