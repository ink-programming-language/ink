# 议题 63：Ink v0 不提供字符串到代码生成

> 状态：已确认，议题 67 确认只允许选择或重复静态源码声明，Parser 议题 05 补充封闭候选集条件导入，Parser 议题 40 定义 class 类型表达式
> 确认日期：2026-08-02

## 1. 不把编译期字符串重新解析为源码

Ink v0 不提供把编译期字符串重新送回 lexer、parser 或当前词法环境执行的语言能力，包括但不限于：

```ink
eval(source_string);                    // 不提供
emit_source(source_string);             // 不提供
syntax.parse_declarations(source_string); // 不提供
```

编译期字符串始终是普通数据。即使字符串内容恰好符合 Ink 语法，编译器也不会自动把它解释成表达式、语句、声明、类型、模块或导入。

`comptime` 只要求普通 Ink 代码在编译期执行，不会建立从执行中的 InkIR 返回源码解析器的任意文本入口。

## 2. 不引入 C 预处理器式 token 粘贴

Ink v0 不提供 C/C++ 宏中的 `##`、字符串化、文本宏替换或独立预处理阶段：

```text
prefix ## name   // 不存在
```

函数体中的编译期条件不能通过 token 拼接改变已经解析的语法结构。泛型、反射和代码生成统一发生在议题 61 的 Staged InkIR 展开模型中，不再建立另一套先于 parser 的文本元编程语言。

## 3. 不提供源码 quote、splice 和局部 `eval`

Ink v0 暂不提供以下语法或等价能力：

- 把任意源码片段 quote 成可修改语法树；
- 把编译期字符串 splice 到源码 token 流；
- 在当前函数局部作用域中解析并执行新语句；
- 通过字符串生成新的 `import` 或改变模块依赖图；
- 通过原始 AST/token API 改写已解析函数体。

这避免为名称卫生、局部变量捕获、源码位置、访问权限、IDE 重构和动态依赖图同时建立复杂规则。

### 3.1 静态候选集中的条件导入

源码中直接写出的静态 `import` 可以位于顶层 `comptime if` 分支中：

```ink
comptime if (target.os == Os.windows) {
    import platform.windows;
} else {
    import platform.linux;
}
```

这不属于字符串到代码，也不属于动态生成 `import`。Parser 在执行 `comptime` 前已经看到两个完整的静态模块路径，因此候选集合固定为：

```text
platform.windows
platform.linux
```

编译器先收集候选导入位置，再求值控制分支；只有保留分支中的导入成为实际依赖，未选中的候选模块不加载、不解析、不参与名称绑定、模块生命周期或代码生成。

该能力只允许从源码中有限、静态可枚举的候选导入中进行选择。仍然禁止：

- 从字符串、文件内容或网络内容计算模块路径；
- 由任何编译期机制创建源码中不存在的 `import`；
- 用反射枚举结果拼接新的模块路径；
- 让导入条件依赖它正在决定是否导入的模块。

导入条件可以使用目标信息、构建配置、当前模块的可用编译期声明以及无条件依赖提供的编译期值。影响选择的外部编译期输入必须遵守议题 61 的输入跟踪与可重复构建规则。

因此本议题所称的“动态 `import`”是运行时导入、计算模块路径或生成新导入位置，不包括从源码静态候选集合中执行编译期分支选择。

## 4. 保留 class 类型表达式和声明区域控制

本议题不撤销 Parser 议题 40 的 class 类型表达式，也不撤销声明区的 `comptime if`、`match`、`for` 和 `while`。它们只操作 Parser 已经从源码中建立的结构：

```ink
return class {
    comptime if (EnableValue) {
        var value: FieldType;
    }
};
```

字段名 `value` 和类型引用 `FieldType` 都真实存在于源码 Token 流中。编译期控制可以决定声明是否保留或重复展开，但不能构造新的字段名、函数名、枚举分支名或语法片段。

Ink v0 不提供以下形式：

```ink
field(name = dynamic_name, type = FieldType);
function(name = dynamic_name, ...);
enum_variant(name = dynamic_name, ...);
```

## 5. 不提供 Identifier splice

反射可以返回字段或函数的名称，编译期代码也可以把名称当作普通数据比较和记录，但名称值不能替换声明 EBNF 中的 `identifier` Token。

```ink
const reflected_name = comptime reflect(User).fields[0].name;

class Copy {
    var reflected_name: i32; // 名称就是源码拼写 reflected_name
}
```

第二个 `reflected_name` 不会被替换成第一个绑定保存的名称。Ink v0 不提供：

- identifier interpolation；
- identifier splice；
- token paste；
- 把字符串或 `Identifier` 元值隐式解释为声明名称；
- 根据反射名称创建新成员。

将来若确实需要动态声明名称，必须统一设计带卫生和作用域规则的机制，不能由 `Identifier.from(...)` 或某个字段专用函数暗中引入。

## 6. 静态声明展开继续遵守固定点规则

声明区域控制选中或重复的普通源码声明仍在议题 61 的固定点循环中处理：

```text
selected or expanded source declarations
→ access checking
→ name binding
→ type checking
→ layout dependency checking
→ InkIR verification
→ next elaboration round
```

展开结果不能立即改变当前解释器调用栈或当前正在遍历的声明快照。本轮提交并验证的声明从后续轮次开始可见；重复名称、无限展开和布局环继续是编译错误。

## 7. 权限和来源

声明区域控制遵守议题 20 的词法权限规则，不能通过反射值、循环元素或输出位置取得目标模块的私有访问权。

每个被选择或重复展开的声明继续保存真实源码位置、控制路径、对应泛型实例和编译期调用栈。诊断引用这些真实来源，不伪造一段从未存在的动态源码。

## 8. 工具和安全收益

不提供字符串到代码可以避免：

- 由转义错误或外部字符串造成的源码注入；
- 原始字符串中的意外名称捕获；
- 生成代码绕过词法访问控制；
- 文件或网络内容静默改变模块依赖图；
- 无法由 IDE 查找、重命名和分析的隐藏源码；
- 每轮固定点重新启动 lexer/parser 的额外成本；
- 为虚拟源码、文本 source map 和嵌套解析错误建立另一套协议。

编译期文件和网络读取仍可产生普通数据，但这些数据不能转成声明名称或源码结构。读取到的内容不会仅因位于 `comptime` 上下文而获得代码权限。

## 9. 源码分发与实现

源码分发包只包含作者实际提供的 Ink 源码和编译期数据资源。构建产物可以缓存声明区域控制的选择与展开结果，但不需要保存或重放动态生成的虚拟源码。

LLVM 不受本议题影响。前端完成所有 class 类型表达式、声明选择、展开和验证后，仍只向 LLVM 后端交付 `Closed InkIR[target]`。

## 10. 后续兼容性

未来如果确实需要动态声明名称或当前规则无法表达的声明，可以独立讨论带卫生规则的 identifier splice 或静态语法模板系统。它不能由本议题自动推导，也不能以兼容名义让普通字符串获得代码解释语义。

即使未来增加 quote，`eval(string)`、动态 `import` 和当前局部作用域文本执行仍可继续保持禁止。
