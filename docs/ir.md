# InkIR 文本格式

> 状态：当前实现的文本格式版本 1
>
> 更新日期：2026-08-18

## 1. 范围

本文档规定 InkIR 的文本词法、模块级声明、类型、操作数、SSA 值、基本块和指令格式，并记录当前 verifier 对这些结构施加的有效性约束。

`src/include/ink/ir/ir.def` 只维护类型、值和指令的枚举元数据，不再承担文本格式文档的职责。指令元数据同时记录助记符、终结指令属性和 SSA 结果策略。文本的 lexer、parser、resolver 和规范化 printer 分别实现在 `src/lib/ir/text` 中，`src/lib/ir/serialization.cpp` 只负责编排这些阶段以及调用 verifier。能够被 lexer 和 parser 接受但不能通过 verifier 的文本不是有效 InkIR。

文本 parser 使用 detached instruction 和临时基本块保留文本中显式给出的 SSA 编号与未解析引用，在完成一个函数的语法解析后通过公共 `ink::ir::IRBuilder` 提交模块结构。面向编译器其他阶段的 `IRBuilder` 提供 `createFunction`、`createBlock` 以及覆盖全部指令种类的类型化 `createXxx` 接口；这些接口会在插入前完整初始化指令，并为值指令自动分配 SSA 编号。后续 AST lowering 因而不依赖任何文本 token 或文本格式实现。

本文使用以下 EBNF 记号：

- `"text"` 表示必须原样出现的终结符。
- `A B` 表示顺序连接。
- `A | B` 表示二选一。
- `[A]` 表示可选。
- `{A}` 表示重复零次或多次。
- `(A)` 用于分组。

语法产生式描述 token 序列，不把换行视为语法的一部分。示例采用规范化序列化器生成的排版。

## 2. 词法

### 2.1 空白与注释

空格、制表符和换行等空白只用于分隔 token。分号 `;` 开始单行注释，注释持续到下一个换行或文件末尾。当前格式没有块注释。

```text
; 整行注释
%0 = add i32 1, i32 2 ; 行尾注释
```

换行本身不结束声明或指令，因此输入可以采用不同排版；序列化器始终输出本文件所示的规范排版。

### 2.2 标点

以下标点各自形成独立 token：

```text
( ) { } [ ] , : = *
```

`@` 只用于引入全局名称，`%` 只用于引入结构体类型名称或十进制 SSA 编号，不能单独出现。

### 2.3 普通名称和带前缀名称

名称必须是有效 UTF-8，并使用与前端 tokenizer 相同的 Unicode 15.1 `XID_Start` 和 `XID_Continue` 字符集。普通 IR 名称额外允许 `_`、`.` 和 `$`：

```ebnf
name-start    = XID_Start | "_" | "." | "$";
name-continue = XID_Continue | "_" | "." | "$";
name          = name-start {name-continue};

global-name   = "@" name;
type-name     = "%" name;
ssa-name      = "%" unsigned-integer;
block-name    = name;
```

名称区分大小写。`XID_Continue` 包含 `XID_Start`，但只属于 `XID_Continue` 的字符不能作为首字符。`.` 和 `$` 是普通 IR 名称的扩展字符，不属于模块名段。全局名称中的 `@` 和类型名称中的 `%` 是文本前缀，不属于存储在 IR 对象中的名称。

IR 对象模型使用统一的 `Name` 值类型存储模块、函数、全局符号、结构体类型、基本块和导入目标名称。`Name` 拥有其 UTF-8 文本并提供上述普通名称规则的合法性检查、比较和哈希；它不按用途拆分子类型。模块名仍在使用位置额外检查 2.4 节的分段约束，匿名模块使用空的 `optional<Name>` 表示。

结构体类型名称不能以数字开头，因为 `%` 后紧跟十进制数字表示 SSA 名称。SSA 名称只允许十进制编号，不接受命名 SSA 值。

字节常量、全局变量和函数共享一个模块级全局名称空间，三者之间也不能重名。结构体类型使用独立名称空间。基本块名称只需在所属函数内唯一。

### 2.4 规范模块名

模块名不使用 `@` 或 `%` 前缀，必须包含至少两个以点分隔的段：

```ebnf
module-segment = (XID_Start | "_") {XID_Continue | "_"};
module-name    = module-segment "." module-segment {"." module-segment};
```

