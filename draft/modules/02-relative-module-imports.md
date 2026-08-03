# Module 议题 02：相对 Module 导入

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 绝对与相对路径

Ink 的 module 导入路径支持两种静态写法：

```ink
import game.graphics.window;
from game.graphics.window import Window;

import .window;
from .window import Window;
from ..common.math import Vector;
```

绝对路径从当前构建中注册的根 package 开始；相对路径从当前 module 所在 package 开始。两者最终都解析为 Module 议题 01 定义的绝对规范 module 身份。

## 2. 路径文法

Parser 使用：

```ebnf
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

`relative_prefix` 使用标准 EBNF special sequence 引用 Tokenizer Token。前导点必须直接相邻，不能跨 Trivia；后面的 Identifier 和路径分隔 `.` 遵守普通 Parser Trivia 规则。

相对路径必须在前导点之后包含一个 module 路径段，不能只写点序列。

## 3. 前导点含义

一个前导点表示当前 module 所在 package；此后每增加一个前导点就向父 package 上移一层。

假设当前 module 是：

```text
game.graphics.renderer
```

当前 module 所在 package 是 `game.graphics`，所以：

```text
.window
→ game.graphics.window

..common.math
→ game.common.math
```

这里 `.window` 中的 `window` 是同 package 下的 `window.ink`；`..common.math` 先从 `game.graphics` 上移到 `game`，再进入 subpackage `common` 的 `math.ink`。

## 4. 根 Package 边界

相对路径不能越过当前根 package。对于根 package 直属 module：

```text
current module = game.main
current package = game
```

`.util` 可以解析为 `game.util`，但 `..util` 试图越过 `game`，因此非法。

相对导入只能引用当前根 package 内的 module。导入外部依赖必须使用以该依赖根 package 名开头的绝对路径。

## 5. 最终目标必须是 Module

相对路径和绝对路径的最后一个段都必须解析为 `.ink` module 文件，不能只解析到 package 目录。

例如存在：

```text
src/graphics/           package game.graphics
src/graphics/window.ink module game.graphics.window
```

则 `.window` 可以作为 module 路径；指向 `game.graphics` package 本身的路径不能作为 module import。

## 6. 规范身份

相对路径只是源码查找写法，不进入 module、声明、反射或缓存的规范身份：

```ink
from .window import Window;
```

解析后记录：

```text
module = game.graphics.window
declaration = game.graphics.window.Window
```

源码使用绝对路径、相对路径或 `as` 别名，不会为同一个 module 创建不同身份。

## 7. 条件导入

Parser 议题 05 的候选收集保存相对路径拼写、当前 module 身份以及控制分支。顶层 `comptime` 选择该候选后，模块系统将它规范化为绝对 module 身份并解析实际文件。

未激活候选路径不要求目标 module 存在。激活路径不能越过根 package，并且必须最终解析为 module 文件。

不同相对拼写规范化到同一个绝对 module 时，它们指向同一实际依赖；各个导入位置仍保留独立 CST 和别名绑定。

## 8. 文件移动

相对导入以当前 module 所在 package 为基准，所以移动当前 `.ink` 文件可能改变相对路径的解析结果。重构工具移动 module 时必须同时更新受影响的相对导入，或者将它们重写为等价绝对路径。

绝对 module 身份始终由移动后的新文件系统位置重新建立，遵守 Module 议题 01 的重命名规则。

## 9. 确认结论

Ink module import 同时支持根 package 开始的绝对路径和前导点表示的相对路径。一个点表示当前 package，额外的点逐层上移，但不能越过根 package；最终目标必须是 `.ink` module。相对拼写解析后立即规范化为绝对 module 身份，不改变依赖、反射和缓存语义。
