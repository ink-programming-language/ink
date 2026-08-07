# Parser 议题 05：源文件与条件导入

> 状态：已确认，2026-08-05 扩展声明区 `comptime` 控制结构；Parser 议题 32 统一为 `TopLevelRegion` 实例并同步固定控制头括号；Parser 议题 36 补全顶层声明集合；Parser 议题 39 为 module 顶层绑定增加访问修饰前缀
> 确认日期：2026-08-03

## 1. 源文件骨架

每个 `.ink` 源文件由零个或多个顶层项和一个 EOF 构成：

```ebnf
source_file = { top_level_item }, end_of_file ;

top_level_item =
    import_declaration
  | top_level_declaration
  | comptime_top_level_item ;

top_level_declaration =
      top_level_binding_declaration
    | function_declaration
    | decorator_declaration
    | class_declaration
    | interface_declaration
    | enum_declaration ;

top_level_binding_declaration =
    top_level_binding_modifier_sequence,
    binding_declaration ;

top_level_binding_modifier_sequence =
    { access_modifier } ;

end_of_file = ? EndOfFile Token ? ;
```

空源文件合法。Trivia 由 Parser 议题 02 的显著 Token 游标隐式跳过，但仍作为 `SourceFile` CST 的真实叶节点完整保留。

Module 议题 01 规定 package/module 身份由包清单、源码根和文件相对路径确定。源码中没有 `package` 或 `module` 声明头；每个 `.ink` 文件准确形成一个 module。因此上述产生式就是完整物理源文件结构。

## 2. 顶层项

顶层允许：

- 普通静态导入；
- 函数、类、接口、枚举、全局绑定和装饰器声明；
- 在声明区域执行的 `comptime` block、条件、匹配和循环结构。

module 顶层绑定可以在 `var` 或 `const` 前写访问修饰符：

```ink
public const api_version = 1;
private var cache: Cache;
const default_visibility = 0;
```

省略访问修饰符仍符合语法；其默认可见性由 module 名称绑定规则决定，不由 Parser 补成某个 Token。`access_modifier` 复用议题 31 的统一产生式。该产生式也包含 `protected`，Parser 会无损保留它以及重复或冲突的修饰符；`protected` 不适用于 module 顶层、重复修饰符和冲突组合均由后续语义检查拒绝。局部绑定不使用 `top_level_binding_declaration`，因此不能写访问前缀。

顶层不允许普通运行时表达式或语句，也不允许单独的 `;` 充当空顶层声明。某种声明自身是否以 `;` 结束由该声明的 EBNF 决定。

顶层名称绑定不依赖声明的文本先后顺序，因此直接 `import` 不要求集中在文件开头。Parser 与导入发现阶段扫描完整 `SourceFile` 后，再进行模块级名称绑定。

## 3. 顶层 `comptime` 结构

顶层声明区复用统一 `comptime` 区域控制。以下产生式是 Parser 议题 32 的 `TopLevelRegion` 标准 EBNF 展开；每个 `{ ... }` 都解析为包含 `top_level_item` 的 `top_level_block`：

```ebnf
comptime_top_level_item =
    "comptime",
    (
        top_level_block
      | top_level_if_tail
      | top_level_match_tail
      | top_level_for_tail
      | top_level_while_tail
    ) ;

top_level_if_tail =
    "if", "(", if_condition, ")", top_level_block,
    [ "else", ( top_level_block | top_level_if_tail ) ] ;

top_level_match_tail =
    "match", "(", expression, ")", "{",
    top_level_match_arm, { top_level_match_arm },
    "}" ;

top_level_match_arm =
    match_arm_pattern, "=>", top_level_block ;

top_level_for_tail =
    "for", "(", for_binding_mode, for_pattern,
    "in", for_source, ")", top_level_block ;

top_level_while_tail =
    "while", "(", while_condition, ")", top_level_block ;

top_level_block = "{", { top_level_item }, "}" ;
```

`if_condition`、`match_arm_pattern`、`for_binding_mode`、`for_pattern`、`for_source` 和 `while_condition` 复用普通语句结构已经确认的非终结符。声明区 `comptime for` 因而同样必须显式写 `var` 或 `const`，且只接受一个名称或 `_`；声明区不会恢复无绑定关键字或逗号多绑定形式。

这些结构不是运行时语句。它们在编译期决定哪些导入和普通声明进入后续模块，或者重复展开源码中静态写出的普通声明。`comptime match` 的每个 arm body 必须是 `top_level_block`，arm 之间不写逗号。

`comptime` 修饰完整条件链；后续分支写作普通 `else if`，由递归的 `top_level_if_tail` 表达，不重复 `comptime`。`comptime while` 是否终止、循环展开次数和生成声明数量都属于语义与资源预算，不改变 Parser 产生式。

Parser 在进入 `parse_top_level_item` 时已经知道当前处于顶层声明区。消费 `comptime` 后，只需按下一个显著 Token `{`、`if`、`match`、`for` 或 `while` 确定分支；它不会先建立 `StatementBlock`，再根据内部内容改判为声明块。

