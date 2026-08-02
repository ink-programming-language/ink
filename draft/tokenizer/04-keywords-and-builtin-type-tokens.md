# Tokenizer 议题 04：硬关键字与内建类型 Token

> 状态：已确认  
> 确认日期：2026-08-02

## 1. 不使用上下文关键字

Ink v0 不使用 contextual keyword。一个完整 ASCII 拼写要么始终按硬关键字或内建类型分类，要么始终是普通 Identifier；Tokenizer 不根据前后语法环境改变它的 TokenKind。

因此关键字分类只依赖议题 03 已经完成 XID 扫描和 NFC 验证后的完整拼写。Parser 不需要先尝试把 Identifier 当作某个关键字，失败后再回退。

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
constructor
decorator
defer
destructor
else
enum
extern
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
return
this
throw
try
var
virtual
while
```

这些拼写在任何源码位置都产生 `Keyword`，不能作为普通声明名称、字段名、参数名或局部变量名。Ink 不提供转义标识符来绕过该表。

关键字全部由小写 ASCII 字母组成，并按大小写精确匹配：

```ink
func // Keyword(Func)
Func // Identifier("Func")
```

## 4. 特殊构造和析构名称

`constructor` 与 `destructor` 虽然主要出现在 `func` 之后，仍是全局硬关键字：

```ink
func constructor(value: i32) {
    // ...
}

func destructor(this: Resource&) {
    // ...
}
```

Tokenizer 不把它们作为上下文 Identifier。Parser 在合法声明位置接受对应 Keyword，在其他位置报告语法错误。

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
let value: i32 = 10;
```

忽略 Trivia 后产生：

```text
Keyword(Let)
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

所有保留拼写都是完整词匹配：

```ink
functional // Identifier，不拆成 func + tional
i32value   // Identifier，不拆成 i32 + value
nullable   // Identifier，不拆成 null + able
```

Tokenizer 议题 05 规定数字字面量紧邻类型后缀时，后缀属于同一个数字 Token，不能通过本节普通 Identifier 分类顺序推导。

## 10. 版本兼容性

关键字表和内建类型表属于 Ink 语言版本。把一个既有 Identifier 拼写新增为硬关键字或 `BuiltinType` 会改变 token stream，属于源码兼容性变更，不能在编译器补丁版本中静默发生。

词法表只包含已经采用的语法和核心类型，不为尚未确定的功能预留普通英文单词。未来语言版本需要新增保留词时，应提供明确迁移诊断。

## 11. 诊断

Tokenizer 只负责准确分类。硬关键字出现在需要 Identifier 的位置通常由 parser 诊断，并应说明该拼写是保留关键字。

工具可以建议用户改名，但不能通过把关键字重新解释为 Identifier 继续正常构建。`BuiltinType` 出现在非法表达式或声明位置同样由 parser 或类型检查器根据语法阶段报告。
