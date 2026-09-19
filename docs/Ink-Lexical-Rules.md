1. **空白**：字符串、字符字面量和注释之外的 ASCII 空格（`U+0020`）、Tab（`U+0009`）和物理换行作为 token 分隔符跳过；物理换行仅支持 LF、CRLF、CR，不产生换行或缩进 token；其他 Unicode 空白以及换页符、垂直制表符不作为空白跳过。

2. **数值（INTEGER_LITERAL、FLOAT_LITERAL）**：数值使用 ASCII 数字；整数支持十进制及 `0b/0B`、`0o/0O`、`0x/0X` 前缀，十进制允许前导零且不因此转为八进制；十进制浮点字面量由“数字串＋指数”或“数字串＋小数点＋数字串＋可选指数”构成，各数字串均为非空的 ASCII 十进制数字序列；指数由 `e/E`、可选的正负号及非空的 ASCII 十进制数字串构成；包含小数点或指数的合法十进制数值归为 `FLOAT_LITERAL`，仅包含十进制数字串的归为 `INTEGER_LITERAL`；字面量前的正负号独立分词，指数中的正负号属于浮点 token；禁止下划线、类型后缀及十六进制浮点；残缺的进制前缀、残缺指数、进制不允许的数字，以及数值扫描结束后紧邻 Unicode 17.0.0 的 `XID_Continue` 字符的情况均报词法错误，不回退拆分为多个合法 token。`.5`、`1.` 不识别为浮点字面量，其中的 `.` 独立分词；词法阶段仅验证书写形式并区分整数和浮点 token，具体数值类型、可表示范围及溢出由后续阶段判断，不因超出某个固定数值类型的范围而报词法错误；例如 `1e3`、`1.0`、`1.0e3` 和 `1e99999` 均产生 `FLOAT_LITERAL`，`1` 产生 `INTEGER_LITERAL`。

3. **字符串（STRING_LITERAL）**：仅支持双引号和三双引号字符串，不支持单引号或三单引号字符串；仅支持无前缀及紧邻起始引号的 `r/R` 前缀；在字符串起始位置优先识别三双引号定界符，识别后未闭合时直接报错，不回退为短字符串；普通字符串按第 5 条处理转义，在首次遇到未被转义的匹配定界符时结束，三双引号字符串中未构成结束定界符的单个或连续两个双引号作为内容保留；`r/R` 表示原始字符串，其中反斜杠始终作为普通字符，不执行转义、不影响结束引号的识别，也不具有续行作用，允许内容以任意数量的反斜杠结尾，并在首次遇到与起始定界符相同的引号序列时结束；普通短字符串不允许未转义的物理换行，原始短字符串不允许任何物理换行，三双引号字符串允许多行；字符串内容不进行 Unicode 规范化；未闭合字符串报词法错误。

4. **字符字面量（CHAR_LITERAL）**：使用一对单引号作为定界符，不支持三单引号形式；仅支持无前缀及紧邻起始单引号的 `r/R` 前缀；普通字符字面量采用与普通短字符串相同的转义及物理换行规则，在首次遇到未被转义的单引号时结束；原始字符字面量采用与原始短字符串相同的规则，反斜杠不执行转义、不影响结束单引号的识别且不具有续行作用，不允许任何物理换行；内容不进行 Unicode 规范化，在完成换行处理及转义处理后必须恰好包含一个 Unicode 标量值，即 `U+0000` 至 `U+D7FF` 或 `U+E000` 至 `U+10FFFF` 中的一个码点；空内容、多于一个 Unicode 标量值、未闭合以及非法转义均报词法错误；字符数按 Unicode 标量值计算，不按 UTF-8 字节数、UTF-16 码元数或显示字形数计算。

   | 字符字面量示例 | 结果 |
      |---|---|
   | `r'\'` | 合法，内容为一个反斜杠，`U+005C`。 |
   | `r'\n'` | 词法错误，内容为反斜杠和字母 `n` 两个 Unicode 标量值。 |
   | `'\U0001F600'` | 合法，内容为一个 Unicode 标量值，`U+1F600`。 |
   | `'\uD83D\uDE00'` | 词法错误，禁止代理码点转义，不将两个代理码点合并为一个字符。 |
   | `'e\u0301'` | 词法错误，内容为两个 Unicode 标量值，字符内容不进行 NFC 规范化。 |

