# 议题 61：`comptime` 泛型、部分求值与编译期执行

> 状态：已确认，议题 62—72 补充泛型、声明区域控制、重载、元组元值与统一函数阶段；Parser 议题 31、32 确认泛型函数声明语法与统一区域控制；议题 67 明确 v0 不提供动态声明构造器
> 确认日期：2026-08-02

## 1. 统一模型

Ink 不建立独立于普通语言的模板元编程系统。普通 Ink 代码原则上都可以由编译器解释执行；`comptime` 直接修饰表达式、代码块或结构化控制流，要求相应部分在编译期求值或选择。

泛型是接收编译期参数的普通声明：

```ink
func duplicate<T: type>(value: const T&) -> (T, T) {
    return (value, value);
}

class Array<
    T: type,
    N: ptrsize
> {
    var values: T[N];
}
```

泛型尖括号天然形成编译期参数域。这里的 `T` 是值为某个具体类型的编译期参数，`N` 是值为某个 `ptrsize` 常量的编译期参数；形参类型前不再重复书写 `comptime`。闭合声明由编译器在实例化位置进行部分求值，最终产生不含开放参数的运行时 InkIR。

## 2. `comptime` 不构成新的类型

`comptime` 是表达式与控制结构的求值阶段要求，不是类型构造器、参数限定符，也不参与类型身份或重载身份：

```text
type_of(comptime i32_expression) == i32
type_of(comptime type_expression) == type
```

因此不能靠是否在调用处使用 `comptime` 建立另一套重载。两个泛型形参和普通函数签名完全相同的声明属于重定义：

```ink
func encode<T: type>(value: const T&) {
    comptime if (is_integer(T)) {
        // 整数实现
    }
}

func encode<T: type>(value: const T&) { // 编译错误：重定义
    comptime if (is_record(T)) {
        // 记录实现
    }
}
```

需要按编译期条件选择实现时，使用一个泛型声明：

```ink
func encode<T: type>(value: const T&) {
    comptime if (is_integer(T)) {
        // 整数实现
    } else if (is_record(T)) {
        // 记录实现
    } else {
        compile_error("unsupported encode type");
    }
}
```

普通重载仍要求签名按照普通重载规则彼此可区分。Ink v0 不采用函数体中的编译期条件建立 SFINAE 式候选，也不把替换失败解释成静默移除同签名声明。

议题 70 进一步规定：显式编译期实参先闭合候选声明头，再执行普通重载解析；声明头或已选中函数体的实例化失败都是硬错误，不能触发候选回退。

## 3. 不要求 `where` 约束

Ink 泛型不要求 `where T: Printable` 一类声明式约束。泛型体可以直接使用依赖于编译期参数的操作，并在具体实例化时完成检查：

```ink
func print<T: type>(value: const T&) {
    value.print();
}
```

如果 `T` 没有兼容的 `print()`，对应实例产生编译错误。需要定制诊断时，可以使用完整编译期反射和 `compile_error`：

```ink
func print<T: type>(value: const T&) {
    comptime if (!reflect(T).has_method("print")) {
        compile_error("T must provide print()");
    }

    value.print();
}
```

这意味着开放泛型体不必在定义时脱离实例参数完成一次全量类型证明；其依赖操作保留在 Staged InkIR 中，并在实例化时解析和验证。普通、不依赖编译期参数的代码仍须在声明分析时立即通过类型检查。

## 4. 泛型实例化就是部分求值

泛型实例化不再是编译后段的独立模板展开或单态化阶段，而是 Staged InkIR 展开循环中的 Partial Evaluation：

```ink
func add_offset<N: i32>(value: i32) -> i32 {
    return value + N;
}
```

实例化：

```ink
add_offset<10>(runtime_value);
```

部分求值器看到：

```text
N     = Known(10)
value = Runtime(value)
```

并产生概念上的残留函数：

```ink
func add_offset$10(value: i32) -> i32 {
    return value + 10;
}
```

