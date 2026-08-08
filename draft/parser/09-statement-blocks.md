# Parser 议题 09：语句块与花括号

> 状态：已确认，议题 20、24—28 补充全部控制流、清理与异常处理规则；2026-08-05 增加 `comptime` block statement，Parser 议题 32 统一区域控制；2026-08-06 汇总完整 `local_declaration` 与 `statement` 产生式；2026-08-08 确认声明与 `match` 结构在 block item 起点确定性提交
> 确认日期：2026-08-03

## 1. 普通语句块

普通运行时语句块使用花括号：

```ebnf
statement_block =
    "{", { block_item }, "}" ;

block_item =
    local_declaration
  | statement ;

local_declaration =
    binding_declaration ;

statement =
    block_statement
  | comptime_block_statement
  | assignment_statement
  | expression_statement
  | if_statement
  | comptime_if_statement
  | match_statement
  | comptime_match_statement
  | while_statement
  | comptime_while_statement
  | for_statement
  | comptime_for_statement
  | break_statement
  | continue_statement
  | return_statement
  | defer_statement
  | throw_statement
  | try_statement ;
```

`local_declaration` 当前只复用议题 10 的 `binding_declaration`，即以 `var` 或 `const` 开头的普通局部绑定。各个 statement 子产生式分别由议题 09、11、18、20、24—28 定义；这里仅汇总已经确认的规则，不引入新的语句形态。空语句块 `{}` 合法。

`block_item` 的分派使用忽略 Trivia 的有限 Token 前瞻：看到 `Keyword(Var)` 或 `Keyword(Const)` 时立即提交到 `local_declaration`，即使后续声明残缺也不能回退成赋值语句或表达式语句。其他开头进入 `statement`。只允许 `statement` 而不允许 `block_item` 的位置若遇到这两个声明关键字，应报告“声明必须放入语句块”并按声明形状恢复，不能把同一 Token 序列重新解释成表达式。

语句入口看到裸 `Keyword(Match)` 时立即且不可回滚地提交到 `match_statement`；看到连续两个显著 Token `Keyword(Comptime)`、`Keyword(Match)` 时同样立即提交到 `StatementRegion` 的 `ComptimeMatchControl`。后续 arm 的逗号、分号或语法错误只用于校验和恢复，不得改变节点种类。`(match (...) { ... });` 与 `comptime (match (...) { ... });` 分别以 `(` 和 `comptime (` 开始，因此进入普通表达式语句。

赋值语句与表达式语句共享其余表达式前缀，按照议题 11 在解析完左侧表达式后由紧随的赋值运算符分流。其他 `comptime` 开头的结构继续按照议题 32 通过有限前瞻分派 block、if、while 或 for；未命中保留结构起点时仍可进入含 `comptime_expression` 的普通表达式语句。

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

comptime_block_statement =
    "comptime", statement_block ;
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

在 block 前添加 `comptime`，要求整个块在编译期执行：

```ink
comptime {
    const table = make_table();
    validate(table);
}
```

块中的绑定、赋值、`try`、`defer`、`return`、`break`、`continue` 和 `throw` 继承该执行上下文，不分别增加 `comptime var`、`comptime return` 等语法。Parser 仍按普通 `statement_block` 解析内部项目，并建立 Parser 议题 32 的统一 `ComptimeBlockControl`，其区域为 `StatementRegion`。

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

函数通过显式 `return` 返回值。议题 21 的 `if_expression` 使用 `if (...) ... else ...`，本身不使用花括号。聚合初始化拥有议题 35 的表达式专用 postfix 后缀和专用 CST 节点；类型构造及其他可能使用花括号的表达式同样拥有各自规则，不会把普通语句块自动变成有值 block expression。

议题 24 允许 `match_expression` 的分支使用 `statement_block`，但仅用于不能正常完成的分支。该块仍不产生值；如果它能到达结束 `}`，对应有值分支属于语义错误。

## 5. 控制流必须使用花括号

使用语句体的 `if`、`while`、`for` 及后续同类控制结构必须接收完整 `statement_block`，不能直接接收单条无花括号语句：

```ink
if (condition) {
    run();
}

while (condition) {
    update();
}
```

以下形式非法：

```ink
if (condition)
    run();

while (condition) update();
```

该规则避免悬空 `else`，并保证增加第二条语句时不会静默改变控制范围。

议题 25 进一步确认 `while (...)`、`while (match ...)` 和普通 `for (...)` 都使用该形状，不接受单语句省略花括号，也不支持循环 `else`。

## 6. `else if`

`else if` 通过 `else` 后嵌套另一个完整 `if_statement` 支持：

```ebnf
if_statement =
    "if", "(", if_condition, ")", statement_block,
    [ "else", ( statement_block | if_statement ) ] ;
```

```ink
if (first) {
    a();
} else if (second) {
    b();
} else {
    c();
}
```

这里每个 `if` 的执行体仍然具有花括号；只有 `else` 与下一个 `if` 之间不额外增加一层块。

议题 20 已确认固定条件括号，并允许括号内使用普通 `expression` 或 `match_condition`。本节 EBNF 只确认 block 形状和 `else` 归属，完整规则以议题 20 为准。

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
TopLevelBlock          顶层 comptime if 中的导入和顶层声明
TypeDeclarationBody    class、interface、enum 等成员
InitializerBody        聚合或命名字段初始化
```

每种结构的内部元素由自己的 EBNF 决定。Parser 不能因为定界符相同就允许顶层导入出现在普通运行时块，也不能把类型成员体当成普通语句块。

该区别同样适用于 `comptime { ... }`。外层 Parser 调用位置在消费 `comptime` 前就已经确定当前 item context：

```text
function/local statement context  → RegionKind::Statement + StatementBlock
module declaration context        → RegionKind::TopLevel + TopLevelBlock
class member context              → RegionKind::ClassMember + ClassMemberBlock
interface member context          → RegionKind::InterfaceMember + InterfaceMemberBlock
enum member context               → RegionKind::EnumMember + EnumMemberBlock
```

这些都是同一个 `ComptimeBlockControl` 对不同 `RegionRules` 的实例，不是多套 `comptime` 语义。因此相同的 `{` Token 不需要通过检查第一个内部 item 或名称绑定结果消歧。各上下文提供自己的 item parser 和错误同步集合；CST 必须保留准确的 block kind。

## 9. CST 与错误恢复

语句块 CST 保留开始和结束花括号以及内部全部 Trivia。如果缺少结束 `}`，Parser 可以在适当同步位置按照议题 03 插入零宽度 `MissingToken('}')`；真实的意外结束符必须保存在 `ErrorNode` 或返回给外层结构处理。

独立块、控制流体和函数体都产生明确 `StatementBlock` 节点，使格式化器、重构工具和增量 Parser 可以按同一运行时块结构处理。

## 10. 确认结论

Ink 普通语句块使用 `{ ... }`、建立词法作用域且不产生表达式值。允许独立 block statement 控制 RAII 与 `defer` 生命周期，也允许 `StatementRegion` 的 `comptime { ... }` 要求完整块在编译期执行；块内其他语句继承该上下文，不各自增加阶段前缀。相同源码前缀位于 declaration region 时由 Parser 议题 32 的统一入口接收不同 `RegionRules`，直接建立对应 declaration block，不能混入普通 statement。所有使用语句体的控制结构必须带花括号。结束 `}` 自身结束 block，不再写分号；其他声明体和初始化体即使使用相同定界符，也由各自语法定义。
