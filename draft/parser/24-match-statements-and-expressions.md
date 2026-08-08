# Parser 议题 24：`match` 语句与表达式

> 状态：已确认；2026-08-05 增加 `comptime match` 语句形式，Parser 议题 32 统一区域控制并统一要求被匹配表达式括号；2026-08-08 确认 statement entry 的裸 `match` 与 `comptime match` 不可回滚地提交到结构控制
> 确认日期：2026-08-04

## 1. 两种产生式

Ink 使用同一个 `match` 关键字提供无值语句和有值表达式，两种结构共享被匹配表达式与分支模式，但分支体和结束符不同：

```ebnf
match_statement =
    "match", "(", expression, ")", "{",
    match_statement_arm, { match_statement_arm },
    "}" ;

match_statement_arm =
    match_arm_pattern, "=>", statement ;

match_expression =
    "match", "(", expression, ")", "{",
    match_expression_arm, { match_expression_arm },
    "}" ;

match_expression_arm =
    match_arm_pattern, "=>",
    match_expression_arm_body, "," ;

match_expression_arm_body =
      expression
    | statement_block ;

comptime_match_statement =
    "comptime", match_statement ;
```

`match_arm_pattern` 由议题 23 定义为 `variant_pattern` 或 `wildcard_pattern`。`statement`、`statement_block` 和 `expression` 使用已经确认的对应产生式。两种 `match` 都至少需要一个分支。

## 2. `match_statement`

语句形式用于控制流和副作用，每个分支体准确是一条 `statement`：

```ink
match (state) {
    .idle => wait();

    .running(task) => {
        poll(task);
        log_progress();
    }

    _ => return;
}
```

简单语句使用自己的结尾分号，`statement_block`、嵌套 `if_statement` 或嵌套 `match_statement` 使用自身的结构结束符。分支之间不写逗号，整个 `match_statement` 的最后一个 `}` 后也不写分号。

分支箭头后不能直接放置局部声明，因为声明不是 `statement`。需要声明或多条语句时使用块：

```ink
match (optional) {
    .some(value) => {
        const copy = value;
        use(copy);
    }

    .none => {}
}
```

以下形式非法：

```ink
match (optional) {
    .some(value) => const copy = value;
    .none => {},
}
```

第一处分支体是未包在块中的声明；第二处给语句分支增加了逗号。外层 statement dispatcher 已经根据裸 `match` 起点提交到 `MatchStatement`，所以 `=>` 后是只接受 `statement` 的位置；下一个显著 Token 是 `var` 或 `const` 时，Parser 必须报告“声明需要放入语句块”，保留该分支的原始 Token 并按分支边界恢复。它不得把 `const copy = value;` 回退为限定类型值赋值或表达式语句，也不在错误分支中建立局部绑定。

## 3. `match_expression`

表达式形式在普通表达式位置产生值：

```ink
const count: ptrsize = match (optional_items) {
    .none => 0,
    .some(items) => items.length,
};
```

每个表达式分支都由真实的 `,` 结束，包括最后一个分支。逗号是 `match_expression_arm` 的必需结束 Token，不是普通列表中的可选尾随逗号。

表达式 arm 继续使用完整 `expression`，因此在已经明确选择 `MatchExpression` 的上下文中，以 `const` 开始的前置限定类型值合法：

```ink
const selected: type = match (mode) {
    .readonly => const Data*,
    _ => Data*,
};
```

能够正常完成的分支体必须是 `expression`。`statement_block` 只用于不可能正常完成的分支，例如抛出、返回、跳出或永久 trap：

```ink
const value = match (result) {
    .ok(value) => value,

    .error(error) => {
        log(error);
        throw error;
    },
};
```

块在这里仍是议题 09 的无值 `statement_block`，不会把最后一条表达式变成隐式结果。类型和控制流检查必须证明该块不能正常到达结束 `}`；否则该分支不能满足有值表达式要求：

```ink
const value = match (optional) {
    .some(value) => value,
    .none => {
        log("missing");
    }, // 语义错误：该块能够正常完成，却没有结果值
};
```