有效示例：

```text
application.main
game.graphics.window
_game._window2
应用.主模块2
```

无效示例包括 `main`、`.main`、`game.`、`game..main` 和 `game.$main`。

### 2.5 整数和十六进制数

```ebnf
unsigned-integer = ASCII-digit {ASCII-digit};
signed-integer   = ["-"] ASCII-digit {ASCII-digit};
hexadecimal      = ("0x" | "0X") hexadecimal-digit {hexadecimal-digit};
```

不接受 `+` 前缀、数字分隔符或非十进制整数常量。输入中的十六进制数字和 `x` 可以使用大小写；规范输出固定使用小写 `0x` 和大写十六进制数字。

### 2.6 字节字符串

字符串使用双引号。原始换行不能出现在字符串中。反斜杠后接受以下转义：

| 拼写 | 解码结果 |
| --- | --- |
| `\n` | 换行字节 `0A` |
| `\r` | 回车字节 `0D` |
| `\t` | 制表字节 `09` |
| `\\` | 反斜杠字节 `5C` |
| `\"` | 双引号字节 `22` |
| `\HH` | 由恰好两个十六进制数字指定的任意字节 |

规范输出直接写出 `20` 至 `7E` 范围内除 `"` 和 `\` 外的 ASCII 字节；其他字节一律写成带大写数字的 `\HH`。因此输入中的短转义在重新序列化后可能变成十六进制转义。

## 3. 文档和模块结构

每个文件必须以格式标识和版本号开始。当前唯一受支持的版本是 `1`：

```ebnf
document = "inkir" unsigned-integer {top-level-declaration};
```

```text
inkir 1
```

顶层声明包括：

```ebnf
top-level-declaration = module-declaration
                      | initializer-declaration
                      | finalizer-declaration
                      | struct-declaration
                      | byte-constant-declaration
                      | global-declaration
                      | imported-global-declaration
                      | external-function-declaration
                      | imported-function-declaration
                      | function-definition;
```

解析器允许大多数顶层声明交错出现，并允许函数、全局变量、字节常量、生命周期函数和基本块的前向引用。结构体字段所引用的结构体类型必须已经声明，不能前向引用。

### 3.1 模块身份

```ebnf
module-declaration = "module" module-name;
```

模块声明可省略；省略时表示匿名的独立 IR 模块。一个文件最多只能有一条模块声明。非空模块名必须符合规范模块名格式。

### 3.2 初始化器和终结器

```ebnf
initializer-declaration = "initializer" global-name;
finalizer-declaration   = "finalizer" global-name;
```

每种声明最多出现一次，可以引用后面定义的函数。目标必须是本模块中定义的无参数 `void` 函数，不能是外部函数或导入函数。普通 `call` 不能直接调用初始化器或终结器；模块加载器负责调用生命周期函数。终结器中不能执行 `import` 指令。

## 4. 类型

### 4.1 类型拼写

```ebnf
byte-pointer-type = ["const"] "byte" "*";
byte-slice-type   = ["const"] "byte" "[" "]";

type = "void"
     | "bool"
     | "byte"
     | "i32"
     | "ptrsize"
     | byte-pointer-type
     | byte-slice-type
     | "f16"
     | "f32"
     | "f64"
     | type-name;
