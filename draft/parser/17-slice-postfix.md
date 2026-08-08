# Parser 议题 17：切片后缀

> 状态：已确认，议题 25 增加仅限 `for` 头的 `..` 半开边界；议题 30 将空 `[]` 统一为类型构造后缀
> 确认日期：2026-08-03

## 1. 使用冒号分隔边界

Ink 使用 `:` 表示切片后缀，不使用 `..`：

```ink
values[:]          // 全部元素
values[start:]     // 从 start 到末尾
values[:end]       // 从开头到 end
values[start:end]  // 从 start 到 end
```

切片后缀加入议题 15、16 的最高优先级后缀集合：

```ebnf
postfix_suffix =
      ordinary_postfix_suffix
    | generic_argument_suffix
    | slice_suffix ;

slice_suffix =
    "[",
    (
        ":", [ expression ]
      | expression, ":", [ expression ]
    ),
    "]" ;
```

`ordinary_postfix_suffix` 由议题 15 定义，`generic_argument_suffix` 由议题 16 定义。

## 2. 四种合法形状

切片准确支持四种边界组合：

```text
[:]          start = 0,        end = length
[start:]     start = supplied, end = length
[:end]       start = 0,        end = supplied
[start:end]  start = supplied, end = supplied
```

省略边界不会构造隐藏的用户表达式，也不会执行额外用户代码；它直接使用基础对象的零起点或当前长度。

## 3. 左闭右开

切片范围采用左闭右开语义：

```ink
values[1:4]
```

结果包含索引 `1`、`2`、`3`，不包含索引 `4`。结果长度为 `end - start`；`start == end` 产生合法空切片。

这种边界约定使完整切片自然写成 `[0:length]`，并使相邻区间可以无重叠地写成 `[a:b]` 和 `[b:c]`。

## 4. 与普通索引区分

方括号中不存在冒号时，使用议题 15 的普通索引后缀；存在顶层冒号时，使用切片后缀：

```ink
values[index]      // 单元素索引
values[start:end]  // 子切片
values[:]          // 完整切片
```

空方括号不属于索引或切片后缀：

```ink
values[] // 语法上是 EmptyBracketTypeSuffix
```

议题 30 要求 Parser 把该形状建立为类型构造尾链中的 `EmptyBracketTypeSuffix`，不根据左侧名称或当前可知类型改变 CST。如果 `values` 求值为 `type`，整体产生安全切片类型；如果它是普通数组、切片或其他非 `type` 值，语义阶段报告类型构造的左侧值必须为 `type`。Parser 不得把它改报为“缺少索引”或“缺少冒号”。

以下形式仍然非法：

```ink
values[1..4] // Ink 不使用 ".." 切片
```

`:` 是一个单独的 Symbol Token，不存在复合符号邻接问题。其两侧可以保留普通 Trivia：

```ink
values[start : end]
```

## 5. 不提供步长和包含式结束边界

Ink v0 不提供 Python 风格的第三个步长位置：

```ink
values[start:end:step]
```

该形式非法。语言也不提供 `..=` 或其他“包含 end”切片拼写。反向遍历、跨步访问和包含式范围由显式循环、迭代器或标准库视图表达，不改变基础切片的两个边界模型。

## 6. 合法基础类型

切片后缀可以应用于固定长度数组和安全切片：

```ink
array[start:end]
slice[start:end]
```

结果是一个新的非拥有安全切片，不复制元素。结果继承原访问路径的元素可写性：可写数组或切片产生可写 `T[]`，只读访问路径产生 `const T[]`。

原始指针不携带长度或生命周期证明，不能通过该语法直接产生安全切片：

```ink
pointer[start:end] // 类型错误
```

需要地址加长度的底层表示时，必须通过已经定义的显式原始切片构造或标准库 API 建立 `RawSlice::<T>`，不能借切片后缀伪造安全生命周期。

基础类型要求属于类型检查；Parser 对任何语法正确的左侧表达式建立相同 `SliceExpression` CST。

## 7. 边界检查

运行时必须满足：

```text
start <= end <= length
```

任一条件不成立时产生安全越界 trap。边界在编译期可证明合法时，编译器可以消除对应检查；可证明非法时可以按照既有 trap 和诊断规则处理，但不能产生越界安全切片。

省略的 `start` 固定为零，因此不需要单独检查下界。边界表达式的准确整数类型和普通整数转换规则由索引类型语义统一规定，不影响 Parser 文法。

## 8. 求值顺序

切片按照议题 13 从左到右求值：

```ink
make_values()[make_start():make_end()]
```

顺序为：

```text
make_values()
→ make_start()
→ make_end()
→ 检查边界
→ 构造切片值
```

省略某一边界时跳过对应表达式。较早步骤抛出异常时，尚未开始的后续边界不会执行。

## 9. `..` 不成为范围运算符

采用冒号切片不会为普通表达式引入 `..`。Ink v0 没有内建范围表达式、范围值或相关运算符优先级。

议题 25 仅在 `for (binding_mode pattern in start .. end) { ... }` 头部把 `..` 定义为两个边界之间的专用半开定界符。它不会产生可以保存、传参或参与其他表达式的 Range 值，也不能用于切片。未来如果需要通用 Range 类型，仍须单独设计其值语义和表达式文法。

## 10. CST 与恢复

`SliceExpression` CST 保存基础表达式、左右方括号、冒号、实际存在的边界表达式和所有 Trivia。省略边界通过缺少对应可选子节点表示，不插入虚假的数字 Token。空 `[]` 则保存为议题 30 的 `EmptyBracketTypeSuffix`，不建立 `SliceExpression`。

如果出现第二个顶层冒号、缺少右方括号或边界表达式语法错误，Parser 按议题 03 使用 `ErrorNode`、`MissingToken` 和方括号同步边界恢复，不能把错误切片静默改成普通索引。

## 11. 确认结论

Ink 使用 `[:]`、`[start:]`、`[:end]` 和 `[start:end]` 四种左闭右开切片后缀，不使用 `..`。空 `[]` 在语法上是 `EmptyBracketTypeSuffix`，左侧值非 `type` 时由语义阶段报错。切片只适用于数组和安全切片，继承元素可写性，边界违反 `start <= end <= length` 时 trap；原始指针不能借此建立安全切片。议题 25 的 `for` 专用 `start .. end` 不改变本节切片语法，v0 仍不提供步长、包含式结束边界或通用范围运算符。