类型参数遵循同一模型。generic parameter list 中的 `T: type` 是已知的类型值；类型布局、成员解析、反射和 `comptime if` 可以依据它执行，尚未取得的普通函数参数和运行时状态继续残留为闭合 InkIR。

## 5. 完整求值与残留化

执行器至少区分两类值：

```text
Known(value)       编译期已知
Runtime(ir_value)  运行时才能取得
```

显式 `comptime expression` 要求表达式最终结果完全由 `Known` 值组成；`comptime { ... }` 要求整个块在编译期执行完成：

```ink
const value = comptime fibonacci(20);
```

如果它们的必经执行路径读取 `Runtime` 值、进入没有编译期实现的操作，或者不能完成求值，则编译失败。

结构化控制前缀采用较窄要求：`comptime if`、`comptime match` 和 `comptime for` 要求条件、被匹配值或迭代源在编译期已知，选中或展开的 body 仍可把依赖运行时输入的普通代码残留为 InkIR。`comptime while` 要求每轮条件都能在编译期决定，并受执行次数和生成代码量预算限制。

Parser 议题 32 把这种控制统一表示为接收类型化输出区域的 `ComptimeRegionControl`。同一个选择或展开算法可以把结果交给 `StatementSink`、`TopLevelDeclarationSink`、`ClassMemberSink`、`InterfaceMemberSink` 或 `EnumMemberSink`；区域只约束 body item 和输出种类，不改变上述阶段要求。类中不存在另一种专门生成字段的 `comptime`。

混合静态和动态输入的泛型实例进入残留化模式。已知的纯计算可以执行；依赖运行时值的运算生成新的 InkIR：

```text
Known(10) + Known(20)    → Known(30)
Runtime(x) + Known(10)   → residual `add x, 10`
```

议题 71 将同一规则逐元素应用到元组：只含普通可表示元素的已知元组可以整体残留；含 `type` 或声明句柄等编译期专用值的元组只能在编译期存在，但从中投影出的普通元素仍可单独残留。

普通优化器仍可在不改变可观察语义的前提下折叠常量，但只有源码要求的 `comptime` 求值构成语言保证。

## 6. 副作用保留所属阶段

部分求值不能仅因实参已知就把普通运行时副作用移动到编译期：

```ink
func calculate<N: i32>(value: i32) -> i32 {
    stdio.write("running calculate\n");
    return value + N;
}
```

`calculate<10>` 的残留代码仍在每次运行时调用时输出文本。只有显式放入 `comptime` 上下文的副作用才在编译期发生：

```ink
func calculate<N: i32>(value: i32) -> i32 {
    comptime stdio.write("generating calculate\n");
    stdio.write("running calculate\n");
    return value + N;
}
```

实例化执行第一项输出；残留函数保存第二项输出。未被选择的 `comptime if` 分支不执行其中的副作用，也不会仅因其中存在目标专用操作而使已选择实例失败。

## 7. 一个执行器、多个执行环境

编译期执行、部分求值和真正的 InkIR 解释执行共享同一个语言语义执行核心，包括：

- 指令语义和控制流；
- 调用栈与普通函数调用；
- 算术、对象和内存语义；
- 异常传播；
- 生命周期操作。

执行上下文提供不同环境：

```text
ComptimeWorld     完整编译期求值和编译器元值
ResidualizeWorld  部分求值并生成残留 InkIR
RuntimeWorld      解释运行时程序
```

`ComptimeWorld` 和 `ResidualizeWorld` 可以操作 `type`、声明、反射结果和受控 IR 构造器；`RuntimeWorld` 不允许创建编译器类型或修改待编译模块。三种环境可以为同一个有环境效果的 InkIR 操作安装不同处理器。

## 8. 固定点展开流程

Ink 编译流程采用以下结构：

