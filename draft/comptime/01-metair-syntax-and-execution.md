# Comptime 议题 01：MetaIR 语法与编译期执行模型

> 状态：本轮讨论确认快照
>
> 记录日期：2026-08-29
>
> 范围：独立实验性 MetaIR，不修改仓库现有 Ink 语言设计

## 1. 目标与基本约束

这套 MetaIR 使用接近源码的文本形式表达普通程序、编译期执行和语法生成，同时优先保证语法完全无二义性并可按 LL(1) 方式解析。

当前约束为：

- 函数调用、类型构造、结构化编译期控制和运算符在语法上显式区分；
- 泛型实参不做隐式推导，所有无法由默认值补齐的泛型实参都必须显式提供；
- `<` 与 `>` 只承担泛型结构边界，不兼任比较或移位运算符；
- 表达式运算符统一使用 `builtin.xxx`，结构标点继续使用固定标点；
- 所有宏分支都必须通过词法与语法检查，是否进行名称绑定和类型检查由控制结构的类别决定；
- 语法合法与语义合法分离，Parser 不替后续阶段判断构造函数是否存在、名称是否可见或类型是否匹配。

`builtin` 是保留名称，用户声明不能遮蔽它。本文统一使用 `builtin`，不再使用此前误写的 `bulitin` 或 `build`。

## 2. 完整样例

```text
private class MyClass<T:type = int32, V:type>
{
	func Foo<M:type, N:type>(A:int32, B:int32)->Vector<T>
	{
		return Vector<T>(A,B);
	}

	field std::map<T> Maps;

	builtin.macro_if(T is int32)
	{
		func Foo1()
		{
			if(a builtin.equal b)
			{
			}
		}
	}
	else
	{
		func Foo2()
		{
		}
	}

	builtin.macro_if(call builtin.compiler_parameter<target_os>(os.target) builtin.equal window)
	{
		var Times:int32 builtin.assign comptime call get_times();

		builtin.comptime_for(Count:int32 builtin.assign 1; Count builtin.less Times; Count builtin.addassign 1)
		{
			field std::vector<T> call builtin.pastesymbol(map, call builtin.value_as_symbol(Count));
		}
	}
};

builtin.macro_if(call builtin.compiler_parameter<target_os>(os.target) builtin.equal window)
{
	private var Test:int32 builtin.assign 1;
}
```

该样例主要展示语法和阶段规则，不承诺 `std::map<T>`、`Vector<T>(A,B)`、`a` 或 `b` 在某个具体程序中一定通过后续语义检查。

## 3. 泛型声明与实参绑定

泛型形参统一写成 `Name:type`，函数泛型同样遵守该形式：

```text
class MyClass<T:type = int32, V:type>
func Foo<M:type, N:type>(A:int32, B:int32)->Vector<T>
```

泛型默认值使用结构性 `=`。带默认值的形参可以出现在必填形参之前，因此以下声明合法：

```text
class MyClass<T:type = int32, V:type>
```

泛型实参支持位置绑定和名称绑定：

```text
MyClass<int64, string>
MyClass<V = string>
MyClass<T = int64, V = string>
```

绑定规则为：

1. 位置实参从左到右绑定；
2. 名称实参按形参名绑定；
3. 位置实参必须全部位于名称实参之前；
4. 名称实参之后不能再出现位置实参；
5. 同一形参不能被重复绑定；
6. 未显式绑定的形参使用其默认值；
7. 没有默认值且未绑定的形参产生语义错误。

MetaIR 不从普通函数实参、返回目标或函数体使用方式推导泛型实参。例如 `Foo` 的 `M` 和 `N` 没有默认值时，调用者必须显式提供它们。

泛型列表中的 `=` 不是赋值表达式。Parser 在泛型实参位置看到 `Identifier =` 后直接提交到名称实参分支；其他起始形式进入位置实参分支，因此无需回溯。

## 4. 函数调用与类型构造

函数调用必须带 `call`：

```text
call Foo<int32, string>(A, B)
call get_times()
```

没有 `call`，而是由类型表达式直接跟随实参列表时，表示类型构造：