所有能够正常完成的表达式分支必须产生兼容类型。只包含不正常完成分支的 `match_expression` 具有 `never` 结果；这些结论属于类型和控制流语义，不改变 Parser 节点形状。

## 4. 语句与表达式的上下文分派

Parser 不使用类型信息、名称绑定或 arm 的结束符选择两种结构。调用者在进入 `match` 前已经确定当前语法上下文：

- 初始化器、赋值右侧、`return`、`throw`、`defer` 操作数、实参和其他必须接收表达式的位置调用 expression Parser，因此解析 `MatchExpression`；
- statement entry 的第一个显著 Token 是 `Keyword(Match)` 时，立即且不可回滚地提交到 `MatchStatement`；
- statement entry 的前两个显著 Token 是 `Keyword(Comptime)`、`Keyword(Match)` 时，立即且不可回滚地提交到 `StatementRegion` 的 `ComptimeMatchControl`；
- 如果需要把整个 `MatchExpression` 作为表达式语句丢弃，必须用括号使 statement entry 先进入普通表达式，例如 `(match (...) { ... });`。

议题 18 的 `statement_expression` 零宽守卫把这项决定直接编码进正式文法：

```ebnf
statement_expression =
    ? next significant Token is neither Keyword(Var), Keyword(Const), nor Keyword(Match), and the next two significant Tokens are not Keyword(Comptime) followed by Keyword(Match) ?,
    expression ;
```

因此下列三种形状在看到第一个 arm 前就已经确定节点种类：

```ink
match (state) {
    .ready => run();
    _ => wait();
} // MatchStatement

(match (state) {
    .ready => 1,
    _ => 0,
}); // ExpressionStatement(MatchExpression)

const code = match (state) {
    .ready => 1,
    _ => 0,
}; // 初始化器中的 MatchExpression
```

选定节点以后，arm 标点只负责验证该节点：

```ink
.ready => run(); // MatchStatement arm
.ready => run(), // MatchExpression arm
```

两种分支结束方式不能在同一个正常完成的 `match` 中混合。语句入口的 `match (state) { .ready => 1, ... };` 始终是带错误逗号和多余外层分号的 `MatchStatement`，不能因为看到了逗号而回退重建为 `MatchExpression`。`match_expression` 唯一允许的非表达式分支是第 3 节规定的、不可能正常完成的 `statement_block`，该分支仍必须在块后写逗号。

## 5. 分支模式

顶层分支只接受议题 23 的枚举分支模式或 `_`：

```ink
.none
.some(value)
.point(x, _)
_
```

裸名称不能成为顶层全匹配绑定：

```ink
match (value) {
    anything => use(anything); // 非法
}
```

剩余分支必须显式写 `_`。分支是否存在、载荷数量和类型是否匹配、分支是否重复、`_` 后是否仍有不可达分支，以及闭合枚举是否穷尽，都由语义检查完成。

v0 不接受 guard、多模式 `|`、范围、字面量、字段或嵌套枚举分支模式：

```ink
.some(value) if (value > 0) => use(value); // 非法：尚无 guard
.red | .blue => use_color();             // 非法：尚无多模式
```

## 6. 求值、作用域与生命周期

被匹配表达式准确求值一次，并在任何分支检查之前完成。随后按照源码分支顺序测试模式，只执行第一个匹配分支；不存在隐式 fallthrough，也不会求值未选择分支的分支体。

每个分支建立独立词法作用域。模式名称只在对应分支体中可见，不会进入后续分支或 `match` 之后的作用域。

如果被匹配表达式产生临时枚举值，该临时值至少存活到整个 `match_statement` 或 `match_expression` 结束。议题 23 的载荷绑定借用活动载荷，并从被匹配对象传播 `T&` 或 `const T&` 访问能力；匹配不会复制、移动或消费载荷。

离开所选分支时，局部对象和 `defer` 按既有逆序清理规则处理。模式绑定本身是非拥有引用，不单独析构，也不会延长枚举或载荷的生命周期。

## 7. 穷尽性与分支顺序

`match_statement` 和 `match_expression` 都必须对被匹配的闭合枚举穷尽。可以逐项列出全部分支，也可以使用 `_` 覆盖尚未出现的分支：

