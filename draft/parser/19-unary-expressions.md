# Parser 议题 19：一元表达式

> 状态：已确认，议题 30 补充表达式末尾的类型构造含义；2026-08-05 增加 `comptime` 表达式前缀，议题 32 与结构化区域控制统一阶段模型
> 确认日期：2026-08-03

## 1. 文法

一元表达式使用右递归标准 EBNF：

```ebnf
unary_expression =
      postfix_expression
    | unary_operator, unary_expression
    | comptime_expression ;

comptime_expression =
    "comptime", unary_expression ;

unary_operator =
    "+" | "-" | "!" | "~" | "*" | "&" | "await" ;
```

所有非终结符遵守议题 04，使用不含空格的 `snake_case` 名称。

右递归使连续一元运算从右侧组合：

```ink
!*pointer     // !(*pointer)
~-value       // ~(-value)
&*pointer     // &(*pointer)
```

每一层运算符都必须分别满足自己的语义要求。

## 2. 一元正号

`+value` 是数值正号，结果数值与操作数相同：

```ink
const value = +input;
```

它只适用于语义支持一元正号的数值类型。Parser 不根据操作数类型决定是否合法。

正号不属于数值字面量 Token：

```ink
+42
```

Tokenizer 产生 `Symbol('+')` 和 `IntegerLiteral("42")`，Parser 再建立一元表达式。

## 3. 一元负号

`-value` 表示数值取负：

```ink
const negative = -value;
```

负号同样不属于数值字面量 Token。固定宽度整数取负继续遵守议题 07 的按类型位宽回绕语义，包括最小有符号值和无符号整数。

## 4. 逻辑非

`!value` 表示布尔逻辑非，只接受 `bool` 并返回 `bool`：

```ink
if (!ready) {
    wait();
}
```

Ink 不把整数、浮点数、指针、对象或其他值隐式转换为布尔真值：

```ink
!pointer // 类型错误，即使 pointer 可以与 null 比较
!1       // 类型错误
```

指针条件必须明确写出比较：

```ink
pointer != null
```

## 5. 按位非

`~value` 表示逐位取反，只接受支持该操作的整数类型：

```ink
const inverted = ~mask;
```

`bool` 不使用按位非；布尔值使用 `!`。结果保持操作数的具体整数类型和位宽。

## 6. 解引用

`*pointer` 解引用原始指针，并产生指向目标存储的 place：

```ink
const value = *pointer;
*pointer = replacement;
```

结果的可读、可写能力继承指针目标类型。`const T*` 解引用得到只读 place，`T*` 解引用得到类型允许的可写 place。

Parser 只识别前缀 `*`。操作数是否为可解引用指针、地址是否非空、有效、对齐且处于对象生命周期内，由类型与原始内存访问语义决定。违反原始指针解引用前置条件继续按照既有安全模型处理，而不是 Parser 错误。

## 7. 取地址的语法与语义分层

取地址在语法上接受任意 `unary_expression`：

```ebnf
unary_expression =
      postfix_expression
    | "&", unary_expression
    | (* other unary operators *) ;
```

Parser 不把 `&` 的操作数限制成语法非终结符 `place_expression`，因为一个表达式是否产生 place 可能依赖名称绑定、函数返回类型、重载选择和值类别。

Parser 对下列源码都建立取地址一元节点：

```ink
&variable
&make_object()
&(left + right)
&42
```

它们是否合法由后续语义阶段判断，不能由 Parser 根据表面形状提前拒绝。

## 8. 内建取地址要求 place

内建 `&expression` 要求操作数完成名称绑定和类型检查后属于 place expression，也就是表示一个具有有效存储位置和生命周期的对象位置。

典型合法形式包括：

```ink
&variable
&object.field
&array[index]
&tuple.0
&*pointer
```

调用表达式是否可取址取决于最终结果类别。Ink 允许普通函数返回引用：

```ink
func get_object() -> Object&;
func get_const_object() -> const Object&;
func make_object() -> Object;

&get_object()       // 合法，结果为 Object*
&get_const_object() // 合法，结果为 const Object*
&make_object()      // 语义错误：调用产生按值临时对象
```

同理，运算结果若合法产生引用 place，可以取址；普通按值算术结果不能取址。Parser 不需要知道重载解析后是否选择了返回 `T&` 的用户运算符。

## 9. 禁止对按值临时对象取址

按值临时结果不满足内建取地址的 place 要求：

```ink
&make_object() // make_object() -> Object 时非法
&(a + b)       // 普通按值算术结果非法
&42            // 字面量非法
```

Ink 不通过取地址隐式物化一个只活到当前完整表达式结束的临时对象，也不为该指针进行临时生命周期延长。这避免产生在分号之后立即悬空的原始指针：

```ink
const pointer = &make_object(); // 不允许建立这种立即悬空指针
```

需要地址时先建立具有明确词法生命周期的绑定：

```ink
const object = make_object();
consume(&object);
```

绑定是否可重新赋值与通过所得指针能否修改目标是不同能力，继续由绑定和目标类型规则决定。

## 10. `&` 与 `*` 的上下文含义

Tokenizer 始终产生单字符 Symbol Token。Parser 根据一元、二元或议题 30 的终止型类型构造位置区分同一符号：

