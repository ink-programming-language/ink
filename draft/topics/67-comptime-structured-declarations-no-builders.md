# 议题 67：`comptime` 只选择和展开源码声明，不公开 Builder

> 状态：已确认，v0 不提供 `field(...)`、动态声明名称或其他声明构造器；Parser 议题 32 统一区域控制，Parser 议题 40 定义 class 类型表达式
> 确认日期：2026-08-02

## 1. 不建立第二套声明构造语言

Ink 不提供用户可见的 `TypeBuilder`、`FunctionBuilder`、`DeclarationGroup` 或手动 `begin`、`finish`、`commit` 协议，也不提供伪装成调用的声明构造形式：

```ink
field(name = dynamic_name, type = DynamicType); // v0 不支持
function(name = dynamic_name, ...);              // v0 不支持
enum_variant(name = dynamic_name, ...);          // v0 不支持
```

`comptime` declaration region 只选择或重复源码中已经写出的普通 Ink 声明。编译器内部仍可使用暂定声明图、事务和 verifier，但这些不是语言 API，也不能被用户代码观察或控制。

## 2. Class 类型表达式直接返回 `type`

编译期函数可以通过 Parser 议题 40 的普通 class 类型表达式产生闭合名义类型：

```ink
func make_pair<
    First: type,
    Second: type
>() -> type {
    return class {
        var first: First;
        var second: Second;
    };
}
```

调用者取得正常的编译期 `type` 值：

```ink
const PairType: type = make_pair::<i32, String>();
```

`class { ... }` 复用普通 class 的完整 Parser，只在表达式上下文把类名改为可选。成员名称 `first`、`second` 必须真实写在源码中，不能由字符串、反射或循环元素替换。

## 3. 声明区直接使用 `comptime if`

编译期条件可以决定源码中某项普通声明是否存在：

```ink
class Player {
    var health: i32;

    comptime if (build.mode == BuildMode.debug) {
        var debug_id: u64;

        func dump_debug() {
            // ...
        }
    }
}
```

Release 构建中，未选择的字段和函数不进入成员集合、对象布局或反射。声明名称仍来自源码 Token；条件只决定声明是否保留。

同一个声明容器中最终产生两个相同名称或相同完整函数签名的声明时，继续按照普通重定义规则处理。编译期条件不能把重复声明变成隐式重载选择机制。

## 4. `comptime for` 只重复静态写出的声明

编译期循环可以让一个源码声明针对不同编译期值展开多次。例如展开同名但参数类型不同的普通函数重载：

```ink
class Encoder {
    comptime for (const Element in (i32, String, bool)) {
        func encode(value: Element) -> String {
            return encode_value(value);
        }
    }
}
```

循环变量 `Element` 可以进入类型位置、初始化表达式、attribute 参数或函数体的 Partial Evaluation，但声明名称 `encode` 必须是源码中的真实 Identifier。

以下形式不属于 v0：

```ink
comptime for (const source_field in reflect(Source).fields) {
    field(name = source_field.name, type = source_field.type);
}
```

因此 v0 不能通过反射自动复制任意类型的字段集合，也不能根据字符串批量生成 getter 名称。需要该能力时，未来必须统一设计 identifier splice 或卫生声明模板，不能只给字段增加特殊调用。

## 5. 声明区域统一复用 `comptime` 控制

module、class、interface 和 enum declaration region 继续统一接受：

```text
comptime { declaration items }
comptime if (condition) { declaration items }
comptime for (binding_mode item in source) { declaration items }
comptime while (condition) { declaration items }
```

Parser 议题 32 使用同一个 `ComptimeRegionControl`，由外层 `RegionKind` 决定 body 可以包含哪些普通声明。每个 declaration item 必须符合当前区域已有的 EBNF；declaration block 不接受表达式语句、赋值、`return`、`break` 或 `continue`。

例如 class member region 可以按编译期布尔条件选择静态字段：

```ink
class Storage<T: type, Inline: bool> {
    comptime if (Inline) {
        var value: T;
    } else {
        var value: T*;
    }
}
```

声明区 `comptime while` 同样只能重复其 body 中静态写出的声明。循环是否终止、最终是否产生重复声明以及展开预算属于后续语义检查，不增加声明构造表达式。

