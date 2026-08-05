# 议题 67：`comptime` 直接生成结构化声明，不公开 Builder

> 状态：已确认，议题 71 补充使用编译期元组组织声明输入；Parser 议题 32 统一区域控制
> 确认日期：2026-08-02

## 1. 不建立第二套用户可见生成器对象

Ink 不提供用户可见的 `TypeBuilder`、`FunctionBuilder`、`DeclarationGroup` 或手动 `begin`、`finish`、`commit` 协议。类型和声明生成就是统一 `comptime` 区域控制把选中或展开结果输出到声明区域的结果，不是独立元编程运行时，也不是类专用的第二套 `comptime`。

```text
comptime       决定代码在编译期执行
type           表示编译期闭合类型值
declaration    表示编译期声明值
Partial Evaluation 产生残留 InkIR
```

编译器内部仍然可以使用 builder、声明事务和 verifier 实现这些语义，但它们不是语言 API，也不能被用户代码观察或控制。

## 2. 类型表达式直接返回 `type`

编译期函数可以使用普通类型表达式产生闭合名义类型：

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

调用：

```ink
const PairType: type =
    make_pair<i32, String>();
```

`class { ... }` 的结果是新的名义 `type`，不是按字段集合自动相等的结构类型。相同生成表达式和相同规范化编译期实参重复求值时，由实例缓存得到同一类型身份；不同生成表达式即使字段相同也产生不同名义类型。

## 3. 声明区直接使用 `comptime if`

编译期条件可以决定类型或模块声明区中是否存在某项结构化声明：

```ink
func make_storage<T: type>() -> type {
    return class {
        comptime if (T != void) {
            var value: T;
        }
    };
}
```

未选择分支不产生字段、方法或布局依赖。选择分支中的声明按照普通语义完成名称绑定、访问检查、类型检查和布局验证。

因此可以按受跟踪的构建配置提供仅在 Debug 构建存在的成员：

```ink
class Player {
    var health: i32;

    comptime if (build.mode == .debug) {
        var debug_id: u64;

        func dump_debug() {
            // ...
        }
    }
}
```

Release 构建中这些成员不进入成员集合、对象布局或反射。引用条件成员的代码也必须位于能够消除 Release 路径的 `comptime` 控制中。构建模式及其他影响声明生成的输入必须进入模块缓存键、类型布局记录和 ABI 兼容性判断；具有条件字段的类型不能在不同构建配置产物之间按同一稳定布局直接交换。`build.mode` 的完整构建上下文 API 由独立议题定义。

同一个声明容器中最终产生两个相同名称或相同完整函数签名的声明时，继续按照普通重定义规则报错；编译期条件不能把重复声明变成隐式重载选择机制。

## 4. 声明区直接使用 `comptime for`

编译期循环可以重复产生结构化声明：

```ink
func wrap_fields<
    Wrapper: GenericTypeDecl,
    Source: type
>() -> type {
    return class {
        comptime for (const source_field in reflect(Source).fields) {
            field(
                name = Identifier.from(source_field.name),
                type = Wrapper.instantiate<source_field.type>(),
                visibility = source_field.visibility
            );
        }
    };
}
```

`field(...)` 是只能出现在声明上下文中的结构化字段声明形式。它接收已经类型化的 `Identifier`、`type`、可见性和其他语义值，不解析字符串源码，也不返回可变 Builder。

议题 71 允许生成器使用现有元组保存若干 `(Identifier, type)` 等结构化编译期输入，再由 `comptime for` 逐项产生 `field(...)`。元组只是普通不可擦除的编译期数据，不成为另一套声明 Builder。

需要动态组成声明名称时，只能使用议题 63 的单标识符验证能力。字符串中的括号、标点、类型语法或函数体不能被 `field(...)` 解释成代码。

## 5. 声明区统一复用全部 `comptime` 控制结构

module、class、interface、enum 及其他声明区域统一接受以下表面形式：

```text
comptime { declaration items }
comptime if (condition) { declaration items }
comptime match (value) { pattern => { declaration items } }
comptime for (binding_mode item in source) { declaration items }
comptime while (condition) { declaration items }
```

Parser 议题 32 把这些形式统一为一个接收 `RegionKind` 的 `ComptimeRegionControl`。Parser 在进入该结构前已经由外层调用者确定 declaration region 种类：module body 使用 `TopLevelRegion`，class body 使用 `ClassMemberRegion`，interface 和 enum 使用各自 MemberRegion。它不会先解析成 `statement_block`，再根据内部 Token 或名称绑定结果改判；区域变化只替换 item parser 和输出 sink，不改变 `comptime` 的阶段语义。

例如，类型成员可以按编译期匹配生成：

```ink
class Storage<T: type> {
    comptime match (reflect(T).kind) {
        .integer => {
            var value: T;
        }

        .record => {
            var value: T*;
        }

        _ => {}
    }
}
```

声明区 `comptime match` 的每个 arm body 都必须使用对应 declaration block，arm 之间不写逗号。声明区 `comptime for` 继续强制 `var` 或 `const` 和单一名称/`_` pattern。声明区 `comptime while` 可以驱动有状态的结构化声明生成，但必须通过条件自然终止，并受执行 fuel 与生成声明数量预算约束：

```ink
class Generated {
    comptime while (generator.has_next()) {
        field(generator.next());
    }
}
```

这些 declaration block 不接受普通赋值、`return`、`break`、`continue`、`throw` 或其他 statement。需要执行普通语句序列时必须位于函数的 `statement_block` 中；声明区只接受该区域本来允许的声明、结构化声明形式以及递归的声明区 `comptime` item。

## 6. 结构化函数和其他声明形式

普通静态名称和签名优先使用普通 Ink 声明语法：

