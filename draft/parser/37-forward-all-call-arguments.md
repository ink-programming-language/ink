# Parser 议题 37：调用中的裸 `...` 全参数转发

> 状态：已确认
> 确认日期：2026-08-05

## 1. 调用实参入口

普通调用后缀增加裸 `...` 形成的全参数转发列表：

```ebnf
call_suffix =
    "(", [ call_argument_sequence ], ")" ;

call_argument_sequence =
      argument_list
    | forward_all_arguments ;

forward_all_arguments =
    "..." ;
```

```ink
function(...)
```

`forward_all_arguments` 是完整实参序列，不是普通实参列表中的一个元素。

## 2. 不与其他实参混写

裸 `...` 必须单独占据整个调用实参列表：

```ink
function(...)
```

下列形式不属于该产生式：

```ink
function(prefix, ...)
function(..., suffix)
function(..., name = value)
```

需要显式传入修改后的参数时，使用既有普通 `argument_list`，不同时保留隐式全参数转发。

## 3. 与列表展开的区别

现有列表展开继续要求 `...` 后存在表达式：

```ebnf
list_expansion =
    "...", expression ;
```

```ink
target(...values)
```

因此两种结构由右括号前是否立即结束确定：

```ink
target(...)       // forward_all_arguments
target(...values) // argument_list 中的 list_expansion
```

## 4. 适用边界

`call_suffix` 不根据调用目标的名称改变语法结构，所以任意普通调用目标后都可以形成该节点：

```ink
target(...)
object.method(...)
```

attribute application 和 decorator application 仍直接复用 `argument_list`，不复用 `call_argument_sequence`，因此它们不接受裸 `...`：

```ink
[reflect(...)]
@trace(...)
```

上述两个 application 不属于对应声明前缀文法。

## 5. 结论

裸 `...` 是普通调用后缀的一种完整实参序列，不能和显式位置实参、命名实参或列表展开混写。它与 `...expression` 列表展开保持不同语法节点；调用目标是否允许全参数转发不改变 Parser 结构。