```text
source
→ lexer / parser
→ AST
→ declaration collection, name binding, initial type checking
→ Staged InkIR

┌─ elaboration + partial evaluation loop ─────────────┐
│ process requested generic instances                 │
│ substitute comptime arguments                       │
│ execute required comptime code                      │
│ generate values, types, declarations and InkIR      │
│ bind names, type-check and verify generated results │
│ enqueue newly requested instances                   │
│ repeat until no new declarations or instances       │
└──────────────────────────────────────────────────────┘

→ Closed InkIR[target]
├─ InkIR interpreter(TargetWorld)
└─ LLVM lowering(TargetABI)
   → LLVM IR
   → AOT
   → Obj
```

泛型实例可以请求其他实例；新请求继续进入同一队列。编译器以“泛型声明身份 + 规范化编译期实参 + 目标配置”为基础缓存进行中的实例，并检测无法收敛的递归实例化和无限声明展开。

固定点结束后，进入运行时代码生成的 InkIR 不再含有开放泛型参数。没有第二个泛型实例化或单态化阶段。

运行时动态反射只能观察和调用编译期间已经生成并选择登记的闭合泛型实例，不能在运行时提交新的类型或值实参并触发编译器实例化。Ink v0 不要求运行时 JIT。

## 9. 完整反射与静态声明展开

为了让普通编译期代码覆盖传统模板元编程用途，Ink 支持：

- `type` 作为一等编译期值；
- 编译期函数返回 `type`；
- 完整声明结构反射；
- 反射结果用于编译期条件、类型选择和普通表达式；
- 类型和模块声明区中的 `comptime if`；
- 通过 `comptime for` 重复源码中静态写出的普通声明；
- class 类型表达式和静态声明展开产生经过验证的新 InkIR。

选择或展开的声明必须重新经过名称绑定、访问检查、类型检查、布局依赖检查和 IR verifier。声明名称必须来自源码中的真实 Identifier Token；编译期代码不能用反射值或字符串替换名称，也不能取得可任意破坏编译器内部不变量的可写裸 IR 指针。

同一轮执行看到确定的声明快照；本轮选择或重复展开的普通声明在验证和提交后进入后续展开轮次。反射当前未完成类型、产生重复成员、形成无限大小布局或建立布局查询环均为编译错误。

本议题不改变议题 19 已确认的边界：结构反射不会自动暴露函数体 AST、局部变量、注释或任意 token stream。

议题 63 进一步禁止把编译期字符串重新解析为源码、Token、Identifier splice 或当前局部代码。议题 67 明确 v0 不提供 `field(...)`、`function(...)` 或其他声明构造器；`comptime` 只能选择或重复静态写出的普通声明。

## 10. 目标上下文与 PDB

编译期执行从编译开始就接收明确的 `TargetContext`。`target.arch`、`ptrsize`、字节序、完整类型布局和议题 08 的平台相关行为按照编译目标解释，不能使用编译器宿主平台代替。

因此展开结果记为：

```text
Closed InkIR[target]
```

它已经可能包含目标条件分支和目标布局查询的结果。Ink 不再规定独立的 `TargetDependentInkIR` 层；前端的目标布局服务按需确定完整类型的大小、对齐和字段偏移，解释器直接以 `TargetWorld` 执行闭合 InkIR，LLVM 后端把已经确定的布局落实到 LLVM IR，并负责 C ABI、调用约定和目标合法化。

如果未来出现多个低级代码生成后端、ABI 级优化或低级 IR 解释需求，可以按实现需要增加 `CodegenInkIR[target]`，但它不是当前语言编译模型的必需层。

## 11. 外部操作与编译期处理器

Ink 不把普通函数永久分类为“编译期函数”或“运行时函数”。编译期解释器正常进入普通调用链，直到遇到具有环境效果的底层 InkIR 操作：

议题 72 据此明确取消 `comptime func` 声明类别，也取消 `Name: comptime Type` 参数限定形式；`comptime` 只直接修饰表达式、代码块或结构化控制流。泛型形参、attribute/decorator 实参等位置是否天然要求编译期值，由各自上下文决定。