```text
Vector<T>(A, B)
```

因此不增加 `construct` 关键字。`call` 已经在首个语法位置区分函数调用和类型构造，这一分流满足 LL(1) 解析需要。

在普通上下文中，`comptime call` 强制函数调用在编译期完成：

```text
var Times:int32 builtin.assign comptime call get_times();
```

进入 `builtin.comptime_if` 或 `builtin.comptime_for` 的编译期执行上下文后，内部普通 `call` 自然由编译期解释器执行，不要求每次重复写 `comptime`。

## 5. 普通调用的命名实参

`builtin.assign` 在普通调用实参表的特定语法位置表示名称绑定：

```text
call Foo(A builtin.assign 1);
call Foo(1, B builtin.assign 2);
call Foo(A builtin.assign 1, B builtin.assign 2);
```

第一行是合法命名传参，含义是把值 `1` 绑定给形参 `A`，不会修改调用者作用域中名为 `A` 的变量。

调用实参同样要求位置实参在名称实参之前，禁止重复绑定，也禁止在名称实参之后继续给出位置实参。实参表达式按源码顺序求值。

这不会与赋值语句产生歧义：

- 在调用实参起点，`Identifier builtin.assign` 提交到命名实参；
- 在语句位置，`Target builtin.assign Value;` 表示赋值；
- 赋值操作本身不产生值，不能作为普通位置实参或参与赋值链。

如果确实需要先修改变量再传值，必须拆成两个操作：

```text
A builtin.assign 1;
call Foo(A);
```

## 6. `builtin` 操作分类

`builtin` 成员固定分成四类；类别决定其语法形态，不能混用。

| 类别 | 形式 | 示例 |
| --- | --- | --- |
| 可调用值操作 | 必须使用 `call` | `call builtin.value_as_symbol(Count)` |
| 结构化控制 | 自带控制头和代码块，不使用 `call` | `builtin.macro_if(...) { ... }` |
| 表达式运算符 | 前缀或中缀运算符形式 | `Value builtin.less Limit` |
| 编译期控制语句 | 独立语句 | `builtin.comptime_break;` |

当前已经确认的可调用值操作签名为：

```text
builtin.value_as_symbol(Value:comptime_value)->symbol
builtin.pastesymbol(First:symbol, Rest:symbol...)->symbol
builtin.compiler_parameter<T:type>(Key:parameter_key)->T
builtin.compiler_parameter<T:type>(Key:parameter_key, Default:T)->T
```

这些签名描述 verifier 规则，不引入用户可见的泛型推导。`symbol`、`parameter_key` 与 `comptime_value` 是 MetaIR 的编译期专用概念，不能作为普通运行时值残留。

## 7. 运算符与优先级

表达式中的符号运算统一改写为 `builtin.xxx`，运算优先级不因拼写变化而改变。结构性标点，例如泛型默认值和名称泛型实参使用的 `=`、参数分隔逗号、控制头分号及圆括号，不改写成 `builtin.xxx`。

当前运算符集合按从高到低的优先级排列如下：

| 层级 | 运算符 |
| --- | --- |
| 一元 | `builtin.logicalnot`、`builtin.bitwisenot`、`builtin.positive`、`builtin.negative` |
| 乘法 | `builtin.multiply`、`builtin.divide`、`builtin.modulo` |
| 加法 | `builtin.add`、`builtin.subtract` |
| 移位 | `builtin.shiftleft`、`builtin.shiftright` |
| 关系 | `builtin.less`、`builtin.lessequal`、`builtin.greater`、`builtin.greaterequal` |
| 相等 | `builtin.equal`、`builtin.notequal` |
| 按位与 | `builtin.bitwiseand` |
| 按位异或 | `builtin.bitwisexor` |
| 按位或 | `builtin.bitwiseor` |
| 逻辑与 | `builtin.logicaland` |
| 逻辑或 | `builtin.logicalor` |
| 初始化与赋值 | `builtin.assign`、`builtin.addassign`、`builtin.subassign`、`builtin.mulassign`、`builtin.divassign`、`builtin.modassign`、`builtin.andassign`、`builtin.orassign`、`builtin.xorassign`、`builtin.shiftleftassign`、`builtin.shiftrightassign` |

