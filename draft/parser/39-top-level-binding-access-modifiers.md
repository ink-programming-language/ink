# Parser 议题 39：Module 顶层绑定访问修饰符

> 状态：已确认，module 顶层绑定增加可选访问修饰前缀，局部绑定保持不变
> 确认日期：2026-08-06

## 1. 表面形式

module 顶层的 `var`、`const` 绑定可以在声明关键字之前写访问修饰符：

```ink
public const api_version = 1;
private var cache: Cache;
const default_visibility = 0;
```

访问修饰符必须位于 `var` 或 `const` 之前。省略访问修饰符仍然符合语法；省略时采用什么默认可见性不由 Parser 决定。

## 2. EBNF

```ebnf
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

access_modifier =
      "public"
    | "protected"
    | "private" ;
```

`binding_declaration` 继续复用议题 10，内部仍必须以 `var` 或 `const` 开头。`access_modifier` 与议题 31 的函数声明使用同一个产生式；这里重列它只是为了完整展示 wrapper 的输入。

使用 `{ access_modifier }` 而不是把可见性写进 `var_declaration` 和 `const_declaration`，使核心绑定语法仍可直接复用于局部声明。零个修饰符对应没有显式访问前缀。

## 3. Parser 与语义边界

module 顶层真正有意义的访问形式是 `public` 和 `private`。统一 `access_modifier` 产生式还会让 Parser 接受并保存 `protected`，也允许先形成含重复或冲突修饰符的 CST：

```ink
protected var value = 0;
public private const version = 1;
```

`protected` 不适用于 module 顶层，重复或冲突的访问修饰符也非法；这些判断需要声明上下文和修饰符集合，统一交给语义分析。Parser 不静默删除、合并或改写任何修饰符。

本议题不为全局绑定增加 attribute、decorator application、`static`、`extern`、`thread_local` 或其他新前缀。

## 4. 局部与条件顶层区域

函数体内的局部绑定不经过 `top_level_binding_declaration`，因此仍只能从 `var` 或 `const` 开始：

```ink
func update() {
    var count = 0;
    private var hidden = 0; // 语法非法
}
```

顶层 `comptime` block、条件和循环的 body 仍由议题 05、32 递归解析 `top_level_item`，所以无需第二套产生式即可使用相同前缀：

```ink
comptime if (build.debug) {
    private var debug_cache: Cache;
}
```

## 5. CST 与递归下降分派

Parser 在顶层收集零个或多个 `access_modifier` 后，若下一个显著 Token 是 `var` 或 `const`，建立绑定 wrapper：

```text
TopLevelBindingDeclaration
├─ AccessModifier*
└─ NamedBindingDeclaration | TupleDestructuringDeclaration
```

没有访问前缀时，顶层上下文可以从 `var` 或 `const` 直接进入同一 wrapper。局部上下文则直接进入绑定核心，不建立访问修饰序列。Parser 不需要根据绑定名称、初始化器或最终导出状态回溯选择声明种类。

## 6. 结论

Ink 允许 module 顶层绑定写成 `public const ...;` 或 `private var ...;`，也允许省略访问修饰符。语法通过一个只用于顶层的 wrapper 复用既有 `binding_declaration`；局部绑定保持 `var`/`const` 起始规则不变。默认可见性、`protected` 在 module 顶层不适用以及重复或冲突修饰符均由语义分析决定。
