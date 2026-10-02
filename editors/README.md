# Ink 的 JetBrains 编辑器支持

这套配置为本仓库的 Ink 编程语言提供 `.ink` 文件语法高亮和代码片段展开，适用于支持 TextMate Bundles 与 Live Templates 的 CLion / IntelliJ IDEA。它与 Inkle 的 Ink 叙事脚本语言无关；bundle 显示名称是 `Ink (programming language)`，根 scope 是 `source.inklang`。

安装只需导入仓库里的配置文件，不需要编译 Ink 或安装 Node.js。本目录不会自动修改 IDE 设置。

## 安装语法高亮

1. 打开 IDE 的 **Settings → Editor → TextMate Bundles**（Windows / Linux 可按 `Ctrl+Alt+S`）。如果没有此设置页，在 **Plugins → Installed** 中检查并启用 JetBrains 的 **TextMate Bundles** 插件，按 IDE 提示重启。
2. 点击 **+**，选择完整的 `editors/textmate/Ink.tmbundle` 目录；不要选择里面的 `info.plist` 或 `Syntaxes/Ink.tmLanguage` 文件。
3. 应用设置，然后重新打开一个 `.ink` 文件。bundle 列表应出现 **Ink (programming language)**。IDE 根据当前配色方案显示关键字、注释、字符串等，不固定具体颜色。

