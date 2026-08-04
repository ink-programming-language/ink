# Tokenizer 议题 04：硬关键字、上下文 `from` 与内建类型 Token

> 状态：已确认，2026-08-04 移除 `let`、`constructor`、`destructor` 硬关键字，并把 `from` 调整为唯一上下文词
> 确认日期：2026-08-04

## 1. 唯一的上下文词 `from`

Ink v0 只有一个 contextual keyword 拼写：`from`。Tokenizer 不为它建立 Keyword Token，也不根据前后语法环境改变 TokenKind；每次扫描到该完整拼写都产生普通 `Identifier("from")`。

Parser 只在两个已经确认的位置按准确拼写解释这个 Identifier：

- Parser 议题 06 的 `from module.path import member;` 成员导入开头；
- Parser 议题 27 中已经完成的新异常表达式之后，即 `throw expression from catch_binding;` 的原因子句。

其他位置的 `from` 都是普通标识符，包括声明名称、表达式起始位置和成员名。Ink v0 没有第二个上下文关键字。

除这一明确例外外，完整 ASCII 拼写要么始终按硬关键字或内建类型分类，要么始终是普通 Identifier。关键字分类只依赖议题 03 已经完成 XID 扫描和 NFC 验证后的完整拼写；Tokenizer 不进行 Parser 回退或语法猜测。类名构造函数和 `~类名` 析构函数由 Parser 在类成员声明上下文中识别，不改变类名 Identifier 的 TokenKind。

## 2. Token 分类

Identifier 形状的源码词可以产生以下 TokenKind：

```text
Keyword(KeywordKind)
BuiltinType(BuiltinTypeKind)
BoolLiteral(bool)
NullLiteral
Identifier
```

实现可以为每个关键字使用独立枚举项，例如 `KwFunc`、`KwIf`，也可以使用带 `KeywordKind` 载荷的统一表示；两者必须产生相同的规范分类。

所有 Token 继续保存议题 02 的原始 UTF-8 `raw` 和字节跨度，分类不改变源码无损恢复。

## 3. 硬关键字表

Ink v0 的硬关键字准确为：

```text
as
async
await
break
catch
class
comptime
const
continue
decorator
defer
else
enum
extern
for
func
if
implicit
import
in
interface
match
override
private
return
this
throw
try
var
virtual
while
```

这些拼写在任何源码位置都产生 `Keyword`，不能作为普通声明名称、字段名、参数名或局部变量名。Ink 不提供转义标识符来绕过该表。

`from`、`let`、`constructor` 和 `destructor` 都不在硬关键字表中：

- `from` 始终产生 `Identifier("from")`，Parser 只在第 1 节列出的两个位置赋予上下文含义；
- `let` 按普通标识符规则产生 `Identifier("let")`，Parser 不接受它作为绑定声明的起始 Token；
- `constructor` 和 `destructor` 是普通 Identifier，可以作为普通声明或成员名称；它们不再标记生命周期函数。

因此以下普通标识符用法合法：

```ink
const from = fallback_error;
const text = String.from("hello");
throw from;
```

最后一行中的 `from` 是新异常表达式的变量名，不是原因标记。相比之下，下面两个位置由 Parser 识别上下文含义，但其底层 Token 仍是 Identifier：

```ink
from core.io import File;
throw WrapperError {} from error;
```

关键字全部由小写 ASCII 字母组成，并按大小写精确匹配：

```ink
func // Keyword(Func)
Func // Identifier("Func")
```

## 4. 基于类名的构造和析构声明

构造函数使用当前类名，析构函数使用 `~` 加当前类名；二者继续保留 Ink 的 `func` 声明引导词：

```ink
class Resource {
    func Resource(value: i32) {
        // ...
    }

    func ~Resource() {
        // ...
    }
}
```

Tokenizer 对两处 `Resource` 都产生普通 `Identifier("Resource")`，对 `~` 产生单字符 Symbol Token。Parser 只按 `Identifier` 或 `~ Identifier` 保存函数名称语法，不查询所属类名，也不在 CST 阶段决定生命周期身份。语义分析把类内与所属类同名的普通名称判定为构造函数，并把 `~ Identifier` 判定为析构函数候选。

`~` 和类名是两个独立 Token，中间允许空白、换行或注释 Trivia；这些写法具有相同语法：

```ink
func ~Resource() {}
func ~ Resource() {}
func ~ /* lifecycle */ Resource() {}
```

格式化器统一输出紧凑的 `~Resource`。`~` 后缺少 Identifier 属于语法错误；析构形式出现在类外、名称与所属类不匹配，以及构造或析构声明违反生命周期约束，均由语义分析报告。