比较运算不允许链式书写，赋值操作不产生值也不允许链式书写。为了让表达式 Parser 只查看一个 Token 就能选择运算层级，每个 `builtin.xxx` 运算符在词法接口上作为一个原子终结符或已分类的运算符 Token 提供，而不是由表达式 Parser 临时拼接 `builtin`、`.` 和名称。

类型相等判断保留专用形式：

```text
T is int32
```

`is` 只比较两个编译期类型值的规范类型身份，不表示继承、可转换性或运行时类型测试。透明别名归一化为同一类型；限定符和泛型实参属于类型身份的一部分。`is` 不允许链式书写，依赖泛型形参时在实参闭合后求值。

## 8. 三种条件控制

运行时分支、编译期执行分支和宏式语法选择保持为三个不同结构，不尝试压缩成同一个 `if`。

| 结构 | 条件 | 分支检查 | 作用域 | 结果 |
| --- | --- | --- | --- | --- |
| `if` | 普通布尔值，可为 `Runtime` | 两个分支都做普通语义检查 | 普通词法作用域 | 保留为运行时控制流，之后可被普通优化折叠 |
| `builtin.comptime_if` | 必须是 `Known(bool)` | 两个分支都按编译期可执行代码做语义检查 | 普通词法作用域 | 只执行选中分支，结构本身不残留 |
| `builtin.macro_if` | 必须是 `Known(bool)` | 所有分支做语法检查；只对选中分支做后续语义检查 | 花括号是透明展开边界 | 把选中语法直接接入外层，结构本身不残留 |

`builtin.macro_if` 只有一套统一规则，不按 module、class、函数或语句位置拆成不同名称的宏节点。它的分支内可以出现任何符合 MetaIR 语法的代码，不因宏所在位置提前施加额外语义限制。Parser 必须完整解析活动和非活动分支；选中分支接入外层后，再按目标位置执行名称绑定、类型检查和其他普通语义检查。未选中分支不做名称绑定、类型检查、布局或代码生成。

宏分支的花括号不建立词法作用域。例如：

```text
builtin.macro_if(UseLarge)
{
	var Size:int32 builtin.assign 128;
}
else
{
	var Size:int32 builtin.assign 64;
}
```

选择完成后，`Size` 是外层区域中的普通声明。两个分支可以产生同名声明，因为最终只接入一个分支。

`builtin.comptime_if` 则是编译期解释器中的正常控制流。它的非活动分支仍须通过普通语义检查，而且每个分支块建立正常词法作用域。

## 9. `builtin.comptime_for` 是编译期执行循环

规范循环头为：

```text
builtin.comptime_for(Index:int32 builtin.assign 0; Index builtin.less 10; Index builtin.addassign 1)
{
	...
}
```

拼写固定为 `builtin.addassign`，不使用 `assignadd`。执行顺序为：

1. 初始化只执行一次；
2. 每轮开始前求值条件，结果必须是 `Known(bool)`；
3. 条件为真时由编译期解释器执行循环体；
4. 执行更新操作；
5. 回到条件求值，直到条件为假。

`builtin.comptime_for` 同时具有执行作用域和输出作用域：

- 循环控制变量是贯穿整个循环的一份可变绑定，只在循环头和循环体内可见；
- 每次迭代都为循环体重新建立执行作用域，循环体内声明的临时 `var` 每轮重新创建，并在该轮结束时销毁；
- 输出作用域保持透明，循环体生成的 `field`、`func`、`class` 等声明直接进入外层声明区域；
- 循环体可以修改外层 `Known` 变量；
- 不同迭代生成同名声明时，按普通重复声明规则报错。

因此，“循环体透明”只描述声明输出位置，不表示各轮临时变量共享同一词法生命周期。

例如：

```text
var Time:int32 builtin.assign 0;
builtin.comptime_for(Index:int32 builtin.assign 0; Index builtin.less 10; Index builtin.addassign 1)
{
	Time builtin.addassign Index;
}
```