```

| 类型 | 含义与主要约束 |
| --- | --- |
| `void` | 无值类型，只能用于函数或 `call` 的无返回值结果位置 |
| `bool` | 布尔整数，常量只能是 `0` 或 `1` |
| `byte` | 8 位无符号整数，常量范围为 `0` 至 `255` |
| `i32` | 32 位有符号整数 |
| `ptrsize` | 目标指针宽度的无符号整数，范围由目标上下文决定 |
| `byte*` | 可写字节指针 |
| `const byte*` | 只读字节指针 |
| `byte[]` | 带长度的可写安全字节切片 |
| `const byte[]` | 带长度的只读安全字节切片 |
| `f16`、`f32`、`f64` | 16、32、64 位浮点数；文本常量保存精确位模式 |
| `%Name` | 当前模块内已声明的具名结构体类型 |

类型修饰符中的空白在输入时不敏感，但规范拼写固定为 `const byte*` 和 `const byte[]`。

### 4.2 结构体

```ebnf
struct-declaration = type-name "=" "type" "{" type {"," type} "}";
```

```text
%Pair = type {i32, i32}
%Record = type {byte, %Pair, const byte*}
```

有效结构体必须至少有一个字段。字段不能是 `void`、`byte[]` 或 `const byte[]`。结构体不能直接引用自身，也不能引用尚未出现的结构体声明；按声明顺序引用先前的结构体可以形成嵌套，但不能形成递归类型。

结构体字段和函数共用同一种 attribute 列表语法：

```ebnf
attribute-list     = "[" [attribute {"," attribute}] "]";
attribute          = identifier ["(" attribute-argument {"," attribute-argument} ")"];
attribute-argument = identifier "=" operand;
```

attribute 名称来自固定的内建注册表；参数名由使用方选择，参数值必须是带类型常量。attribute 和参数的顺序都会在 InkIR 文本往返中保留。

### 4.3 内存值类型

`load`、`store` 和全局变量只接受内存值类型。当前内存值类型为：

```text
bool, byte, i32, ptrsize, f16, f32, f64
```

字段全部递归满足同一条件的结构体也是内存值类型。`void`、指针、切片以及包含这些字段的结构体不是内存值类型。

函数返回类型不能是 `byte[]` 或 `const byte[]`。函数参数不能是 `void`；此外，`extern "C"` 函数的参数不能是切片。Ink 定义函数和导入函数可以接收切片参数。

## 5. 值和操作数

除少数明确说明的语法位置外，指令操作数总是同时写出类型和值：

```ebnf
operand = type operand-value;
```

例如 `i32 %0`、`ptrsize 16` 和 `const byte* @message[0]`。文本中的类型不是提示信息；它必须与 SSA 定义、声明或常量种类要求的类型完全一致。

### 5.1 SSA 值

```ebnf
ssa-value = ssa-name;
```

函数参数从 `%0` 开始连续编号。所有产生结果的指令继续按函数内基本块和指令的序列化顺序连续编号，不能跳号、重复或重新定义。

普通 SSA 使用必须由同一函数内可用的定义支配。在同一基本块内，定义必须位于使用之前。`phi` 的传入值按对应前驱块末尾的位置检查，因此循环回边可以引用文本中稍后定义的 SSA 值。

### 5.2 整数常量

```ebnf
integer-value = signed-integer;
```

整数常量只适用于 `bool`、`byte`、`i32` 和 `ptrsize`。`bool` 和 `byte` 不接受负数；`ptrsize` 不接受负数，且不能超过目标指针宽度的最大值；`i32` 必须落在 32 位有符号范围内。

```text
bool 1
byte 255
i32 -42
ptrsize 4096
```

### 5.3 浮点常量

```ebnf
float-value = "floatbits" "(" float-format "," hexadecimal ")";
float-format = "f16" | "f32" | "f64";
```

外层操作数类型必须与 `float-format` 一致。`f16`、`f32` 和 `f64` 的位模式必须分别包含恰好 4、8 和 16 个十六进制数字：

```text
f16 floatbits(f16,0x3C00)
f32 floatbits(f32,0x80000000)
f64 floatbits(f64,0x7FF8000000000042)
```

这种格式保留正负零、无穷、NaN payload 和所有其他位级差异，不进行十进制浮点转换。

### 5.4 字节字符串常量

```ebnf
string-value = "c" string-literal;
```

字符串值的操作数类型必须是 `const byte[]`：

```text
const byte[] c"hello\0A"
```

### 5.5 空指针和零初始化

```ebnf
null-value             = "null";
zero-initializer-value = "zeroinitializer";
```

`null` 只适用于 `byte*` 和 `const byte*`。`zeroinitializer` 表示给定非 `void` 类型的全零值，包括结构体的递归零初始化：

```text
byte* null
%Pair zeroinitializer
```

### 5.6 聚合常量

```ebnf
aggregate-value = "{" [operand {"," operand}] "}";
```

聚合常量的外层类型必须是结构体，元素数量和顺序必须与字段一致。每个元素都完整写出自己的类型，并且必须仍是常量，不能使用 SSA 值：

```text
%Outer {i32 7, %Inner {f32 floatbits(f32,0x7FC00042), const byte* null}}
```

### 5.7 字节常量地址

```ebnf
byte-constant-address = global-name "[" unsigned-integer "]";
```

字节常量地址的操作数类型必须是 `const byte*`。方括号中的字节偏移可以等于常量长度以表示尾后地址，但不能超过长度：

```text
const byte* @message[0]
```

### 5.8 全局变量地址

```ebnf
global-variable-address = global-name;
```

没有方括号的全局名称表示全局变量地址，操作数类型必须是 `byte*` 或 `const byte*`：

```text
byte* @counter
const byte* @answer
```

可变全局变量可以取得可写地址。模块自有的不可变全局变量只有在其初始化器中可以取得可写地址；其他函数必须使用只读地址。导入的不可变全局变量始终不能取得可写地址。

## 6. 顶层数据和函数声明

### 6.1 私有字节常量

```ebnf
byte-constant-declaration = global-name "=" "private" "constant"
                            "[" unsigned-integer "x" "byte" "]"
                            "c" string-literal;