`constructor` 和 `destructor` 不具有上下文关键字含义：

```ink
func constructor() {} // 名为 constructor 的普通函数
object.destructor()   // 名为 destructor 的普通成员调用
```

## 5. 布尔与空指针字面量

以下保留拼写不产生 Keyword，而是直接产生字面量 Token：

```text
true  → BoolLiteral(true)
false → BoolLiteral(false)
null  → NullLiteral
```

它们不能作为 Identifier。把布尔值直接编码进 TokenKind 可以避免 parser 或常量求值器再次按文本区分 `true` 和 `false`；`raw` 仍保留原始字节。

## 6. 内建类型表

核心内建类型拼写产生 `BuiltinType`：

```text
i8  i16  i32  i64  i128
u8  u16  u32  u64  u128

int
uint
ptrsize

f16
f32
f64

bool
byte
void
never
type
```

对应的规范枚举可以表示为：

```text
BuiltinType(I8)
BuiltinType(I16)
...
BuiltinType(PtrSize)
BuiltinType(Bool)
BuiltinType(Type)
```

这些拼写不能用作用户声明名称，也不通过普通导入或局部作用域被遮蔽。类型检查器仍负责确定每种内建类型的语义、目标宽度和合法使用位置；Tokenizer 只完成准确拼写分类。

## 7. 分类示例

源码：

```ink
const value: i32 = 10;
```

忽略 Trivia 后产生：

```text
Keyword(Const)
Identifier("value")
Symbol(':')
BuiltinType(I32)
Symbol('=')
IntegerLiteral
Symbol(';')
```

完整 Token 列表中仍包含各项之间的 `SpacesAndTabs`，因而能够还原原始源码。

## 8. 函数式内建操作不是关键字

以下函数式内建操作按普通 Identifier 扫描：

```text
cast
bitcast
ptrcast
try_cast
reflect
```

它们的表面形式与普通函数或泛型函数调用一致，例如：

```ink
cast<u16>(value)
reflect(Player)
```

名称绑定和语义分析负责把特定可见名称关联到编译器内建声明。Tokenizer 不为每个内建函数增加特殊 TokenKind。

同理：

- 装饰器中的特殊 `function` 参数词法上是 Identifier；
- `[reflect]` 等属性名是 Identifier；
- `String`、`UnicodeScalar` 等标准库类型是 Identifier；
- 普通库函数和未来新增内建声明不自动进入硬关键字表。

## 9. 扫描与分类顺序

Identifier 形状的候选按以下顺序处理：

```text
scan maximal IdentifierStart IdentifierContinue*
→ validate Ink identifier profile and NFC
→ exact lookup in hard-keyword table
→ exact lookup of true / false / null
→ exact lookup in BuiltinType table
→ otherwise emit Identifier
```

`from` 不在 hard-keyword lookup 中命中，因此总是走到最后一步。上下文识别发生在 Tokenizer 完成整个 Token 流之后，不改变上述扫描顺序。

所有保留拼写都是完整词匹配：

```ink
functional // Identifier，不拆成 func + tional
i32value   // Identifier，不拆成 i32 + value
nullable   // Identifier，不拆成 null + able
```

Tokenizer 议题 05 规定数字字面量紧邻类型后缀时，后缀属于同一个数字 Token，不能通过本节普通 Identifier 分类顺序推导。

## 10. 版本兼容性

关键字表、上下文词位置和内建类型表都属于 Ink 语言版本。把一个既有 Identifier 拼写新增为硬关键字或 `BuiltinType` 会改变 token stream；增加新的上下文解释位置虽然不改变 TokenKind，也可能改变既有源码的 parse。两者都属于源码兼容性变更，不能在编译器补丁版本中静默发生。

词法表只包含已经采用的语法和核心类型，不为尚未确定的功能预留普通英文单词。未来语言版本需要新增保留词时，应提供明确迁移诊断。

## 11. 诊断

Tokenizer 只负责准确分类。硬关键字出现在需要 Identifier 的位置通常由 parser 诊断，并应说明该拼写是保留关键字。

工具可以建议用户改名，但不能通过把关键字重新解释为 Identifier 继续正常构建。`BuiltinType` 出现在非法表达式或声明位置同样由 parser 或类型检查器根据语法阶段报告。

对于 `from`，Tokenizer 不报告关键字诊断。Parser 只有在成员导入开头或新异常表达式之后匹配准确拼写时才建立对应上下文节点；其他位置按普通 Identifier 继续解析。上下文结构缺失后续 module path、`import`、原因绑定或分号时，也由相应 Parser 议题执行错误恢复。