循环由编译期解释器真实执行，结束后 `Time` 为 `Known(45)`。它不是简单把循环体复制十份后交给运行时执行；循环结构本身会消失，只保留编译期状态变化、可物化常量以及执行期间生成的声明。

编译期循环可以使用编译期条件：

```text
var Sum:int32 builtin.assign 0;
builtin.comptime_for(Index:int32 builtin.assign 0; Index builtin.less 10; Index builtin.addassign 1)
{
	builtin.comptime_if(Index builtin.less 5)
	{
		Sum builtin.addassign Index;
	}
}
```

结果 `Sum` 为 `Known(10)`。这里的赋值是编译期解释器对编译期状态的修改，所以若把 `Sum` 当成普通运行时局部变量阅读会显得奇怪；正确读法是“在编译期执行循环并计算常量 10”。只有后续运行时代码使用该值时，编译器才把它物化为运行时常量。

`builtin.comptime_break;` 与 `builtin.comptime_continue;` 分别控制最近一层编译期循环。普通 `break` 与 `continue` 仍表示准备输出的运行时控制语句；它们接入最终位置后若没有合法运行时循环，由普通语义检查报告错误。

首版采用严格执行模型：循环初始化、条件、更新以及循环体实际读取或写入的普通值都必须是 `Known`。一旦读取 `Runtime` 值就产生编译期错误，不对循环做局部残留或运行时拆分。

临时变量与声明输出可以同时使用：

```text
builtin.comptime_for(Index:int32 builtin.assign 0; Index builtin.less 3; Index builtin.addassign 1)
{
	var Temp:int32 builtin.assign Index builtin.multiply 2;

	field int32 call builtin.pastesymbol(Value, call builtin.value_as_symbol(Index)) builtin.assign Temp;
}
```

循环结束后，`Temp` 和 `Index` 都不再可见，外层得到的声明等价于：

```text
field int32 Value0 builtin.assign 0;
field int32 Value1 builtin.assign 2;
field int32 Value2 builtin.assign 4;
```

## 10. `Known`、`Runtime` 与变量

编译期解释器至少区分两种普通值状态：

```text
Known(Value)
Runtime(Value)
```

字面量、已绑定泛型实参、编译器参数和成功完成的编译期调用产生 `Known`。依赖运行时输入或未执行运行时调用的结果为 `Runtime`。

`var` 不增加 `comptime var` 变体，其阶段由初始化值和后续操作推导。只在编译期使用的 `Known` 变量可以在编译期执行完成后消除；若普通运行时代码读取它，则可将已知结果物化为运行时常量。`symbol`、`type` 等编译期专用值不能物化到运行时。一个已经是 `Known` 的变量后来接收 `Runtime` 值时应报错、提升阶段还是拆分状态，本轮尚未决定。

普通函数可以在不同阶段调用。`comptime call` 强制当前调用完成编译期求值；编译期控制结构内部的调用继承编译期执行上下文。若必经操作无法由编译期解释器执行，或者结果依赖 `Runtime`，则编译期执行失败。

## 11. 编译器参数

编译器参数通过显式、带结果类型的 builtin 调用读取：

```text
call builtin.compiler_parameter<target_os>(os.target)
call builtin.compiler_parameter<target_os>(os.target, window)
```

第一种形式要求参数存在；第二种形式在参数缺失时使用默认值。`os.target` 在语义上是规范点分参数键，`target_os` 是期望的编译期类型。参数不存在且没有默认值，或实际值不能按声明类型读取时，产生编译期错误。点分键最终采用专用词法形式还是普通 Token 序列，留给完整语法设计确定。

外部编译器接口可以使用如下形式注入参数：

```text
--parameter os.target=window
```

命令行中的 `=` 属于编译器接口，不属于 MetaIR 表达式运算符。

读取到的编译器参数值以及实际读取的参数集合必须进入构建缓存键和实例化身份。未被 `builtin.macro_if` 选中的分支不执行参数读取，因此不会仅因语法中出现读取调用就要求该参数存在。

典型目标选择写成：

```text
builtin.macro_if(call builtin.compiler_parameter<target_os>(os.target) builtin.equal window)
{
	...
}
```

## 12. 编译期符号生成