```ink
match (color) {
    .red => handle_red();
    _ => handle_other();
}
```

分支按源码顺序匹配，因此 `_` 覆盖其后的所有分支；在 `_` 后继续列出分支属于不可达模式错误。重复枚举分支同样是语义错误。Parser 仍保留这些分支的完整 CST，不根据枚举声明删除或重排源码。

空分支集合不符合产生式：

```ink
match (value) {} // 非法
```

## 8. `=>`、逗号与分号

Tokenizer 按 Tokenizer 议题 09 为 `=>` 产生相邻的 `Symbol('=')` 和 `Symbol('>')`。Parser 只在两个 Symbol 直接相邻时识别分支箭头：

```ink
.ready => run();  // 合法
.ready = > run(); // 非法
```

箭头两侧可以存在 Trivia，但两个组成字符之间不能存在空格、注释或换行。

标点规则总结为：

```text
match_statement arm  → statement 自己的分号或结构结束符；不加逗号
match_expression arm → 每个 arm 必须以逗号结束
match_statement      → 最后一个 } 后不加分号
match_expression     → 自身不含外层分号，由包含它的声明或表达式语句提供
```

这些标点验证发生在上下文已经选定节点以后，不参与 `MatchStatement` 与 `MatchExpression` 的选型。

## 9. 表达式优先级与后缀

`match_expression` 是议题 14 的 `structured_expression`，因此属于 `postfixable_primary_expression`，可以作为调用、索引和成员访问的后缀基础：

```ink
const length = (match (optional) {
    .none => empty_items,
    .some(items) => items,
}).length;
```

在已经进入表达式上下文时，括号在该例中只用于提高可读性，后缀文法本身不强制它；如果整个后缀表达式位于 statement entry 并直接以 `match` 开始，则议题 18 的结构起点守卫要求先把完整 `match_expression` 括起来。被匹配位置和普通结果分支都接受完整 `expression`，嵌套 `if_expression` 或另一个 `match_expression` 时继续遵守各自定界符和优先级规则。

`match_statement` 不进入表达式优先级，也不能直接作为初始化器、操作数或调用实参。

## 10. `comptime match`

语句形式可以由 `comptime` 直接修饰：

```ink
comptime match (target.arch) {
    .x86_64 => enable_x86_backend();
    .arm64 => enable_arm_backend();
    _ => compile_error("unsupported architecture");
}
```

Parser 建立议题 32 的统一 `ComptimeMatchControl`，在普通函数体中使用 `RegionKind::Statement`，并继续复用完整 `match_statement` 的 header、标点和模式文法。被匹配值能否在编译期求出、只保留哪个分支以及分支中的运行时代码是否残留，由语义阶段决定。

在 declaration region 中保留同一个 `comptime match (value) { ... }` 表面结构。Parser 议题 32 只替换 `RegionRules`，所以每个 arm 的 `=>` 后必须是当前区域的 declaration block，而不是 `statement`。顶层具体 EBNF 展开为 Parser 议题 05 的 `top_level_match_arm`；class、interface、enum 使用对应 member block。所有区域都不在 arm 之间写逗号。

statement entry 的显著 Token 序列 `comptime match` 固定选择上述结构控制，即使后续出现表达式 arm 的逗号也不能回退到 `comptime_expression`。在初始化器等已经要求表达式的位置，`comptime` 可以直接修饰 `match_expression`；圆括号只用于明确其范围：

```ink
const selected = comptime match (target.arch) {
    .x86_64 => x86_value,
    .arm64 => arm_value,
    _ => fallback_value,
};
```

如果要在 statement context 丢弃这个编译期值，必须改变外层起点，例如：

```ink
comptime (match (target.arch) {
    .x86_64 => x86_value,
    .arm64 => arm_value,
    _ => fallback_value,
});
```

`(comptime match (...) { ... });` 同样先由 `(` 建立表达式上下文。

## 11. CST 与错误恢复

CST 使用独立节点保存：

```text
MatchStatement
MatchStatementArm
MatchExpression
MatchExpressionArm
ComptimeMatchControl
```