```

```text
@message = private constant [6 x byte] c"hello\0A"
```

方括号中的大小是字符串解码后的字节数，不是源文本字符数，并且必须精确匹配。字节常量只能通过 `const byte* @name[offset]` 取址。

### 6.2 模块自有全局变量

```ebnf
global-declaration = global-name "=" "global" ("mutable" | "constant") memory-value-type;
```

```text
@counter = global mutable i32
@answer = global constant i32
```

声明只规定存储和可变性，文本中没有内联初始化值。不可变全局的初始化写入由模块初始化器完成。

### 6.3 导入全局变量

```ebnf
imported-global-declaration = "declare" "import" "global"
                              ("mutable" | "constant") memory-value-type global-name
                              "from" "module" module-name "," "symbol" global-name;
```

```text
declare import global constant i32 @dependency.answer from module dependency.api, symbol @answer
```

第一个全局名称是本模块使用的本地符号，`symbol` 后的名称是目标模块导出的符号。目标模块名必须有效；具名模块不能从自身导入。

### 6.4 外部 C 函数

```ebnf
external-function-declaration = "declare" "extern" string-literal type global-name
                                "(" [declaration-parameter {"," declaration-parameter}] ")"
                                [attribute-list];
declaration-parameter = [identifier ":"] type ["=" operand];
```

当前唯一支持的外部调用约定字符串是精确的 `"C"`：

```text
declare extern "C" i32 @write(i32, const byte*, ptrsize) [sideeffect]
```

声明参数可以携带名称和带类型常量默认值，但不写 SSA 名称。外部函数没有函数体。函数 attribute 使用有序 attribute 列表和带类型常量参数；`[sideeffect]` 是外部副作用标记。

### 6.5 导入 Ink 函数

```ebnf
imported-function-declaration = "declare" "import" type global-name
                                "(" [declaration-parameter {"," declaration-parameter}] ")"
                                "from" "module" module-name "," "symbol" global-name
                                [attribute-list];
```

```text
declare import void @dependency.hook() from module dependency.api, symbol @hook [sideeffect]
```

导入函数隐式使用 Ink 调用约定且没有函数体。本地符号、目标模块和目标符号的含义与导入全局变量相同。函数体中的 `call` 始终引用本地符号 `@dependency.hook`，不在调用点重复写模块名。

## 7. 函数、基本块和控制流

### 7.1 函数定义

```ebnf
function-definition = "define" type global-name
                      "(" [definition-parameter {"," definition-parameter}] ")"
                      [attribute-list]
                      "{" basic-block {basic-block} "}";