```ink
&value     // 一元取地址
left & right // 二元按位与
return T&; // 当前子表达式末尾的引用类型构造

*pointer   // 一元解引用
left * right // 二元乘法
return T*; // 当前子表达式末尾的指针类型构造
```

类型构造候选不能到达当前表达式结束符时，Parser 回滚局部后缀试探并继续按本节的一元或议题 12 的二元运算解析。例如 `T**pointer` 解析为 `T * (*pointer)`。不需要为三种拼写建立不同 TokenKind，也不查询 `T` 是否确实表示类型。

普通上下文首先遵守议题 02 的全语言最长匹配，因此相邻 `&&` 通常是一个逻辑与序列。`terminal_type_constructor_tail` 是受限覆盖：它可以在 checkpoint 内把 `&&` 暂时逐字符观察为两个引用类型后缀，但只有最大尾链到达调用者 EndSet 时才提交：

```ink
return T&&;      // 提交 ReferenceTypeValue(ReferenceTypeValue(T))
return T&&value; // 试探失败并回滚，按 T && value 解析
```

尾链试探不得拆开 `*=`、`&=` 等赋值复合终端。试探失败必须同时撤销游标、临时 CST 和诊断，再恢复全语言最长匹配。

## 11. `await`

`await task` 是一元表达式，并使用与其他一元运算相同的右侧操作数结构：

```ink
const value = await task;
```

其任务启动、暂停、异常传播、取消和结果复制继续遵守既有异步议题。本节只确定它在表达式文法中的前缀位置和优先级。

## 12. `comptime` 表达式前缀

`comptime` 直接修饰一个 `unary_expression`，要求该操作数在编译期完成求值：

```ink
const value = comptime calculate();
const object = comptime TypeName(arguments);
const member = comptime object.make_value();
```

函数调用、构造调用和成员调用都是 postfix expression，因此不需要额外的“编译期调用”产生式。CST 使用专用 `ComptimeExpression` 节点保存 `comptime` Token 与右侧表达式；是否能在编译期完成属于语义分析和 Partial Evaluation，不由 Parser 判断。议题 32 把它视为统一阶段模型的 `ValueRegion` 形式；字段、局部和 module 初始化器都复用当前节点，类中不另建专用表达式节点。

该前缀与其他一元结构具有相同的局部结合范围：

```ink
comptime f() + g()       // (comptime f()) + g()
comptime (f() + g())     // 整个加法必须在编译期完成
```

最低优先级的 `if_expression` 或其他需要完整表达式边界的结构，可以通过圆括号明确作为操作数：

```ink
const selected = comptime (if (condition) first else second);
```

`comptime if (condition) { ... }`、`comptime match (value) { ... }`、`comptime for (...) { ... }` 和 `comptime while (condition) { ... }` 是语句上下文中的结构化控制形式，不由本节的 `comptime_expression` 吞并。

## 13. 不支持自增与自减

议题 02、11 已经把相邻 `++`、`--` 注册为保留的非法复合符号序列。它们不属于 `unary_operator`：

```ink
++value
--value
value++
value--
```

全语言最长匹配要求 Parser 先识别完整 `++` 或 `--`，随后直接产生语法诊断；一元表达式不能消费第一个 `+` 或 `-` 后再把第二个解释为下一层一元运算，普通后缀表达式也不能消费其中任何前缀。

Trivia 会打断复合符号序列，所以有意书写的嵌套一元运算仍然合法：

```ink
+ +value // +(+value)
- -value // -(-value)
```

递增和递减继续使用无结果复合赋值语句：

```ink
value += 1;
value -= 1;
```

## 14. Parser、CST 与恢复

一元 CST 保存运算符的真实 Token 和右侧完整 `unary_expression`。连续一元运算可以保存为嵌套节点，也可以保存为有序前缀列表后在 lowering 时右结合；两者必须产生相同 AST。

Parser 不在 CST 中记录“可取址”“可写”“指针有效”或“操作数是 bool”等语义结论。缺少右操作数时，按照议题 03 使用 `MissingToken` 或 `ErrorNode` 恢复。

## 15. 确认结论

Ink 的普通一元运算符为 `+`、`-`、`!`、`~`、`*`、`&` 和 `await`；`comptime` 在同一优先级层建立专用 `ComptimeExpression`。相邻 `++`、`--` 按全语言最长匹配形成保留非法序列，不能拆成嵌套一元运算；需要连续应用一元正负号时必须用 Trivia 分开。函数调用已经是表达式，因此 `comptime f()` 不需要特殊调用语法；需要扩大强制求值范围时使用圆括号。议题 30 在当前子表达式末尾为 `*`、`&` 增加类型构造含义：完整最大尾链到达 EndSet 时可以受限地逐字符提交，否则必须事务性回滚到本节和议题 12 的全局最长匹配；赋值复合终端不可拆分。Parser 对 `&unary_expression` 只建立语法节点；内建取地址在语义阶段要求操作数最终属于 place。`T&` 或 `const T&` 表达式结果都可以取址；按值临时对象不能取址且不会获得隐式生命周期延长。普通函数允许返回引用，但 Ink 不保证返回引用的目标仍然有效。