5. **转义与换行处理**：以下转义形式参考 [Python 3.14](https://docs.python.org/3.14/reference/lexical_analysis.html#escape-sequences)，仅适用于不带 `r/R` 前缀的字符串和字符字面量；未知或不完整的转义直接报词法错误；八进制转义值不得超过 `0xFF`，任何转义均不得产生代理码点（`U+D800` 至 `U+DFFF`）或超过 `U+10FFFF` 的值；转义数字仅使用 ASCII 数字及十六进制字母，十六进制字母不区分大小写，转义引导字母区分大小写；转义结果不再次作为源码解析，也不递归执行转义；包括原始字面量在内，字面量中的物理换行先统一为 LF，再按对应字面量规则处理，转义产生的 `\r` 等字符不参与物理换行归一化。

   | 转义形式 | 规则及结果 |
      |---|---|
   | `\\` | 反斜杠，`U+005C`。 |
   | `\'` | 单引号，`U+0027`。 |
   | `\"` | 双引号，`U+0022`。 |
   | `\a` | 响铃，`U+0007`。 |
   | `\b` | 退格，`U+0008`。 |
   | `\f` | 换页，`U+000C`。 |
   | `\n` | 换行，`U+000A`。 |
   | `\r` | 回车，`U+000D`。 |
   | `\t` | 水平制表，`U+0009`。 |
   | `\v` | 垂直制表，`U+000B`。 |
   | `\ooo` | 连续读取 1 至 3 位 `0` 至 `7` 的八进制数字，取其值对应的 Unicode 字符；`\0` 属于此形式；值超过 `0o377` 报错。 |
   | `\xhh` | 恰好读取 2 位十六进制数字，取其值对应的 Unicode 字符。 |
   | `\uhhhh` | 恰好读取 4 位十六进制数字，取其值对应的 Unicode 字符。 |
   | `\Uhhhhhhhh` | 恰好读取 8 位十六进制数字，取其值对应的 Unicode 字符。 |
   | `\N{name}` | 按 Unicode 17.0.0 的正式字符名称或 [NameAliases.txt](https://www.unicode.org/Public/17.0.0/ucd/NameAliases.txt) 中的别名解析为一个 Unicode 字符；名称中的 ASCII 字母不区分大小写，空格和连字符精确匹配；不支持命名字符序列，空名称、未知名称或缺少花括号均报错。 |
   | 反斜杠紧接物理换行 | 删除反斜杠及该换行，不产生字符；反斜杠与换行之间不得插入空格或其他字符；换行后的缩进仍属于字面量内容。 |

   八进制转义读取最多 3 位数字，定长转义只读取规定数量的数字，多余数字作为后续内容；例如 `"\1234"` 的内容为 `S4`，`"\x414"` 的内容为 `A4`，`'\x414'` 因结果包含两个 Unicode 标量值而报错。`\xFF` 表示 `U+00FF`，不表示一个未经解码的原始字节。

6. **标识符（IDENTIFIER）**：采用 [Unicode 17.0.0](https://www.unicode.org/versions/Unicode17.0.0/) 的字符属性，首字符为 `XID_Start` 或 `_`，后续字符为零个或多个 `XID_Continue`；区分大小写；按原始字符验证合法性后进行 NFC 规范化，规范化结果相同的标识符视为同一名称；完整扫描并规范化后，匹配关键字的归为对应关键字 token，单独的 `_` 归为 `UNDERSCORE`，其余归为 `IDENTIFIER`。

7. **注释**：`//` 从起始符延续至物理换行或文件结束，不支持反斜杠续行；`/*` 延续至首个 `*/`，块注释不可嵌套，未闭合时报词法错误；注释仅在字符串和字符字面量之外识别；注释作为 token 分隔符跳过，不产生 token，`TokenKind` 不定义 `LINE_COMMENT`、`BLOCK_COMMENT`；跳过注释不得拼接两侧文本，例如 `a/**/b` 产生两个标识符。

8. **文件结束（END_OF_FILE）**：文件结束 token 统一命名为 `END_OF_FILE`，纳入 `token` 的候选项及 `TokenKind`；正常扫描到文件末尾时，在输出 token 流末尾追加一个 `END_OF_FILE`，其源码长度为零；空文件同样产生该 token，文件末尾不要求存在换行。

9. **文件边界与非法字符**：源码必须采用严格的 UTF-8 编码，禁止文件开头的 UTF-8 BOM（字节序列 `EF BB BF`）；非法 UTF-8 序列，包括截断序列、非法连续字节、过长编码、代理码点编码及超出 `U+10FFFF` 的编码，在任何位置均直接报词法错误，字符串、字符字面量和注释内部也不例外；源文件中直接出现的 `U+0000` 在任何位置均禁止，但普通字符串和字符字面量允许通过 `\0`、`\x00` 等合法转义产生空字符；非文件开头的 `U+FEFF` 可作为字符串、字符字面量或注释的内容，在这些区域之外报词法错误；除上述全局限制外，字符串、字符字面量和注释中的 Unicode 字符按各自的内容及定界规则处理；其他区域中，不能被当前词法规则接受的字符直接报词法错误，NBSP（`U+00A0`）、全角空格（`U+3000`）等其他 Unicode 空白以及换页符（`U+000C`）、垂直制表符（`U+000B`）均不得跳过；字符串、字符字面量和注释之外的反斜杠没有续行作用，直接报词法错误；未闭合的字符串、字符字面量或块注释直接报词法错误；正常到达文件末尾时产生 `END_OF_FILE`。

10. **最长匹配规则**：从左到右扫描，在当前词法状态下优先匹配最长 token；完整扫描标识符并进行 NFC 规范化后，再判断是否为关键字或单独的 `_`；字面量前缀只在 token 起始位置识别，且仅当当前字符为 `r/R` 并紧邻对应的起始引号时，才与后续字面量整体识别；其他名称按标识符和关键字规则完整扫描，再从引号位置识别后续字面量；不支持的前缀拼写本身不构成词法错误，所得 token 序列是否符合语法由语法分析判断；字符串定界符的识别优先级以及未闭合字面量、未闭合注释和非法数值不回退的规则优先于通用最长匹配规则。

    | 前缀边界示例 | 分词结果 |
        |---|---|
    | `r"abc"` | 一个原始 `STRING_LITERAL`。 |
    | `r "abc"` | `IDENTIFIER`（`r`）和一个普通 `STRING_LITERAL`。 |
    | `xr"abc"` | `IDENTIFIER`（`xr`）和一个普通 `STRING_LITERAL`。 |
    | `f"abc"` | `IDENTIFIER`（`f`）和一个普通 `STRING_LITERAL`。 |
    | `r/**/"abc"` | `IDENTIFIER`（`r`）和一个普通 `STRING_LITERAL`，注释跳过。 |

11. **源码位置**：每个 token 的源码范围使用原始 UTF-8 文件中从零开始的字节偏移，以左闭右开的区间 `[start, end)` 表示，源码长度为 `end - start`；字面量的源码范围包含其前缀（若有）、起始定界符、内容和结束定界符；标识符的 NFC 规范化、字面量的物理换行归一化及转义处理仅影响解析得到的名称或字面量值，不改变 token 对应的原始源码范围；`END_OF_FILE` 的起止偏移均等于原始文件的字节长度。

12. **相邻字符串**：词法阶段不合并相邻字符串字面量；在遵守定界符识别优先级的前提下，无分隔符或仅以空白、注释分隔的多个字符串字面量分别产生 `STRING_LITERAL`；例如 `"a" "b"`、`"a""b"` 和 `"a"/**/"b"` 均产生两个 `STRING_LITERAL`；是否允许相邻字符串以及是否自动拼接，由语法和语义规则规定。

```ebnf
KW_AS ::= "as" ;
KW_BREAK ::= "break" ;
KW_CASE ::= "case" ;
KW_CLASS ::= "class" ;
KW_COMPTIME ::= "comptime" ;
KW_CONST ::= "const" ;
KW_CONTINUE ::= "continue" ;
KW_DEFAULT ::= "default" ;
KW_DEFER ::= "defer" ;
KW_DO ::= "do" ;
KW_ELSE ::= "else" ;
KW_ENUM ::= "enum" ;
KW_FIELD ::= "field" ;
KW_FOR ::= "for" ;
KW_FROM ::= "from" ;
KW_FUNC ::= "func" ;
KW_IF ::= "if" ;
KW_IMPLEMENTS ::= "implements" ;
KW_IMPORT ::= "import" ;
KW_IN ::= "in" ;
KW_INTERFACE ::= "interface" ;
KW_MATCH ::= "match" ;
KW_RETURN ::= "return" ;
KW_SWITCH ::= "switch" ;
KW_VAR ::= "var" ;
KW_WHILE ::= "while" ;
KW_YIELD ::= "yield" ;

keyword ::= KW_AS
          | KW_BREAK
          | KW_CASE
          | KW_CLASS
          | KW_COMPTIME
          | KW_CONST
          | KW_CONTINUE
          | KW_DEFAULT
          | KW_DEFER
          | KW_DO
          | KW_ELSE
          | KW_ENUM
          | KW_FIELD
          | KW_FOR
          | KW_FROM
          | KW_FUNC
          | KW_IF
          | KW_IMPLEMENTS
          | KW_IMPORT
          | KW_IN
          | KW_INTERFACE
          | KW_MATCH
          | KW_RETURN
          | KW_SWITCH
          | KW_VAR
          | KW_WHILE
          | KW_YIELD ;

LPAREN ::= "(" ;
RPAREN ::= ")" ;
LBRACKET ::= "[" ;
RBRACKET ::= "]" ;
LBRACE ::= "{" ;
RBRACE ::= "}" ;
COMMA ::= "," ;
SEMICOLON ::= ";" ;
COLON ::= ":" ;
DOUBLE_COLON ::= "::" ;
DOT ::= "." ;
ELLIPSIS ::= "..." ;
UNDERSCORE ::= "_" ;
PLUS ::= "+" ;
MINUS ::= "-" ;
STAR ::= "*" ;
SLASH ::= "/" ;
PERCENT ::= "%" ;
PLUS_PLUS ::= "++" ;
MINUS_MINUS ::= "--" ;
BANG ::= "!" ;
TILDE ::= "~" ;
AMP ::= "&" ;
PIPE ::= "|" ;
CARET ::= "^" ;
AMP_AMP ::= "&&" ;
PIPE_PIPE ::= "||" ;
SHIFT_LEFT ::= "<<" ;
SHIFT_RIGHT ::= ">>" ;
LESS ::= "<" ;
LESS_EQUAL ::= "<=" ;
GREATER ::= ">" ;
GREATER_EQUAL ::= ">=" ;
EQUAL_EQUAL ::= "==" ;
BANG_EQUAL ::= "!=" ;
ASSIGN ::= "=" ;
PLUS_ASSIGN ::= "+=" ;
MINUS_ASSIGN ::= "-=" ;
STAR_ASSIGN ::= "*=" ;
SLASH_ASSIGN ::= "/=" ;
PERCENT_ASSIGN ::= "%=" ;
AMP_ASSIGN ::= "&=" ;
PIPE_ASSIGN ::= "|=" ;
CARET_ASSIGN ::= "^=" ;
SHIFT_LEFT_ASSIGN ::= "<<=" ;
SHIFT_RIGHT_ASSIGN ::= ">>=" ;
QUESTION ::= "?" ;
NULL_COALESCE ::= "??" ;
QUESTION_DOT ::= "?." ;
ARROW ::= "->" ;
FAT_ARROW ::= "=>" ;

symbol ::= LPAREN
         | RPAREN
         | LBRACKET
         | RBRACKET
         | LBRACE
         | RBRACE
         | COMMA
         | SEMICOLON
         | COLON
         | DOUBLE_COLON
         | DOT
         | ELLIPSIS
         | UNDERSCORE
         | PLUS
         | MINUS
         | STAR
         | SLASH
         | PERCENT
         | PLUS_PLUS
         | MINUS_MINUS
         | BANG
         | TILDE
         | AMP
         | PIPE
         | CARET
         | AMP_AMP
         | PIPE_PIPE
         | SHIFT_LEFT
         | SHIFT_RIGHT
         | LESS
         | LESS_EQUAL
         | GREATER
         | GREATER_EQUAL
         | EQUAL_EQUAL
         | BANG_EQUAL
         | ASSIGN
         | PLUS_ASSIGN
         | MINUS_ASSIGN
         | STAR_ASSIGN
         | SLASH_ASSIGN
         | PERCENT_ASSIGN
         | AMP_ASSIGN
         | PIPE_ASSIGN
         | CARET_ASSIGN
         | SHIFT_LEFT_ASSIGN
         | SHIFT_RIGHT_ASSIGN
         | QUESTION
         | NULL_COALESCE
         | QUESTION_DOT
         | ARROW
         | FAT_ARROW ;

token ::= IDENTIFIER
        | INTEGER_LITERAL
        | FLOAT_LITERAL
        | CHAR_LITERAL
        | STRING_LITERAL
        | keyword
        | symbol
        | END_OF_FILE ;
```

```cpp
enum TokenKind
{
    END_OF_FILE,
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,
    STRING_LITERAL,
    KW_AS,
    KW_BREAK,
    KW_CASE,
    KW_CLASS,
    KW_COMPTIME,
    KW_CONST,
    KW_CONTINUE,
    KW_DEFAULT,
    KW_DEFER,
    KW_DO,
    KW_ELSE,
    KW_ENUM,
    KW_FIELD,
    KW_FOR,
    KW_FROM,
    KW_FUNC,
    KW_IF,
    KW_IMPLEMENTS,
    KW_IMPORT,
    KW_IN,
    KW_INTERFACE,
    KW_MATCH,
    KW_RETURN,
    KW_SWITCH,
    KW_VAR,
    KW_WHILE,
    KW_YIELD,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    COMMA,
    SEMICOLON,
    COLON,
    DOUBLE_COLON,
    DOT,
    ELLIPSIS,
    UNDERSCORE,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    PLUS_PLUS,
    MINUS_MINUS,
    BANG,
    TILDE,
    AMP,
    PIPE,
    CARET,
    AMP_AMP,
    PIPE_PIPE,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    EQUAL_EQUAL,
    BANG_EQUAL,
    ASSIGN,
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    STAR_ASSIGN,
    SLASH_ASSIGN,
    PERCENT_ASSIGN,
    AMP_ASSIGN,
    PIPE_ASSIGN,
    CARET_ASSIGN,
    SHIFT_LEFT_ASSIGN,
    SHIFT_RIGHT_ASSIGN,
    QUESTION,
    NULL_COALESCE,
    QUESTION_DOT,
    ARROW,
    FAT_ARROW,
};
```