```ink
return class {
    var value: i32;

    func get() const -> i32 {
        return this.value;
    }
};
```

确实需要由反射数据决定函数名称或签名时，语言可以提供与 `field(...)` 同类的结构化 `function(...)` 声明形式。它直接接收参数描述、结果类型和普通 Ink 函数体语义，不暴露可任意修改的裸 InkIR。

结构化枚举分支、属性和其他声明沿用同一原则：每种声明种类由语言定义准确的类型化构成项，不使用通用源码字符串，也不要求所有声明通过一个万能 Builder 对象构造。各声明形式的完整表面语法可以随对应语言模块细化。

## 7. 普通函数体由 Partial Evaluation 产生 InkIR

生成方法或函数的实现仍然写成普通 Ink 代码。编译期参数、反射值和 `comptime if` 在议题 61 的 Partial Evaluation 中执行，运行时输入相关部分自动残留成 InkIR：

```ink
func offset<N: i32>(value: i32) -> i32 {
    return value + N;
}
```

用户不需要也不能为了生成该函数而手动创建基本块、SSA 值或 LLVM 指令。需要底层控制流时直接写普通 `if`、循环、异常和其他 Ink 语句，由正常 lowering 和 verifier 处理。

## 8. 自递归类型

类型表达式可以提供只在自身声明中可见的本地名称，用于递归引用：

```ink
func make_node<T: type>() -> type {
    return class Node {
        var value: T;
        var next: Node*;
    };
}
```

`Node*` 合法，因为指针不要求完成 `Node` 的内联布局。直接按值包含 `Node` 形成无限大小，仍是编译错误：

```ink
return class Node {
    var next: Node; // 编译错误
};
```

本地名称只用于类型表达式内部的名称解析，不自动向外部模块导出一个名为 `Node` 的声明。表达式结果通过返回的 `type` 值使用。

## 9. 相互递归声明

同一个声明级 `comptime` 块中生成的多个具名声明按照普通声明组处理：编译器先收集该组的声明身份，再分析各自定义，因此可以通过指针或其他间接方式相互引用。

编译器内部可以为整组建立原子声明事务；任一声明验证失败时整组不提交。用户不需要手动创建 `DeclarationGroup`，也不能选择只提交一部分已经失败的组。

相互按值包含形成的递归布局环继续由类型布局依赖图统一检测。

## 10. 声明身份与确定性

生成声明身份至少由以下语义来源确定：

```text
generator declaration identity
+ closed generic arguments
+ declaration expression source identity
+ enclosing generated declaration identity
+ canonical comptime loop element/key when repeated
+ target configuration when generation depends on target
```

编译器不能仅按“本轮生成的第几个声明”分配可观察身份。对同一个循环键重复生成同一名称属于冲突；改变无关声明的遍历顺序不应静默把旧类型身份重新分配给另一个声明。

动态 `[reflect]`、热更新或公开名称需要稳定身份时，生成器必须提供可验证的 `Identifier` 和确定循环键。只作为私有闭合类型返回的表达式仍具有编译器规范化的内部名义身份。

## 11. 固定点可见性

类型表达式完成验证后可以把其闭合 `type` 结果返回给当前编译期调用链。全局名称查找和完整反射快照不会在正在遍历的同一轮中途改变：

```text
execute current round
→ collect structured declarations
→ bind, type-check and verify as one transaction
→ commit
→ expose through the next elaboration snapshot
```

这避免编译期循环一边枚举声明、一边让新声明加入当前枚举而产生顺序依赖。持有直接返回 `type` 句柄的当前生成逻辑可以继续构造依赖请求，但最终代码生成仍等待固定点验证完成。

## 12. 权限

类型和声明表达式按照生成代码的词法定义模块取得权限。把表达式结果插入另一个类型、模块或高阶泛型，不会借用目标位置的私有访问能力。

结构化声明必须显式通过正常可见性和覆盖检查。`Identifier`、反射字段偏移或声明句柄都不能绕过议题 20 的访问规则。

## 13. v0 只动态生成闭合声明

生成函数本身可以具有编译期参数，但一次执行产生的类型、函数和成员必须已经闭合：

```ink
make_pair<i32, String>() -> closed type
wrap_fields<Optional, User>() -> closed type
```

Ink v0 不提供在声明表达式中动态创建新的开放泛型形参列表。用户需要开放泛型时，直接在源码中声明普通泛型，并让其闭合实例内部执行结构化生成。

这不限制议题 66 把源码中已有的开放泛型声明作为编译期值传递和实例化。

## 14. 诊断与资源

编译器必须报告：

- 结构化声明参数类型错误；
- 非法或重复 `Identifier`；
- 递归布局环；
- 生成声明重定义；
- 当前声明上下文不允许该声明种类；
- 生成代码访问不可见成员；
- 本轮声明组验证失败；
- 不能收敛的声明生成和实例化；
- 超过编译期声明数量、控制流或 InkIR 预算。

诊断来源链包含生成器、闭合泛型实参、`comptime` 循环元素和结构化声明位置，不伪造不存在的源码字符串。

## 15. 实现边界

编译器内部可以把声明表达式 lowering 到私有 builder、暂定声明图和验证事务。该实现层可以改变，不构成标准库 API、反射对象布局或源码兼容承诺。

LLVM 只接收固定点结束后的 `Closed InkIR[target]`，不理解类型表达式、结构化声明形式或编译器内部 Builder。

## 16. 后续问题

以下内容留给对应语言模块：

- 动态名称 `function(...)` 的完整参数和函数体表面语法；
- 结构化 enum 分支、属性和元数据声明语法；
- 公开生成声明的完全限定名称格式；
- 生成声明参与热更新时的迁移与兼容性分类；
- 是否允许未来生成新的开放泛型声明。
