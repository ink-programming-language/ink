# Tokenizer 议题 04：硬关键字与内建类型 Token

> 状态：已确认，2026-08-04 移除 `constructor`、`destructor` 硬关键字；2026-08-05 确认 `operator` 无特殊语法，并补齐访问修饰符、`static` 与 `final`；2026-08-08 将 `from` 与 `let` 确认为硬关键字
> 确认日期：2026-08-04
> 最近更新：2026-08-08

## 1. 硬关键字 `from` 与 `let`

`from` 与 `let` 都是硬关键字。Tokenizer 每次扫描到这两个完整拼写都产生对应的 `Keyword` Token，不根据前后语法环境改变 TokenKind，也不把它们回退为 Identifier。

Parser 在两个已经确认的位置使用 `from`：

- Parser 议题 06 的 `from module.path import member;` 成员导入开头；
- Parser 议题 27 中已经完成的新异常表达式之后，即 `throw expression from catch_binding;` 的原因子句。

`let` 保留供语言使用，但当前 Parser 文法不以它引导绑定声明；现有绑定声明仍只使用 `var` 与 `const`。它与 `from` 一样不能作为声明名称、表达式中的普通名称或成员名；需要识别它时，Parser 可以直接检查 `KeywordKind`，不需要读取 Identifier 文本。

Ink v0 没有上下文关键字。完整 ASCII 拼写要么始终按硬关键字或内建类型分类，要么始终是普通 Identifier。关键字分类只依赖议题 03 已经完成 XID 扫描和 NFC 验证后的完整拼写；Tokenizer 不进行 Parser 回退或语法猜测。类名构造函数和 `~类名` 析构函数由 Parser 在类成员声明上下文中识别，不改变类名 Identifier 的 TokenKind。

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
final
for
from
func
if
implicit
import
in
interface
let
match
override
private
protected
public
return
static
this
throw
try
var
virtual
while
```

这些拼写在任何源码位置都产生 `Keyword`，不能作为普通声明名称、字段名、参数名或局部变量名。Ink 不提供转义标识符来绕过该表。

访问修饰符的三个拼写全部属于硬关键字：

```ebnf
access_modifier =
      "public"
    | "protected"
    | "private" ;
```

因此函数或其他声明 Parser 可以直接按 TokenKind 识别访问修饰符，不需要读取 Identifier 文本或引入新的上下文关键字。它们能否出现在某种声明上、重复或冲突修饰符如何诊断，仍由相应 Parser 与语义规则决定。

`static` 与 `final` 同样属于硬关键字。`static` 在类成员函数声明中表示该函数没有隐式接收者；`final` 可以封闭类的具体继承或封闭一个虚函数槽。Parser 议题 31 保存对应函数声明修饰符，能否出现在当前声明上下文以及它们与其他修饰符的组合是否合法，由语义分析检查。

`constructor`、`destructor` 和 `operator` 不在硬关键字表中：

- `constructor` 和 `destructor` 是普通 Identifier，可以作为普通声明或成员名称；它们不再标记生命周期函数。
- `operator` 是普通 Identifier；Ink v0 不提供 `operator+`、`operator[]`、`operator()` 等运算符重载声明语法，也不把它解释成上下文关键字。

因此以下普通标识符用法合法：

```ink
func constructor() {}
object.destructor()
func operator() {}
```

相比之下，以下两个位置的 `from` 都产生硬关键字 Token：

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

对应的 v0 函数名称产生式准确为：

```ebnf
function_name =
      identifier
    | "~", identifier ;
```

运算符符号不属于 `function_name`。由于 `operator` 仍是普通 Identifier，`func operator() {}` 只是一个名为 `operator` 的普通函数；在名称后继续出现 `+`、`[]` 或第二组 `()` 不能形成运算符声明。

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

- 装饰器体内隐式 continuation 的 `function` 拼写词法上是 Identifier；
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

`from`、`let`、`public`、`protected`、`static` 与 `final` 都在 hard-keyword lookup 中命中。`constructor`、`destructor` 与 `operator` 不命中任何保留拼写，因而走到最后一步并产生 Identifier。

所有保留拼写都是完整词匹配：

```ink
functional // Identifier，不拆成 func + tional
i32value   // Identifier，不拆成 i32 + value
nullable   // Identifier，不拆成 null + able
```

Tokenizer 议题 05 规定数字字面量紧邻类型后缀时，后缀属于同一个数字 Token，不能通过本节普通 Identifier 分类顺序推导。

## 10. 版本兼容性

关键字表和内建类型表都属于 Ink 语言版本。把一个既有 Identifier 拼写新增为硬关键字或 `BuiltinType` 会改变 token stream，属于源码兼容性变更，不能在编译器补丁版本中静默发生。

`let` 的硬关键字身份已由本议题明确确认；除此之外，词法表不为尚未确定的功能预留普通英文单词。未来语言版本需要新增保留词时，应提供明确迁移诊断。

## 11. 诊断

Tokenizer 只负责准确分类。硬关键字出现在需要 Identifier 的位置通常由 parser 诊断，并应说明该拼写是保留关键字。

工具可以建议用户改名，但不能通过把关键字重新解释为 Identifier 继续正常构建。`BuiltinType` 出现在非法表达式或声明位置同样由 parser 或类型检查器根据语法阶段报告。

`from` 与 `let` 出现在需要 Identifier 的位置时遵循普通硬关键字诊断，不得按上下文回退为 Identifier。当前没有 Parser 产生式接受 `let`；成员导入或异常原因子句缺失后续语法时，由相应 Parser 议题执行错误恢复。