```text
fs.read
fs.write
network.request
environment.read
clock.now
random.read
process.spawn
target.extern_call
inline_assembly
```

执行上下文为这些操作提供能力表和处理器。以文件读取为例：

```ink
const schema = comptime stdio.read_file("schema.json");
```

标准库最终进入 `fs.read` 内建操作；`ComptimeWorld` 的处理器通过编译器文件系统读取内容并记录路径和内容依赖，`RuntimeWorld` 使用运行环境适配器，LLVM lowering 则生成目标平台调用。

如果当前上下文没有对应能力、处理器或已知实参，必须在实际执行到该操作时报告编译期错误，并给出完整编译期调用栈。不能根据函数名称猜测，也不能仅因未选择分支中出现目标系统调用就拒绝整个函数。

目标裸 FFI、目标系统调用和内联汇编默认没有宿主编译期实现；除非编译器显式提供语义等价的处理器，否则不能在 `comptime` 中执行。编译期临时内存的裸地址、宿主句柄和其他只在编译器进程中有意义的值不能逃逸到残留程序。

文件、环境、网络和其他外部输入必须进入依赖或不可复现性记录。缓存键至少包含执行代码、规范化实参、目标配置、编译器版本和已经声明的外部依赖；能力策略可以拒绝网络、进程等操作。

## 12. 源码分发与 C ABI 二进制边界

Ink 不定义稳定的 Ink-to-Ink 二进制模块 ABI。可复用 Ink 模块和开放泛型只能以源码形式分发，由使用方针对自己的编译器版本、目标配置和编译期输入执行展开。

编译器可以保存 AST、Staged InkIR、闭合 InkIR、目标文件等内部缓存，但这些缓存是可失效的实现细节，不是稳定分发格式。

需要把可复用库作为二进制分发时，公开边界必须退回 C ABI：

```ink
extern "C" func image_create(
    width: u32,
    height: u32
) -> ImageHandle;
```

该边界只能直接使用 C ABI 可表示的类型和约定。Ink 类、接口、异常、开放泛型、`Task<T>` 和其他 Ink 私有 ABI 必须通过不透明句柄、错误码、回调和具体闭合包装显式适配，不能作为稳定二进制接口直接导出。

最终可执行文件、普通目标文件和由同一次兼容构建协调的内部模块镜像仍然可以是二进制；本规则禁止的是把私有 Ink ABI 当作跨编译器、跨构建的稳定库分发协议。

## 13. 诊断与终止

编译器必须诊断：

- 强制 `comptime` 求值依赖运行时未知值；
- 执行到没有编译期处理器的环境操作；
- 同签名泛型重定义；
- 具体实例中的成员、调用或复制等依赖操作无效；
- 无法收敛的实例化或声明展开；
- 编译期值包含不能残留的宿主地址或句柄；
- 类型布局和反射生成形成阶段循环。

编译期执行必须支持用户取消和实现定义的资源预算。超过预算不是运行时 UB；编译器应报告当前编译期调用栈、实例化栈和已经消耗的主要资源，并允许构建配置提高合理上限。

## 14. LLVM 与实现边界

LLVM 不需要理解 Ink 泛型、`comptime`、反射或 Partial Evaluation。前端固定点展开产生 `Closed InkIR[target]`，LLVM 后端只接收已经闭合的运行时程序，把前端确定的目标布局落实为 LLVM 类型，并完成 ABI lowering 和合法化。

InkIR 解释器、编译期处理器、声明区域展开器和固定点调度属于 Ink 编译器。未来即使替换 LLVM 后端，也不改变本议题的语言语义。

## 15. 后续问题

以下内容留给独立议题：

- 外部效果能力清单、构建权限和依赖清单格式；
- Partial Evaluation 的代码膨胀预算与实例合并优化；
- 编译期并发、调度和确定性规则；
- C ABI 安全类型及 `repr(C)` 的完整规则。