例如本机仓库在 `D:\github\ink` 时，导入目录为 `D:\github\ink\editors\textmate\Ink.tmbundle`。[JetBrains 的 TextMate 安装说明](https://www.jetbrains.com/help/clion/tutorial-using-textmate-bundles.html)给出了同样的目录导入方式。可打开 [`textmate/example.ink`](textmate/example.ink) 检查关键字、Unicode 名称、注释、原始字符串、三引号字符串和转义的颜色；这个文件用于高亮及语法验证，不是语义功能测试。

如果 `.ink` 仍显示为纯文本或另一种语言，检查 **Settings → Editor → File Types** 是否将 `*.ink` 手动分配给其他类型，以及是否启用了另一个处理 `.ink` 的 bundle / 语言插件。先解除相应冲突，再重新打开文件；IDE 的专用语言插件可能优先于 TextMate bundle。本仓库不自动覆盖已有文件关联。

## 安装 Live Templates

模板文件为 [`jetbrains/live-templates/Ink.xml`](jetbrains/live-templates/Ink.xml)。这是模板组 XML，不是 **Import Settings** 使用的设置压缩包，也不是 IDE 插件。

1. 在 **Help → Diagnostic Tools → Special Files and Folders** 中查看当前 IDE 的**配置目录**。通常 Windows 为 `%APPDATA%\JetBrains\CLion<版本>` 或 `%APPDATA%\JetBrains\IntelliJIdea<版本>`；版本、产品或自定义配置可能使路径不同，以当前 IDE 为准。可参考 [JetBrains 的配置目录说明](https://www.jetbrains.com/help/clion/directories-used-by-the-ide-to-store-settings-caches-plugins-and-logs.html)。
2. 退出使用该配置目录的 IDE，避免退出时覆盖刚复制的设置。在配置目录下创建 `templates` 文件夹（若不存在）。
3. 将仓库中的 `Ink.xml` 复制到 `<IDE 配置目录>/templates/Ink.xml`。如果目标已有同名文件，先备份并合并需要保留的模板，不要直接覆盖自定义内容。不要复制到项目的 `.idea` 目录或 IDE 安装目录。
4. 重新打开 IDE，进入 **Settings → Editor → Live Templates**，确认出现 **Ink** 组以及下面的 8 个模板。

启用 Backup and Sync / Settings Sync 时，请留意同步内容覆盖本地模板的情况；某些版本将同步模板保存在配置目录的 `settingsSync/templates` 下。以设置页实际显示的模板为准，必要时使用 IDE 自带的模板导入或复制功能。[JetBrains 的共享模板说明](https://www.jetbrains.com/help/clion/sharing-live-templates.html)解释了模板目录和同步差异。

## 使用代码片段

在 `.ink` 文件中输入完整缩写并按 `Tab`，然后继续按 `Tab` 在变量占位处移动；最后光标停在代码块内。变量的重复引用会一起更新，例如修改 `inkfor` 的循环变量会同步修改条件和自增部分。

| 缩写 | 展开内容 | 可编辑项 |
| --- | --- | --- |
| `inkmain` | `func main(): i32`，含 `return 0;` | 函数体 |
| `inkfunc` | 函数定义 | 函数名、参数列表、返回类型 |
| `inkif` | `if` 代码块 | 条件，默认 `true` |
| `inkife` | `if / else` 代码块 | 条件，默认 `true` |
| `inkwhile` | `while` 代码块 | 条件，默认 `false` |
| `inkfor` | `for (var I: i32 = 0; I < 10; I++)` | 变量名、上界 |
| `inkforin` | `for (Item in Items)` | 元素名、集合表达式 |
| `inkct` | `comptime` 代码块 | 编译期语句 |

模板使用 Allman 大括号和 2 个空格缩进，并关闭展开后的自动重新格式化。`inkfunc` 的参数列表可以留空，或填写 `Value: i32` 等完整参数；若将返回类型改为非 `void`，需自己填写相应 `return`。

例如，输入 `inkfor` 并保留默认值后得到：

```ink
for (var I: i32 = 0; I < 10; I++)
{
  // 在这里填写循环体
}
```

模板上下文使用 IntelliJ 平台已有的 **Everywhere**（XML ID 为 `OTHER`），因此其他文件、字符串和注释中也可能触发。`ink` 前缀用于降低冲突；单独导入 XML 无法将这些模板可靠地限制为本仓库的 Ink scope。如未展开，检查模板是否启用、**Applicable in** 是否包含 **Everywhere**，以及 **Expand with** 是否为 `Tab`；补全弹窗抢占按键时可先关闭弹窗再按 `Tab`。[Live Templates XML 规范](https://plugins.jetbrains.com/docs/intellij/live-templates-configuration-file.html)说明了展开键、变量和上下文设置。

## 能力范围与验证

TextMate 基于文本规则高亮；Live Templates 提供可编辑的固定代码骨架。这套配置不提供语义补全、类型检查、参数提示、跳转定义、重命名或编译错误提示，也不修改编译器接受的语言规则。函数名和内建类型按拼写和相邻文本着色，不解析作用域或名称遮蔽。

Unicode 标识符边界使用正则引擎提供的字母、数字、组合标记和连接符类别作近似判断，不复制或生成 Ink 的 XID 表，也不执行 NFC 校验。Unicode 15.1 的精确合法性、转义的标量值和名称、字符字面量长度仍由编译器验证。部分明显错误的数字和转义带有 `invalid` scope，其显示样式由主题决定。

模板与 [`Ink-grammar-Rules.bnf`](../docs/Ink-grammar-Rules.bnf) 对齐，但语法合法不等于当前语义分析器已经实现全部功能。例如 `inkforin` 提供文法骨架，集合语义是否可执行仍以当前编译器支持为准；默认 `Items` 也需要替换成实际存在的表达式。高亮有颜色同样不表示程序合法。

维护 TextMate 规则时，在 `editors/textmate` 目录执行 `npm ci`、`npm test`；这些是开发验证步骤，使用者导入 bundle 不需要 Node.js。模板修改后应检查 XML，并将占位符替换为默认文本、移除 `$END$`，使用 `ink-parse` 验证展开代码的语法。

更新仓库后，可在 TextMate Bundles 设置中重新加载该目录；Live Templates 需要重新复制 XML 或在 IDE 中合并修改。卸载时从 TextMate Bundles 列表移除该 bundle，并在 Live Templates 设置中删除或禁用 **Ink** 组即可。