## 6. 名称必须来自源码 Token

普通声明 EBNF 中的 `identifier` 必须对应源文件中的真实 Identifier Token：

```ink
var value: T;
func encode(value: T) -> String;
class Generated {}
```

编译期字符串、反射得到的名称和 `Identifier` 元值都不能直接占据该 Token 位置：

```ink
var reflect(T).name: T; // 不符合普通字段语法
```

Ink v0 不提供 identifier interpolation、token paste、identifier splice 或“返回 Identifier 即声明名称”的隐式规则。反射名称仍可作为普通编译期数据比较、记录或传入 attribute，但不能改变已经解析完成的声明结构。

## 7. 普通函数体由 Partial Evaluation 残留 InkIR

函数实现继续使用普通 Ink 代码。编译期参数、反射值和 `comptime` 控制在 Partial Evaluation 中执行，运行时输入相关部分自动残留为 InkIR：

```ink
func offset<N: i32>(value: i32) -> i32 {
    return value + N;
}
```

用户不需要也不能手动创建基本块、SSA 值或 LLVM 指令。需要底层控制流时直接使用普通 `if`、循环和其他 Ink 语句。

## 8. 自递归 class 类型表达式

class 类型表达式可以提供只在自身类体中可见的局部名称：

```ink
func make_node<T: type>() -> type {
    return class Node {
        var value: T;
        var next: Node*;
    };
}
```

`Node` 不进入外层函数或 module 作用域。按值包含自身形成无限布局、指针间接递归、名义类型身份和实例缓存均由正常类型语义处理。

## 9. 声明收集与固定点

一个 declaration region 的 `comptime` 控制执行后，编译器收集被选择或展开的普通源码声明，再统一进入后续阶段：

```text
execute declaration-region comptime controls
→ collect selected or expanded source declarations
→ bind, type-check and verify
→ commit
→ expose through the next elaboration snapshot
```

同一轮执行看到确定的声明快照；本轮选择或展开的声明不会在正在遍历的同一轮中途加入反射枚举。编译器内部可以事务化提交，但用户不获得 Builder 或手动提交接口。

## 10. 展开身份与确定性

重复展开的声明身份至少由以下信息确定：

```text
source declaration identity
+ closed generic arguments
+ canonical comptime control path
+ canonical loop element or key
+ target configuration when selection depends on target
```

编译器不能仅按“本轮第几个声明”分配可观察身份。相同控制路径产生重复完整签名时按普通冲突处理；改变无关遍历顺序不应把旧身份静默分配给另一项。

## 11. 权限

被选择或展开的声明保留其源码词法模块和容器权限。`comptime` 控制不能通过反射值、循环元素或输出位置取得其他模块的私有访问能力。

普通访问修饰符、覆盖检查、字段布局和反射可见性继续作用于最终保留的每一项声明。未选中分支不进入后续名称绑定和类型检查，但仍必须词法和语法正确。

## 12. v0 只产生闭合结果

返回类型的编译期函数本身可以具有编译期参数，但一次执行返回的 class 类型表达式以及最终保留的函数和成员必须能够在固定点结束前闭合：

```ink
make_pair::<i32, String>() -> closed type
```

Ink v0 不通过动态声明构造器创建新的开放泛型形参列表。用户需要开放泛型时，直接在源码中声明普通泛型，再让其闭合实例内部执行 `comptime` 控制。

## 13. 实现边界

编译器内部可以把 declaration-region 控制 lowering 到私有的暂定声明图、选择记录和验证事务。该实现层不构成标准库 API、反射对象布局或源码兼容承诺。

LLVM 只接收固定点结束后的 `Closed InkIR[target]`，不理解 class 类型表达式、声明区域控制或编译器内部事务。

## 14. 结论

Ink v0 的编译期声明能力完全建立在普通源码声明之上：`comptime if`、`for` 和 `while` 只选择或重复当前区域 EBNF 已经允许的声明。字段、函数、类型和枚举分支的名称必须真实写在源码中；语言不提供 `field(...)`、`function(...)`、动态声明名称、identifier splice 或公开 Builder。class 类型表达式仍可产生新的闭合名义类型，但其成员同样使用普通 class BNF。