`builtin.pastesymbol` 是返回 `symbol` 的编译期值操作，必须通过 `call` 使用：

```text
call builtin.pastesymbol(map, call builtin.value_as_symbol(Count))
```

规则为：

- 当前示例中的 `map` 是字面 symbol 片段，不做普通值名称查找；
- `call builtin.value_as_symbol(Count)` 把已知编译期值转换为 symbol 片段；
- `builtin.pastesymbol` 按实参从左到右直接拼接，不自动插入分隔符；
- 当 `Count` 为 `3` 时，上例得到新 symbol `map3`；
- 返回结果可以出现在任何要求 symbol 的语法位置；
- 生成结果随后参加与手写名称相同的合法性、保留字、名称绑定和重复声明检查；
- 当前结果按请求生成的名称参加普通绑定，不另行约定卫生机制。

字段生成示例为：

```text
field std::vector<T> call builtin.pastesymbol(map, call builtin.value_as_symbol(Count));
```

要求 symbol 的位置当前接受标识符或返回 `symbol` 的 `call` 表达式；调用结果类型由语义阶段检查。`value_as_symbol` 支持的完整值集合和各类型的规范文本格式仍待后续确定；当前只确认它要求输入为已知编译期值，并且整数示例使用无分隔的十进制形式。除当前字面片段示例以外，裸标识符如何区分 symbol 字面量和已有 symbol 变量也仍待确定。

## 13. `field`、`var` 与访问修饰符

`field` 声明实例字段，使用“类型在前、名称在后”的形式：

```text
field std::map<T> Maps;
field int32 Value builtin.assign 1;
```

`var` 声明普通变量，使用“名称在前、类型在后”的形式，并始终显式给出类型：

```text
var Times:int32 builtin.assign comptime call get_times();
```

`var` 的存储类别由所在位置确定：module 中为全局变量，class 中为 class/static 变量，函数中为局部变量。它与表示每个实例存储的 `field` 不同。

访问修饰符 `public`、`protected`、`internal`、`private` 是完整声明的前缀，不使用 `private:` 一类状态式访问区段：

```text
private class MyClass<T:type = int32, V:type>
private var Test:int32 builtin.assign 1;
```

Parser 接受声明前缀，具体声明是否允许某种访问级别以及修饰符组合是否冲突由语义阶段检查。默认访问级别为：module 声明 `internal`，class 成员 `private`，未来的 struct 成员 `public`；局部变量不具有访问级别。

## 14. 函数与普通作用域

函数声明骨架为：

```text
func Name<Generic:type>(Parameter:Type)->ResultType
{
	...
}
```

省略返回类型时固定为 `void`。所有普通参数都显式写成 `Name:Type`。函数体、普通 `if` 分支和 `builtin.comptime_if` 分支建立词法作用域；`builtin.macro_if` 分支是透明语法展开边界；`builtin.comptime_for` 每轮建立临时执行作用域，同时把生成声明透明输出到外层。

函数名以及其他 symbol 位置可以使用返回 `symbol` 的 builtin 调用生成。生成后的函数声明与手写声明进入同一普通名称处理和重复检查流程；具体重载模型尚未确定。

## 15. 执行与绑定的已确认边界

本轮只确认以下阶段约束，不把它们扩展成一条已经定案的完整编译流水线：

```text
先完整解析所有语法分支
    -> 按实际执行需求解析名称、闭合泛型并读取编译器参数
    -> 执行编译期控制与调用，接入选中 macro 分支并生成 symbol 或声明
    -> 对生成后的有效程序完成普通语义处理
    -> 只把仍需运行时存在的内容交给 Runtime lowering
```

已确认的不变量为：

- 活动和非活动宏分支都必须先形成完整语法树；
- `comptime call` 在执行前必须完成足够的名称解析、签名检查和可执行性验证；
- 编译器参数按实际执行的读取操作惰性取得，不能预先读取未选中宏分支中的参数；
- 编译期执行只允许消费满足当前操作要求的 `Known` 值；
- `builtin.comptime_if` 的两个分支都在执行前完成普通编译期语义检查；
- `builtin.macro_if` 只把选中分支送入后续语义阶段；
- 编译期生成的名称和声明在执行完成后，与手写内容一起完成重复检查和普通绑定；
- 函数内普通局部变量仍按语句顺序进入作用域；
- 最终 lowering 只接收仍需运行时存在的值、声明和控制流。

