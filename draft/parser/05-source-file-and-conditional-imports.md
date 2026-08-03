# Parser 议题 05：源文件与条件导入

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 源文件骨架

每个 `.ink` 源文件由零个或多个顶层项和一个 EOF 构成：

```ebnf
source file = { top level item }, end of file ;

top level item =
    import declaration
  | top level declaration
  | comptime top level conditional ;

end of file = ? EndOfFile Token ? ;
```

空源文件合法。Trivia 由 Parser 议题 02 的显著 Token 游标隐式跳过，但仍作为 `SourceFile` CST 的真实叶节点完整保留。

Module 议题 01 规定 package/module 身份由包清单、源码根和文件相对路径确定。源码中没有 `package` 或 `module` 声明头；每个 `.ink` 文件准确形成一个 module。因此上述产生式就是完整物理源文件结构。

## 2. 顶层项

顶层允许：

- 普通静态导入；
- 函数、类型、全局绑定、装饰器及其他已定义的顶层声明；
- 在声明区域执行的 `comptime` 条件结构。

顶层不允许普通运行时表达式或语句，也不允许单独的 `;` 充当空顶层声明。某种声明自身是否以 `;` 结束由该声明的 EBNF 决定。

顶层名称绑定不依赖声明的文本先后顺序，因此直接 `import` 不要求集中在文件开头。Parser 与导入发现阶段扫描完整 `SourceFile` 后，再进行模块级名称绑定。

## 3. 顶层 `if comptime`

顶层条件结构的基本文法为：

```ebnf
comptime top level conditional =
    "if", "comptime", expression, top level block,
    [ "else", ( top level block | comptime top level conditional ) ] ;

top level block = "{", { top level item }, "}" ;
```

该结构不是运行时语句。它在编译期决定分支中的导入和普通声明是否进入后续模块。

`else if comptime` 由递归的 `comptime top level conditional` 表达。是否支持其他顶层编译期循环结构，以及它们的准确 EBNF，由后续独立语法议题确定。

## 4. 条件导入示例

```ink
if comptime target.os == Os.windows {
    import platform.windows;

    func platform_name() -> String {
        return "Windows";
    }
} else {
    import platform.linux;

    func platform_name() -> String {
        return "Linux";
    }
}
```

Parser 构造包含两个分支全部 Token 的无损 CST。当前源文件的所有分支都必须词法和语法正确；未选中分支在编译期执行后从 Staged InkIR 中消除，不参与名称绑定、类型检查或最终代码生成。

## 5. 候选导入收集

在执行顶层 `comptime` 前，编译器遍历当前源文件 CST，收集所有静态 `import` 位置：

```text
source file CST
→ collect syntactic import sites
→ candidate import set
```

候选导入只记录源码中明确写出的静态 package/module 导入目标、别名和控制它的编译期分支路径；此时不要求读取或编译候选模块。

对于前述示例，候选集合为：

```text
platform.windows guarded by target.os == Os.windows
platform.linux   guarded by not(target.os == Os.windows)
```

重复出现的同一路径可以共享模块解析和加载结果，但每个导入位置仍保留独立 CST 身份，以便处理别名、可见性和源码工具。

## 6. 候选依赖与实际依赖

候选导入不是实际模块依赖。编译器求值顶层条件后只激活保留分支中的导入：

```text
parse current source file
→ collect finite candidate import sites
→ evaluate import guards
→ prune inactive branches
→ resolve and load active imports
→ recursively process active modules
→ build active dependency graph
→ module name binding and later stages
```

未激活的候选模块：

- 不要求文件或包在当前目标环境中存在；
- 不运行自己的 Tokenizer 或 Parser；
- 不参与名称绑定和类型检查；
- 不运行模块加载与卸载钩子；
- 不进入最终链接或反射注册。

递归处理已激活模块是普通依赖图遍历，不是“编译期执行不断生成未知导入”的声明固定点循环。

## 7. 封闭候选集

为了保证候选集合在执行前有限且可发现，`import declaration` 的 package/module 导入目标必须由后续模块系统语法定义为静态源码结构。

禁止：

```ink
import comptime make_module_path();
comptime generate_import("platform.windows");
```

编译期代码不能通过字符串、反射、文件、网络或结构化声明构造器产生源码中不存在的导入位置或 package/module 目标。它只能保留或删除 Parser 已经看到的静态候选导入。

## 8. 导入条件的可用依赖

控制条件导入的 `comptime` 表达式可以使用：

- 目标平台、架构和 ABI 等目标信息；
- 构建配置和受跟踪编译期输入；
- 当前模块中不依赖该条件导入即可建立的编译期声明；
- 无条件实际依赖已经提供的编译期值。

条件不能读取它正在决定是否导入的模块：

```ink
if comptime platform.windows.is_available() {
    import platform.windows;
}
```

这里在 `platform.windows` 激活前无法解析条件中的名称，形成导入选择依赖环，因此非法。多个条件之间如果形成同类循环也非法。

## 9. 依赖环与缓存

模块依赖环只根据激活后的实际依赖图判断；只存在于未选中分支中的候选边不构成当前构建的依赖环。

条件导入的求值结果属于模块构建缓存键。目标配置或受跟踪编译期输入改变并导致选择不同模块时，相关模块必须重新建立实际依赖图，不能复用旧选择对应的名称绑定或 InkIR。

## 10. CST 与错误恢复

`SourceFile` CST 覆盖从第一个 Token 到 EOF 的完整 Token 流。顶层无法归类的合法 Token 按 Parser 议题 03 放入 `ErrorNode`，并同步到下一个可能的顶层项起始 Token或 EOF。

词法失败的文件遵守 Tokenizer 议题 10，不进入 Parser，因此也不产生本议题的 `SourceFile` CST。

## 11. 确认结论

Ink 源文件是顶层项序列，不含 package/module 声明头，也不要求导入集中在文件开头。顶层 `if comptime` 可以同时控制普通声明和静态写出的导入。编译器在执行前收集有限候选导入集合，求值条件后只加载激活模块；编译期代码不能计算 module 导入路径或生成新的导入位置，因此模块发现不需要加入声明生成固定点循环。