definition-parameter = [identifier ":"] type ssa-name ["=" operand];
```

```text
define i32 @add_one(i32 %0) {
entry:
  %1 = add i32 %0, i32 1
  ret i32 %1
}
```

参数 SSA 编号必须按 `%0`、`%1`、……连续出现。参数名称是可选的签名元数据；默认值必须是与参数类型相同的带类型常量。默认值不省略 InkIR `call` 的显式实参，调用处仍须传入全部参数。定义函数可以携带普通 function attribute，但不能使用 `[sideeffect]`，并且必须至少包含一个基本块。

### 7.2 基本块

```ebnf
basic-block = block-name ":" instruction {instruction};
```

函数中的第一个基本块是入口块。每个基本块必须非空，并以恰好一条终结指令结束；终结指令不能提前出现。入口块不能成为 `br` 或 `condbr` 的目标。

所有 `phi` 必须连续位于基本块开头，普通指令之后不能再出现 `phi`。入口块不能包含 `phi`。

## 8. 指令

下列格式中的 `T value` 表示完整的带类型操作数。`%N =` 只出现在产生 SSA 结果的指令前。`import`、`store`、`lifetime.end`、`br`、`condbr` 和 `ret` 不能定义结果。

### 8.1 `call`

```text
call void @callee(<argument>, ...)
%N = call <non-void-type> @callee(<argument>, ...)
```

```ebnf
call-argument = operand;
```

被调用函数可以在文本后面声明，但最终必须存在。调用处的返回类型、参数数量和每个参数类型必须与声明完全一致。`void` 调用不能定义 SSA 结果，非 `void` 调用必须定义结果。生命周期函数不能由普通 `call` 调用。

```text
call void @notify(i32 1)
%2 = call i32 @read(i32 0, byte* %1, ptrsize 16)
```

### 8.2 `import`

```text
import <module-name>
```

`import` 请求在运行时加载指定的规范模块。目标不能是当前具名模块，且该指令不能出现在模块终结器中。`import` 不是终结指令，也不产生结果。

```text
import dependency.runtime
```

### 8.3 `alloca`

```text
%N = alloca byte[] ptrsize <size>
```

`alloca` 分配指定字节数的栈存储并返回可写 `byte[]`。它只能出现在函数入口块中，大小操作数必须是 `ptrsize`。

```text
%0 = alloca byte[] ptrsize 64
```

### 8.4 `getelementptr`

```text
%N = getelementptr <element-type>, <pointer-operand>, <root-index>[, <field-index>]...
```

根指针必须是 `byte*` 或 `const byte*`，结果保留完全相同的指针类型。`element-type` 必须具有目标相关的内存布局；根索引必须是 `ptrsize`，表示以 `element-type` 为步长的索引。

每个可选字段索引都必须写成 `i32 <non-negative-constant>`，并依次在当前结构体类型中选择字段；动态 SSA 字段索引、负索引、越界索引或对非结构体继续取字段均无效。

```text
%2 = getelementptr %Outer, const byte* %0, ptrsize %1, i32 1, i32 0
```

### 8.5 `load`

```text
%N = load <memory-value-type>, <pointer-operand>
```

指针操作数可以是 `byte*` 或 `const byte*`。结果类型必须是内存值类型。如果直接从全局变量地址读取，结果类型还必须与该全局变量的声明类型一致。

```text
%1 = load i32, const byte* @answer
```

### 8.6 `store`

```text
store <memory-value-operand>, byte* <pointer-value>
```

被存储值必须是内存值类型，目标必须是可写 `byte*`。如果直接写入全局变量地址，被存储值类型还必须与全局变量声明类型一致。

```text
store i32 42, byte* @counter
```

### 8.7 `lifetime.end`

```text
lifetime.end byte[] <slice-value>
```

该指令结束一个可写安全字节切片的生命周期。它不接受 `const byte[]`，不产生结果，也不是终结指令。

### 8.8 `slice.data`

```text
%N = slice.data <pointer-type> <slice-operand>
```

结果类型必须是 `byte*` 或 `const byte*`，操作数必须是 `byte[]` 或 `const byte[]`。从 `const byte[]` 提取数据时结果必须保持为 `const byte*`；从 `byte[]` 提取时可以得到可写或只读指针。

```text
%1 = slice.data byte* byte[] %0
%3 = slice.data const byte* const byte[] %2
```

### 8.9 `slice.length`

```text
%N = slice.length <slice-operand>
```

操作数必须是 `byte[]` 或 `const byte[]`，结果类型固定为 `ptrsize`，因此助记符后不再单独写结果类型。

```text
%1 = slice.length byte[] %0
```

### 8.10 `phi`

```text
%N = phi <type> [<value>, <predecessor>], ...
```

`phi` 至少需要一个传入项。传入值省略类型，因为所有值都使用 `phi` 后给出的结果类型；例如结果类型为 `i32` 时写 `[0, entry]` 和 `[%2, body]`，不能写 `[i32 0, entry]`。

传入项必须与基本块的控制流前驱边一一对应，值类型必须等于结果类型。若同一前驱通过多条边到达该块，对应传入值必须一致。

```text
%0 = phi i32 [0, entry], [%2, body]
```

### 8.11 `add`

```text
%N = add <integer-operand>, <integer-operand>
```

当前支持 `byte`、`i32` 和 `ptrsize`。左右操作数必须具有相同类型，结果也具有该类型。

```text
%1 = add i32 %0, i32 1
```

### 8.12 `icmp`

```text
%N = icmp <predicate> <comparable-operand>, <comparable-operand>
```

谓词为：

| 谓词 | 含义 |
| --- | --- |
| `eq` | 相等 |
| `ne` | 不相等 |
| `lt` | 小于 |
| `le` | 小于等于 |
| `gt` | 大于 |
| `ge` | 大于等于 |

两个操作数类型必须相同，结果类型固定为 `bool`。可比较类型为 `bool`、`byte`、`i32`、`ptrsize`、`byte*` 和 `const byte*`。其中 `bool` 和两种指针只支持 `eq` 与 `ne`；整数类型支持全部六种谓词。

```text
%2 = icmp lt i32 %0, i32 10
```

### 8.13 `insertvalue`

```text
%N = insertvalue <struct-operand>, <field-operand>, <field-index>
```

第一个操作数必须是结构体，结果类型与它相同。字段索引是没有类型前缀的非负十进制整数，必须落在字段范围内；第二个操作数类型必须与对应字段类型一致。

```text
%0 = insertvalue %Pair zeroinitializer, i32 20, 0
```

### 8.14 `extractvalue`

```text
%N = extractvalue <struct-operand>, <field-index>
```

字段索引是没有类型前缀的非负十进制整数，必须落在字段范围内。结果类型由所选字段推导，因此文本中不额外写结果类型。

```text
%1 = extractvalue %Pair %0, 0
```

### 8.15 `br`

```text
br <target-block>
```

`br` 无条件跳转到同一函数内的非入口基本块，是终结指令。

### 8.16 `condbr`

```text
condbr bool <condition>, <true-target>, <false-target>
```

条件必须是 `bool`，两个目标都必须是同一函数内的非入口基本块。`condbr` 是终结指令。

### 8.17 `ret`

```text
ret void
ret <type> <value>
```

`void` 函数必须使用 `ret void`。非 `void` 函数必须返回一个与函数结果类型完全一致的操作数。`ret` 是终结指令。

## 9. 完整示例

```text
inkir 1
module application.main
initializer @init