名称解析、编译期执行、声明收集和类型检查可能需要交错或形成固定点。具体在哪个阶段收集 module、class 和函数声明头，生成声明允许哪些前向引用，以及增量执行时如何重做名称收集，本轮尚未最终确认。

## 16. 必须诊断的情况

至少需要诊断以下错误：

- 任意宏分支存在词法或语法错误；
- 泛型实参缺失、重复绑定、未知名称绑定或在名称实参后出现位置实参；
- 调用实参重复绑定、未知名称绑定或在名称实参后出现位置实参；
- 编译器参数缺失且没有默认值，或参数值类型不匹配；
- `builtin.macro_if`、`builtin.comptime_if` 或 `builtin.comptime_for` 条件不是 `Known(bool)`；
- 严格编译期上下文读取 `Runtime` 值；
- 编译期专用的 `symbol`、`type` 或其他元值试图残留到运行时；
- 生成 symbol 非法、命中保留字或与已有声明冲突；
- 编译期循环不收敛或超过实现规定的执行资源预算；
- 编译期控制语句没有对应的编译期循环目标。

## 17. LL(1) 解析承诺

当前已经用下列硬边界消除主要歧义：

- `call` 首 Token 提交到函数调用；类型表达式后直接出现 `(` 提交到类型构造；
- `<` 和 `>` 只用于泛型结构，比较和移位只能使用 `builtin` 运算符；
- `builtin.macro_if`、`builtin.comptime_if` 与 `builtin.comptime_for` 是不同结构头；
- 表达式运算符由词法接口作为原子终结符或已分类 Token 提供；
- 泛型列表中的 `Identifier =` 提交到名称泛型实参；
- 调用实参中的 `Identifier builtin.assign` 提交到名称调用实参；
- 赋值不属于值表达式，因而不会与位置实参竞争同一语法分支；
- 声明由 `class`、`func`、`field`、`var` 及可选访问前缀等固定起始标记区分。

完整 EBNF、错误恢复集合和每个 `builtin` Token 的词法枚举仍需单独编写并做 FIRST/FOLLOW 集验证；在完成该验证前，不把“目标为 LL(1)”扩大表述为已经机械证明整套文法为 LL(1)。

## 18. 尚待后续确定

本轮没有确定以下内容：

- 完整 MetaIR EBNF、Tokenizer 细节与语法错误恢复；
- `is` 在完整表达式优先级表中的精确层级；
- 各运算符的完整结合方向定义；
- `comptime_value`、`parameter_key`、`target_os` 等元类型的完整集合与序列化格式；
- `os.target` 一类点分参数键的具体 Token 与语法树表示；
- 所有 builtin 的最终注册表、签名编码和版本兼容策略；
- 未知 `builtin` 成员的词法、语法和语义诊断边界，以及原子运算符 Token 的具体实现方式；
- 编译期调用允许的外部效果、依赖追踪和权限模型；
- 编译期执行 fuel、递归深度、循环次数和声明生成量的具体上限；
- `builtin.comptime_if` 允许出现的完整上下文集合；
- `builtin.comptime_for` 三段式循环头是否允许省略其中某一段；
- `Known` 变量后来接收 `Runtime` 值时的阶段变化规则；
- 生成节点的源码映射、宏展开栈与诊断展示格式；
- `value_as_symbol` 对整数以外值的规范转换格式；
- `pastesymbol` 参数中的裸标识符如何区分 symbol 字面量与已有 symbol 值；
- 类型构造是否支持命名实参；
- 生成声明的精确收集时点和前向引用范围；
- 最终名称修饰、ABI、序列化格式以及与运行时 IR 的边界；
- `comptime_while`、编译期 `switch` 等尚未讨论的额外控制结构。

这些项目应在后续交互讨论中逐项确认，不能从现有 Ink 设计自动继承。