普通 `comptime expression` 仍是表达式结构，不会因此成为顶层声明。`comptime generate();` 之类的表达式语句在 module 或类型成员 declaration context 中非法；需要条件选择或重复声明时，必须把符合当前区域 EBNF 的普通声明直接写在本节控制结构的 body 中。Ink v0 不提供 `field(...)`、动态声明名称或其他声明构造表达式。

## 4. 条件导入示例

```ink
comptime if (target.os == Os.windows) {
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

在执行顶层 `comptime` 前，编译器遍历当前源文件 CST，并递归进入所有 `comptime_top_level_item` 的声明块，收集全部静态 `import` 位置：

```text
source file CST
→ collect syntactic import sites
→ candidate import set
```

候选导入只记录源码中明确写出的静态 package/module 导入目标、别名和控制它的编译期控制路径；此时不要求读取或编译候选模块。循环重复执行同一个静态导入位置不会产生新的动态 module path。

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
→ execute declaration-region comptime controls
→ prune inactive branches and expand declaration iterations
→ resolve and load active imports
→ recursively process active modules
→ build active dependency graph
→ module name binding and later stages
```

未激活的候选模块：

- 不要求文件或包在当前目标环境中存在；
- 不运行自己的 Tokenizer 或 Parser；
- 不参与名称绑定和类型检查；
- 不初始化模块全局对象，也不安装强类型注册记录；
- 不进入最终链接或反射注册。

递归处理已激活模块是普通依赖图遍历，不是“编译期执行不断生成未知导入”的声明固定点循环。

## 7. 封闭候选集

为了保证候选集合在执行前有限且可发现，`import_declaration` 的 package/module 导入目标必须由后续模块系统语法定义为静态源码结构。

禁止：

```ink
import comptime make_module_path();
comptime generate_import("platform.windows");
```

编译期代码不能通过字符串、反射、文件、网络或其他机制产生源码中不存在的导入位置或 package/module 目标。它只能保留或删除 Parser 已经看到的静态候选导入。

## 8. 导入控制表达式的可用依赖

控制导入的 `comptime if` 条件、`match` 被匹配值、`for` source 和 `while` 条件可以使用：

- 目标平台、架构和 ABI 等目标信息；
- 构建配置和受跟踪编译期输入；
- 当前模块中不依赖该条件导入即可建立的编译期声明；
- 无条件实际依赖已经提供的编译期值。

控制表达式不能读取它正在决定是否导入的模块：

```ink
comptime if (platform.windows.is_available()) {
    import platform.windows;
}
```

这里在 `platform.windows` 激活前无法解析条件中的名称，形成导入选择依赖环，因此非法。其他顶层 `comptime` 控制表达式或多个结构之间如果形成同类循环也非法。

## 9. 依赖环与缓存

模块依赖环只根据激活后的实际依赖图判断；只存在于未选中分支中的候选边不构成当前构建的依赖环。

顶层 `comptime` 控制结果属于模块构建缓存键。目标配置或受跟踪编译期输入改变并导致选择不同模块时，相关模块必须重新建立实际依赖图，不能复用旧选择对应的名称绑定或 InkIR。

## 10. CST 与错误恢复

`SourceFile` CST 覆盖从第一个 Token 到 EOF 的完整 Token 流。顶层无法归类的合法 Token 按 Parser 议题 03 放入 `ErrorNode`，并同步到下一个可能的顶层项起始 Token或 EOF。

顶层编译期结构使用 Parser 议题 32 的统一 `ComptimeBlockControl`、`ComptimeIfControl`、`ComptimeMatchControl`、`ComptimeForControl` 和 `ComptimeWhileControl`，并以父节点或 `RegionKind::TopLevel` 确定 body 必须是 `TopLevelBlock`。不建立具有独立语义的 `ComptimeTopLevelIf` 等节点，也不能把 body 降成 `StatementBlock` 或丢失准确 block kind。

消费 `comptime` 后若下一个显著 Token 不是 `{`、`if`、`match`、`for` 或 `while`，Parser 建立错误节点并同步到下一个顶层 item。`match` arm 缺少 `=>` 或 declaration block、`for` 缺少 `var`/`const`、pattern、`in` 或 source、`while` 缺少 condition 时，分别复用对应普通结构的局部恢复规则，但恢复出的 block 仍必须是 `TopLevelBlock`。

词法失败的文件遵守 Tokenizer 议题 10，不进入 Parser，因此也不产生本议题的 `SourceFile` CST。

## 11. 确认结论

Ink 源文件是顶层项序列，不含 package/module 声明头，也不要求导入集中在文件开头。顶层声明区实例化 Parser 议题 32 的统一区域控制，支持 `comptime {}`、`comptime if`、`comptime match`、`comptime for` 和 `comptime while`；每个 body 都是 `top_level_block`，不是 `statement_block`。它们可以控制普通声明和静态写出的导入。编译器在执行前收集有限候选导入集合，执行控制结构后只加载激活模块；编译期代码不能计算 module 导入路径或产生新的导入位置，因此模块发现不需要加入声明展开固定点循环。
