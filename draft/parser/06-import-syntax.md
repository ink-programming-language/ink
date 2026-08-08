# Parser 议题 06：Import 语法

> 状态：已确认，议题 27 同步 `from` 硬关键字；议题 39 明确 module 顶层绑定的显式访问前缀由名称绑定阶段解释
> 确认日期：2026-08-03

## 1. 两种导入形式

Ink 支持模块命名空间导入和成员导入：

```ebnf
import_declaration =
    module_import_declaration
  | member_import_declaration ;

module_import_declaration =
    "import", module_path, [ import_alias ], ";" ;

member_import_declaration =
    "from", module_path, "import", imported_member,
    { ",", imported_member }, ";" ;

import_alias = "as", identifier ;

imported_member = identifier, [ import_alias ] ;

module_path =
    absolute_module_path
  | relative_module_path ;

absolute_module_path =
    identifier, ".", identifier, { ".", identifier } ;

relative_module_path =
    relative_prefix, identifier, { ".", identifier } ;

relative_prefix =
    ? One or more directly adjacent Symbol('.') Tokens ? ;
```

`identifier` 引用 Parser 议题 04 所定义的 Token 类别。`from`、`import` 和 `as` 都是硬关键字，产生式中的终结字符串直接匹配 Tokenizer 产生的相应 Keyword Token；路径分隔符、逗号和分号继续匹配规范 Symbol Token。

这一分派不需要回溯：module 顶层和条件导入体中允许 `import_declaration` 的位置若以 `Keyword(from)` 开始，就进入成员导入产生式。`from` 不能匹配 `identifier`，Parser 不在其他位置把它重新解释为变量、函数或成员名称。

## 2. 模块命名空间导入

```ink
import core.io;
import network.http as http;
```

未写 `as` 时，默认绑定 module 路径的最后一个段，也就是 `.ink` 文件主名：

```ink
import core.io;

func main() {
    io.print("hello");
}
```

写出别名后，以别名作为当前模块中的绑定名称：

```ink
import network.http as http;

func load() {
    http.get(...);
}
```

模块命名空间导入不把被导入模块的成员批量注入当前作用域。

## 3. 成员导入

`from` 形式直接导入模块中按名称导出的声明：

```ink
from core.io import File;
from core.io import read_file as read;
from math.scalar import sin, cos, Vector as MathVector;
```

未写别名时，成员在当前模块中使用原名称；写出 `as` 时使用别名：

```ink
from core.io import read_file as read;

func load_config() {
    const content = read("config.json");
}
```

`from` 导入不自动建立来源 module 的命名空间绑定。以下声明只建立 `File`：

```ink
from core.io import File;
```

因此不能仅凭该声明使用 `io.print`；如果同时需要模块命名空间，必须另写模块导入。

## 4. 导入对象

`imported_member` 表示被导入模块中具有对应名称、且允许从当前模块访问的顶层导出声明。成员究竟是函数、类型、全局绑定、装饰器或其他声明，由名称绑定阶段确定，不由 Parser 判断。

全局绑定可以按 Parser 议题 39 显式写 `public` 或 `private` 前缀，也可以省略访问修饰符。显式前缀和省略时的默认可见性都由名称绑定阶段决定是否构成可导入声明；`imported_member` 的 EBNF 不为不同声明种类增加分支。

如果该名称表示函数重载集合，则成员导入引入该名称下可见的整个重载集合，后续调用仍按普通重载解析规则选择具体函数。

成员是否导出、访问权限是否允许、名称是否存在以及别名是否冲突，均不属于 EBNF；Parser 只构造对应 CST。

## 5. Package/Module 导入路径

Module 议题 01 规定完整绝对 module 路径为：

```text
root-package.subpackage*.module
```

EBNF 至少要求两个 Identifier 段：第一个段是根 package，最后一个段必须解析为 `.ink` module 文件，中间各段必须解析为 subpackage 目录。例如：

```ink
import core.io;
from company.network.http import Client;
```

Module 路径不能是字符串、普通表达式或编译期计算结果：

```ink
import comptime make_path();
from "core.io" import File;
```

这些形式不符合 EBNF。`.` 只写逻辑 package/module 名称分段，不表示宿主文件系统路径分隔符。`.ink` 扩展名不出现在导入路径中。

Module 议题 02 允许从当前 module 所在 package 开始的相对路径：

```ink
import .window;
from .window import Window;
from ..common.math import Vector;
```

一个前导点表示当前 package，每增加一个前导点向上一级 package；路径不能越过根 package。相对路径解析后规范化为同一个绝对 module 身份。

## 6. 分号与 Trivia

两种导入声明都必须以 `;` 结束。标准 Parser Trivia 规则允许关键语法元素之间出现空白、换行和注释：

```ink
from core.io import
    File,
    read_file as read;
```

Parser 议题 07 规定普通逗号列表不允许尾随逗号，因此成员导入的最后一个名称后直接写 `;`。多行导入同样不增加尾随逗号或额外括号形式。

## 7. 作用域与冲突

直接或条件激活的导入绑定都属于当前模块作用域，不属于顶层 `comptime if` 花括号形成的临时局部作用域。绑定的可见性不依赖导入声明在源文件中的文本位置。

名称冲突只在 Parser 议题 05 完成条件筛选后，对同时激活的导入和声明进行检查。因此可以在互斥分支中建立同名绑定：

```ink
comptime if (target.os == Os.windows) {
    from platform.windows import Window as NativeWindow;
} else {
    from platform.linux import Window as NativeWindow;
}
```

一次构建中只有一个 `NativeWindow` 被激活，所以不构成重复绑定。

如果两个同名绑定同时激活，则由名称绑定阶段报告冲突；Parser 不根据当前构建目标删除 CST 分支。

## 8. 候选依赖

两种导入形式都建立 Parser 议题 05 所定义的静态候选 module 路径：

```ink
import platform.windows;
from platform.linux import Window;
```

候选发现阶段只需要读取 `module_path`，不需要先解析 `imported_member` 是否真实存在。顶层 `comptime` 求值后，仅保留分支中的 module 路径成为实际依赖。

一个激活的 `from` 导入仍然使整个来源模块成为实际模块依赖。成员绑定在最终代码中未被引用，不自动取消模块依赖，因为来源模块可能具有已确认的模块生命周期行为；是否可以在无可观察生命周期时优化掉依赖属于后端优化，不改变语言语义。

## 9. 确认结论

Ink 同时支持 `import module-path [as alias];` 和 `from module-path import member [as alias], ...;`。前者绑定 module 命名空间，后者直接绑定指定的公开顶层名称；绝对或相对 module 路径都必须静态写在源码中，并统一参与条件导入的候选依赖收集。相对路径激活后规范化为绝对 module 身份。