@message = private constant [14 x byte] c"Hello, world!\0A"

@answer = global constant i32

declare extern "C" i32 @write(i32, const byte*, ptrsize) [sideeffect]

define void @init() {
entry:
  import dependency.runtime
  store i32 42, byte* @answer
  ret void
}

define i32 @main() {
entry:
  %0 = load i32, const byte* @answer
  %1 = call i32 @write(i32 1, const byte* @message[0], ptrsize 14)
  ret i32 %0
}
```

## 10. 规范化序列化

序列化器在输出前验证整个模块；无效 IR 不产生部分文本。有效模块按以下规则输出：

1. 第一行固定为 `inkir 1`。
2. 可选的 `module`、`initializer` 和 `finalizer` 紧随其后，并按此顺序输出。
3. 随后依次输出结构体、私有字节常量、全局变量和函数；每个声明前有一个空行，同一类别内保留 IR 对象中的顺序。
4. 基本块标签不缩进，指令缩进两个空格。
5. 逗号后写一个空格，其他空白采用本文件示例中的形式。
6. 整数使用十进制最短拼写；浮点位模式使用固定宽度的大写十六进制数字。
7. 字节字符串使用规范 `\HH` 转义规则。
8. 使用 LF 换行，并在文件末尾保留一个换行。

反序列化器接受更自由的空白、注释和大多数顶层声明顺序。将成功反序列化的模块再次序列化，会得到上述唯一规范形式。

## 11. 扩展要求

新增或修改类型、值种类或指令时，必须同步更新以下内容：

- `src/include/ink/ir/ir.def` 中的枚举元数据；
- 文本 lexer、parser 和 serializer；
- verifier 的类型、SSA、控制流和指令约束；
- 本文档中的语法与语义说明；
- 覆盖规范输出、成功往返和非法输入的 IR 测试。