每个节点按源码顺序保存 `match`、固定左右括号、被匹配表达式、花括号、pattern、组成 `=>` 的两个 Symbol、分支体、表达式分支逗号以及全部 Trivia。Parser 不把两个 Symbol 合并成一个虚构 Token。

恢复不能改变入口已经选定的节点种类。裸 statement `match` 缺少 `(`、`)`、`{`、pattern、箭头、分支体或 `}` 时只在 `MatchStatement` 内使用议题 03 的 `MissingToken`、缺失 body 节点和 `ErrorNode`；expression context 中的对应错误同样只在 `MatchExpression` 内恢复。随后出现的 arm 标点、外层分号或 postfix 都不能触发节点回退。

arm 结束恢复按已经解析出的 body 状态确定：

- `MatchStatement` 的完整结构化 statement 已由自身的 `}` 等结构结束符闭合，在下一 arm 前不插入分号。尚未以分号结束的简单 statement 若在当前层遇到下一完整 arm pattern 与 `=>`、真实 `,` 或当前 `match` 的 `}`，先插入 `MissingToken(';')`；如果 `=>` 后没有任何 body，则建立缺失 body 节点。真实 `,` 随后始终保存在该 arm 的 `ErrorNode` 中，并可给出“若要丢弃值形式，请把整个 match 括起来”的定向说明。
- `MatchExpression` arm 若在当前层的下一完整 arm pattern 与 `=>` 或当前 `match` 的 `}` 前仍没有真实 `,`，插入 `MissingToken(',')`；完全缺少 body 时先建立缺失 body 节点。真实 `;` 保存在 `ErrorNode` 中：其后若有真实 `,`，仍正常消费该逗号；否则在下一 arm 或 `}` 前继续插入缺失逗号。语句式分号不得触发改判为 `MatchStatement`。

arm body Parser 还必须接收只在当前 `match` 花括号深度生效的零宽同步谓词：在把 `.` 消费为成员后缀前，先无诊断试认完整 `variant_pattern` 后是否紧跟合法 `=>`；对准确拼写的 `_` 做同样检查。如果成立，当前 body 在该 Token 前停止并把 pattern 留给下一 arm，再由上一段规则依据 body 种类和完成状态决定是否插入分号、逗号或缺失 body。这样 `.next =>` 不会被前一个残缺表达式误吞为成员访问；普通 `object.field` 因后面没有 `=>` 而不触发同步。嵌套括号、方括号和花括号内的相同 Token 序列不触发外层谓词。

直接 `var`、`const` 声明错误由 recovery-only declaration Parser 保留形状但不建立局部绑定。真实的顶层 `;` 由该恢复 Parser 消费为错误声明的终止符；当前层真实 `,`、下一完整 arm pattern 与 `=>`、当前 `match` 的 `}` 构成不消费的 StopSet。声明形状在这些 StopSet 前缺少 `;` 时可在错误子树中插入 `MissingToken(';')`，随后把同步 Token 留给 outer arm Parser；其中真实逗号仍按上一段保存到 arm 的 `ErrorNode`。所有恢复都不能删除真实 Token。

## 12. 确认结论

Ink 的 `match_statement` 和 `match_expression` 都把被匹配表达式放在固定括号内。statement entry 的裸 `match` 与 `comptime match` 在解析 arm 前不可回滚地提交到结构控制；初始化器、操作数、实参等明确 expression context 中的 `match` 建立 `MatchExpression`，完整值形式作为表达式语句时必须先整体加括号。这一分派只需要调用者上下文和至多两个显著 Token 的前瞻，不查询符号表、名称或类型，arm 的逗号与分号只验证已选节点而不参与选型。语句形式使用一条 `statement` 作为每个分支体，不使用分支逗号，也不在整个结构后写分号；已经提交到语句形式后，分支起点的 `var` 或 `const` 必须定向诊断为需要外包语句块，不能回退成赋值或表达式。表达式形式的正常分支产生表达式值，每个分支都必须以逗号结束，不正常完成的分支可以使用同样以逗号结束的 `statement_block`。两种结构至少包含一个分支，并共享议题 23 的模式、一次求值、分支局部借用和穷尽检查规则。
