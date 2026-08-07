# Module 议题 01：文件系统 Package 与单文件 Module

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 基本模型

Ink 使用与 Python 类似的目录映射建立 package/module 层级，但 v0 不引入 package 初始化源码：

```text
package manifest
└─ source root                 root package
   ├─ subdirectory            subpackage
   │  └─ source.ink           module
   └─ source.ink              module
```

规则为：

- 包清单确定根 package 的规范名称和源码根目录；
- 源码根目录表示根 package；
- 源码根目录下的每层子目录表示一层 subpackage；
- 每个 `.ink` 源文件准确形成一个 module；
- 一个 module 不跨越多个物理源文件；
- 源文件不声明 `package` 或 `module` 头。

因此 `package` 和 `module` 都是正式语言概念，但它们的身份由包清单和源码根内的规范相对路径建立，不由源文件内的声明建立。

## 2. 映射示例

概念布局：

```text
manifest:
    root package = game
    source root  = src

src/
├─ main.ink
├─ graphics/
│  ├─ window.ink
│  └─ renderer.ink
└─ network/
   └─ http/
      └─ client.ink
```

规范身份为：

```text
src/main.ink                      → game.main
src/graphics/window.ink           → game.graphics.window
src/graphics/renderer.ink         → game.graphics.renderer
src/network/http/client.ink       → game.network.http.client
```

`.ink` 扩展名不进入 module 名称。源码根目录自身不额外增加物理目录名；它直接映射为清单指定的根 package。

## 3. Package 是纯命名空间

v0 中 package 和 subpackage 只组织名称层级及子 package/module，不直接拥有函数、类型、全局绑定或 module 生命周期代码。

Ink 不定义 `__init__.ink`、隐式 package 初始化文件或扫描整个目录后自动执行的 package body。生命周期属于具体 module。

因此，如果：

```text
src/graphics/          package game.graphics
src/graphics/window.ink module game.graphics.window
```

则 `game.graphics` 不能作为 module 导入；`game.graphics.window` 可以导入。导入一个 module 不会隐式导入同 package 中的其他 module。

需要聚合公开接口时，应显式创建普通 module，例如：

```text
src/graphics/api.ink → game.graphics.api
```

## 4. 一个文件一个 Module

每个 `.ink` 文件都是独立 module 和独立 Parser 输入。两个物理文件不能声明或合并为同一个 module。

这意味着：

- module 内声明顺序就是该文件内的词法顺序；
- 不存在跨文件 partial module 或隐式声明合并顺序；
- module 生命周期、增量编译和缓存都具有明确的文件级边界；
- 文件移动或改名会改变 module 规范身份。

例如：

```text
src/graphics/window.ink → src/ui/window.ink
```

会把：

```text
game.graphics.window.Window
```

改名为：

```text
game.ui.window.Window
```

该变化同时影响导入、反射名称和其他使用规范声明身份的接口。

## 5. 名称合法性

根 package 名、每个 subpackage 目录名和 `.ink` 文件主名都必须是合法 Ink Identifier，并满足 Tokenizer 已确认的 NFC、大小写和不可见字符规则。

规范名称区分大小写。即使宿主文件系统不区分大小写，构建系统也必须：

- 要求导入拼写与规范路径大小写准确一致；
- 拒绝只因大小写不同而冲突的两个目录或源码文件；
- 在所有支持平台上产生相同的 package/module 身份。

物理路径分隔符、盘符、符号链接拼写和源码根之外的绝对路径不进入规范名称。

## 6. Package/Module 同名冲突

同一父 package 中不能同时存在主名相同的 module 文件和 subpackage 目录：

```text
src/graphics.ink
src/graphics/
```

两者都会占用规范名称 `game.graphics`，因此整个 package 布局非法。该冲突在 package/module 发现阶段报告，不能根据某次构建实际使用哪一个来选择。

同一父 package 中的所有直接 subpackage 和 module 主名必须唯一；大小写折叠后在目标文件系统上发生冲突的布局同样必须拒绝。

## 7. Module 导入路径

完整 module 路径使用点分规范名称：

```text
root-package.subpackage*.module
```

例如：

```ink
import game.graphics.window;
from game.graphics.window import Window;
```

第一个段标识根 package，最后一个段必须解析为 `.ink` module 文件，中间各段必须解析为 subpackage 目录。点分路径中不使用物理路径分隔符，也不包含 `.ink`。

Module 议题 02 进一步定义前导点形式的相对导入。相对拼写解析后仍规范化为本节定义的绝对 module 路径，不能改变 module 的规范完全限定名称。

## 8. 外部 Package

每个外部依赖向当前构建注册自己的根 package 名及源码根。版本、下载地址、缓存路径和本地 checkout 位置不进入源码 module 路径或规范声明名称。

当前构建中可见的根 package 名必须唯一。包清单如何声明依赖别名、版本约束和多个源码根，留给包管理议题确定。

条件导入遵守 Parser 议题 05：未激活候选路径不要求在当前目标环境中解析；激活后再从当前构建已注册的根 package 集合中解析完整 module 路径。

## 9. 生命周期与依赖

只有 module 可以包含普通声明、全局对象以及编译期生成的强类型注册记录。Package 目录没有独立运行时实体或初始化顺序。Ink 不提供用户模块加载或卸载钩子。

导入边建立在 module 之间：

```text
game.main → game.graphics.window
```

Package 层级只构成规范名称和查找结构，不因为目录包含关系自动建立 module 依赖。

## 10. 确认结论

Ink 的包清单确定根 package 和源码根，子目录形成 subpackage，每个 `.ink` 文件形成唯一 module。Package 是不含代码的纯命名空间，module 是声明、导入依赖、生命周期、解析和增量编译的实际单位。源码不包含 `package`/`module` 声明头，完整身份由根 package 名与源码根内相对路径确定。
