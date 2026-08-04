# Parser 议题 09：语句块与花括号

> 状态：已确认，议题 20、24—26、28 补充控制结构、清理与异常处理规则
> 确认日期：2026-08-03

## 1. 普通语句块

普通运行时语句块使用花括号：

```ebnf
statement_block =
    "{", { block_item }, "}" ;

block_item =
    local_declaration
  | statement ;
```

`local_declaration` 和各类 `statement` 的完整产生式在后续议题中定义。空语句块 `{}` 合法。

换行和缩进仍然是 Trivia；只有真实的 `Symbol('{')` 与 `Symbol('}')` 建立和结束语句块。

## 2. 词法作用域

每个 `statement_block` 建立一个新的词法作用域。块内声明的局部名称在结束 `}` 后不可见：

```ink
{
    const value = calculate();
    use(value);
}

// value 在这里不可见
```

嵌套块可以遮蔽外层名称还是必须拒绝，将在名称绑定与局部声明议题中确定；本议题只确认块边界形成作用域。

议题 28 确认 `try` body 和每个 `catch` body 都是独立 `statement_block`。`try` body 的局部名称不进入处理器作用域；捕获绑定只在对应处理器 block 内可见，也不进入后续处理器。

## 3. RAII 与 `defer`

单独的语句块可以作为 block statement 出现在另一个语句块中：

```ebnf
block_statement =
    statement_block ;
```

这允许程序显式缩短局部资源和 `defer` 的作用域：

```ink
func process() {
    {
        var file = File.open("data.txt");
        defer log("file scope ended");
        use(file);
    }

    continue_work();
}
```

正常到达 `}`、跳转离开块或异常展开离开块时，局部对象析构和 `defer` 遵守议题 03 已确认的逆序清理规则。

议题 26 确认 `defer` 同时接受 `defer expression;` 和 `defer { ... }`。两种形态都注册到当前 `statement_block` 的清理栈，并在真正离开该作用域时执行；defer block 自身也建立一个嵌套词法作用域。

## 4. Block 不是表达式

普通 `statement_block` 是语句结构，不是表达式，不产生隐式结果。块中最后一个表达式仍必须作为表达式语句以 `;` 结束：

```ink
{
    calculate();
}
```

以下形式不能把普通块的最后一个表达式作为值返回：

```ink
const value = {
    calculate()
};
```

函数通过显式 `return` 返回值。议题 21 的 `if_expression` 使用 `if ... then ... else ...`，本身不使用花括号。聚合初始化、类型构造或其他可能使用花括号的表达式拥有各自独立产生式和专用 CST 节点，不会把普通语句块自动变成有值 block expression。

议题 24 允许 `match_expression` 的分支使用 `statement_block`，但仅用于不能正常完成的分支。该块仍不产生值；如果它能到达结束 `}`，对应有值分支属于语义错误。

## 5. 控制流必须使用花括号

使用语句体的 `if`、`while`、`for` 及后续同类控制结构必须接收完整 `statement_block`，不能直接接收单条无花括号语句：

```ink
if condition {
    run();
}

while condition {
    update();
}
```

以下形式非法：

```ink
if condition
    run();

while condition update();
```

该规则避免悬空 `else`，并保证增加第二条语句时不会静默改变控制范围。

议题 25 进一步确认 `while`、`while match` 和普通 `for` 都使用该形状，不接受单语句省略花括号，也不支持循环 `else`。

## 6. `else if`

`else if` 通过 `else` 后嵌套另一个完整 `if_statement` 支持：

```ebnf
if_statement =
    "if", expression, statement_block,
    [ "else", ( statement_block | if_statement ) ] ;
```

```ink
if first {
    a();
} else if second {
    b();
} else {
    c();
}
```

这里每个 `if` 的执行体仍然具有花括号；只有 `else` 与下一个 `if` 之间不额外增加一层块。

议题 20 已确认条件不强制括号，并允许普通 `expression` 或 `match_condition`。本节 EBNF 只确认 block 形状和 `else` 归属，完整规则以议题 20 为准。

## 7. Block 自身结束结构

结束 `}` 已经结束 block statement。按照 Parser 议题 08，后面不写额外分号，单独的 `;` 也不是空语句：

```ink
{
    run();
}  // 合法

{
    run();
}; // 非法
```

## 8. 不同花括号结构

以下结构虽然都使用 `{` 和 `}`，但不是同一个 CST 节点种类：

```text
StatementBlock         运行时语句和局部声明
TopLevelBlock          顶层 if comptime 中的导入和顶层声明
TypeDeclarationBody    class、interface、enum 等成员
InitializerBody        聚合或命名字段初始化
```

每种结构的内部元素由自己的 EBNF 决定。Parser 不能因为定界符相同就允许顶层导入出现在普通运行时块，也不能把类型成员体当成普通语句块。

## 9. CST 与错误恢复

语句块 CST 保留开始和结束花括号以及内部全部 Trivia。如果缺少结束 `}`，Parser 可以在适当同步位置按照议题 03 插入零宽度 `MissingToken('}')`；真实的意外结束符必须保存在 `ErrorNode` 或返回给外层结构处理。

独立块、控制流体和函数体都产生明确 `StatementBlock` 节点，使格式化器、重构工具和增量 Parser 可以按同一运行时块结构处理。

## 10. 确认结论

Ink 普通语句块使用 `{ ... }`、建立词法作用域且不产生表达式值。允许独立 block statement 控制 RAII 与 `defer` 生命周期；所有使用语句体的控制结构必须带花括号。结束 `}` 自身结束 block，不再写分号；其他声明体和初始化体即使使用相同定界符，也由各自语法定义。
