# 语义分析接口

入口为 [`Analyzer::analyze`](../src/include/ink/semantic/analyzer/analyzer.h)，头文件和实现分别位于 `src/include/ink/semantic/analyzer` 和 `src/lib/semantic/analyzer`：

```cpp
class Analyzer
{
  public:
    ir::Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
};
```

通过 `Analyzer` 实例调用。模型和构建器统一位于 `ink::ir`；`SemanticContext` 组合独立的 `IRContext` 与 `ScopeStore`，分析器使用 `IRBuilder(Context.irContext())`。当前已实现可编译的流程骨架：检查解析结果和源码归属，通过 `IRBuilder` 创建 Context 拥有的模块、模块独立拥有的声明根及入口块。每次调用的 `AnalysisState` 直接拥有独立 `NameResolver`，从 Context 的共享根作用域开始，再进入与本次模块关联的成员作用域，逐条分析顶层语句。成功返回上下文拥有的 `Module *`，失败返回 `nullptr`；调用结束时销毁 resolver，作用域及绑定仍保存在 Context 中。

`analyzeStmt()` 与 `analyzeDecl()` 从 Parser 的 `ASTNodes.def` 按 `Category` 生成严格分派。每种语句和声明均有手工声明、定义的 `analyzeXXX()`，不存在自动生成的空处理函数或基类回退；新增 AST 种类后缺少处理函数会导致编译失败，只有声明没有定义则导致链接失败。`DeclStmt` 转发到声明分派，`BlockStmt` 使用 `BlockDepthGuard` 管理嵌套深度、`NameResolver::ScopeGuard` 管理子作用域，并递归处理子语句；正常结束或提前返回时均自动恢复进入前的深度和作用域。块嵌套上限由 `INK_SEMANTIC_BLOCK_DEPTH_LIMIT` 配置，默认 256，模块和函数的成员作用域也使用 `ScopeGuard`。

当前支持空模块、块作用域，以及普通定参函数的声明和定义。函数签名通过检查后创建 `Function`，保存形参名称、类型、C 调用约定及 Ink/C 语言链接，并登记到当前词法作用域；成功的函数由当前 IR 块拥有。函数成员作用域绑定形参，函数体另建词法块作用域并逐条分派语句，嵌套函数继承外层查找环境，但不支持捕获外层形参。当前直线函数体支持显式 return、返回值类型检查、嵌套块的返回传播及 return 后不可达语句诊断；非 void 函数走到结尾报错，void 函数走到结尾补充无值返回。分支与循环的完整返回路径分析仍待实现。函数在分析主体前临时绑定，失败时通过 RAII 销毁函数及子节点并撤销相关绑定，不影响后续同级声明的作用域或插入点。

`analyzeType()` 支持 `void`、`bool`、`i8/i16/i32/i64/i128`、对应的无符号整数、`f16/f32/f64`、括号类型及 `*T`/`&T`；未限定指针和引用暂按 `ReadWrite` 构造，允许 `*void`，拒绝 `&void` 和 `*type`。类型名字先查词法绑定，再回退到内建类型；值不能用于类型位置。形参不能是 void 或元类型；未知类型、重名形参和冲突函数报告源码诊断。普通 Ink 函数可按不同参数类型列表形成重载集，不能仅按返回类型重载；同一作用域的 C 链接函数不能形成重载。类型表达式递归上限由 `INK_SEMANTIC_TYPE_DEPTH_LIMIT` 配置，默认 256，每次分析开始时读取一次；深度从 0 计数，达到上限即报告 ICE 并 panic，0 会拒绝任何类型分析。

同一函数的形参名必须唯一；每个重复出现的名字使用专用诊断 `SemanticDuplicateParameterName`（`INK-S0015`），定位到该次形参名的 token，消息明确说明函数内形参名不得重复。普通符号冲突仍使用 `SemanticDuplicateName`。

`analyzeFunctionLinkage()` 返回 `std::optional<LanguageLinkage>`：没有 extern 时返回 Ink，完整解码字符串精确为 `"C"` 时返回 C，其他情况报告诊断并返回 `std::nullopt`；不截断内嵌 NUL。`checkFunctionConflicts()` 集中检查当前作用域的已有函数。参数列表、返回类型和语言链接全部一致，且至少一份声明没有函数体时，才属于将来可合并的兼容重复声明；目前仍报告未实现。linkage 是独立于 `FunctionType` 的函数元数据，关系到语言链接及符号命名约定，因此不能仅比较函数类型就忽略 Ink/C 的差异。不同 linkage 不用于区分合法重载，而按当前规则报告名称冲突。

`analyzeExpr()` 支持名字、括号、整数和字符串字面量、整数常量的一元正负号、固定位置参数调用；未绑定的 true/false 名字作为 bool 常量。整数在参数或返回类型确定后按目标位宽解析，支持 2/8/10/16 进制和完整 128 位范围；无期望类型时默认为 i32。已具有类型的值只接受完全同型传递，不隐式窄化、改变符号或把 bool 当整数。调用先分析实参，再选择重载，不在候选试探中生成调用；逐参数比较转换等级，整数常量优先 i32，其余能容纳该常量的整数类型同级，无法唯一选择时诊断歧义。未知名字、非可调用对象、实参数量或类型不符均报告用户错误。表达式深度由 `INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT` 控制，默认 256，达到上限报告 ICE。

字符串常量保持只读 u8 切片类型。在直接调用 C 链接函数时，字符串常量可传给可写 `*u8` 形参：`CStringInstruction` 表示每次执行都创建独立可写副本，包含完整 UTF-8 字节和额外终止 NUL，存活到调用者函数返回。不同调用的副本不共享可写存储；副本指针不能在该函数返回后继续使用。该转换仅用于 C 调用实参，不允许从函数返回字符串时隐式创建副本，也不适用于普通 Ink 函数、其他指针类型或一般切片。含内嵌 NUL 的字符串报告 `SemanticEmbeddedNull`，避免静默截断；宿主 `tryGetCString()` 只用于检查，不作为目标程序地址。调用者必须处于函数体内。

以下程序能完成语义分析，main 的基本块依次包含 `CStringInstruction`、`CallInstruction` 和返回 i32 零的 `ReturnInstruction`：

```ink
extern "C" func printf(msg: *u8): i32;
func main(): i32
{
  printf("hello, world");
  return 0;
}
```

这里按源码声明将 printf 视为定参外部函数；C 变参原型、变参实参提升、目标 ABI、字符串存储的后端 lowering、符号解析及链接仍未接入，不表示已经能够执行 libc 的 printf。泛型、属性、默认参数、变参、命名/展开实参、其他语言链接、声明合并、数组/切片/函数类型语法以及其他表达式和控制流仍报告 `SemanticUnsupported` ICE 并终止。

一处可恢复的用户错误不会阻止后续同级语句的诊断；ICE 会立即终止。恢复节点有显式处理函数，但公开入口拒绝带词法或语法错误、取消、超限或缺少根节点的输入。模块名无效时报告 `SemanticConstructionFailed`。分析失败时模块中已成功分析的同级函数仍由 Context 拥有，但不会返回成功模块。

内建名称的统一登记、声明预登记、完整类型/表达式分析、泛型实例化、编译期执行及完整结果验证尚未实现。当前按源码顺序处理声明；辅助类的泛型绑定能力可以独立使用，不表示 Analyzer 已支持泛型源码。

诊断通过 Core 的 `DiagnosticEngine::report<Kind>(SourceId, SourceRange, Arguments...)` 直接构造并报告，保留参数数量和类型的编译期检查。`AnalysisState::report<Kind>(SourceRange, Arguments...)` 自动使用本次分析的 Context 和 Source；在分析状态创建前，入口直接调用 Engine 的重载。诊断报告与失败返回分别处理。

实现按职责分组在 `src/lib/semantic/analyzer` 的 `expr`（表达式与转换）、`stmt`（语句与控制流）、`decl`（声明）和 `type`（类型）子目录中；入口和共享头文件保留在父目录，所有处理方法仍属于同一个 `Analyzer` 类：

| 文件 | 职责 |
| --- | --- |
| `analyzer.cpp` | 模块分析入口、未支持诊断，以及 `MissingStmt`、`ErrorStmt`、`MissingDecl`、`ErrorDecl` 的未支持处理 |
| `analyzer_internal.h` | 各实现文件共享的私有 `AnalysisState`、`ExpressionResult` 定义与辅助函数声明 |
| `expr/analyzer_expr.cpp` | 表达式分派与嵌套深度检查 |
| `expr/analyzer_call.cpp` | 重载选择、实参分析与调用 |
| `expr/analyzer_conversion.cpp` | 整数常量解析、表达式转换与类型诊断描述 |
| `expr/analyzer_literal.cpp` | 整数与字符串字面量 |
| `expr/analyzer_name.cpp` | 表达式名字解析与捕获检查 |
| `expr/analyzer_paren.cpp` | 括号表达式 |
| `expr/analyzer_unary.cpp` | 一元表达式 |
| `stmt/analyzer_stmt.cpp` | 语句分派、简单语句、块作用域及声明语句转发 |
| `stmt/analyzer_if.cpp`、`stmt/analyzer_switch.cpp` | 条件与 switch 语句 |
| `stmt/analyzer_while.cpp`、`stmt/analyzer_classic_for.cpp`、`stmt/analyzer_for_in.cpp` | 循环语句 |
| `stmt/analyzer_return.cpp` | 返回语句与返回值类型检查 |
| `stmt/analyzer_break.cpp`、`stmt/analyzer_continue.cpp`、`stmt/analyzer_yield.cpp`、`stmt/analyzer_defer.cpp` | break、continue、yield 与 defer 语句 |
| `stmt/analyzer_import.cpp` | 直接导入与 from 导入 |
| `stmt/analyzer_comptime.cpp` | comptime 语句 |
| `decl/analyzer_decl.cpp` | 声明分派、变量声明与字段声明 |
| `decl/analyzer_function.cpp` | 函数签名、链接方式、名称冲突、形参作用域及函数体分析 |
| `decl/analyzer_class.cpp` | 类声明 |
| `decl/analyzer_enum.cpp` | 枚举声明 |
| `decl/analyzer_interface.cpp` | 接口声明 |
| `type/analyzer_type.cpp` | 基础类型名称、括号类型、指针与引用类型解析 |

## 名字绑定与作用域

[`ScopeStore`](../src/include/ink/semantic/name_resolve/scope_store.h) 由 `SemanticContext` 独占，通过 `scopeStore()` 访问，拥有作用域树、名字绑定、实体成员作用域索引与泛型定义作用域索引。它通过 IR 的 `LifetimeObserver` 订阅值和声明的销毁通知，以清理借用绑定。SemanticContext 销毁时先释放 ScopeStore 并注销订阅，再销毁 IRContext 的模块、常量和类型；IR 不包含或依赖 semantic 头文件。

[`NameResolver`](../src/include/ink/semantic/name_resolve/name_resolver.h) 借用 `ScopeStore`，只保存本实例的当前作用域并提供绑定和查找操作。多个 resolver 共享持久数据，各自维护当前位置；创建或销毁 resolver 不会清空名字绑定，也不会改变其他 resolver 的当前位置。它不遍历 AST，也不执行类型检查、实例化或重载选择。

名字解析的头文件位于 `src/include/ink/semantic/name_resolve`，实现位于 `src/lib/semantic/name_resolve`。[`Binding<T>`](../src/include/ink/semantic/name_resolve/binding.h)、[`Scope`](../src/include/ink/semantic/name_resolve/scope.h)、`ScopeStore` 和 `NameResolver` 是独立类型。`Binding<T>` 仅接受 `Value *` 或 `Decl *` 两种模板参数，保存名称和该类别的候选指针。同一头文件中的 `BindingTable<T>` 是 `std::unordered_map<Name, Binding<T>>` 的受约束别名，供 `Scope` 保存 `BindingTable<Value *>` 和 `BindingTable<Decl *>`。两表共同组成一个词法名字空间，复用 `NameResolver` 的表内插入、去重及查找模板；resolver 将持久索引和目标绑定位置记录在同一份 `ScopeStore` 中。

- `NameResolver(SemanticContext &)` 从 Context 的共享根作用域开始；`NameResolver(Scope &InitialScope)` 从已有作用域开始，并使用该作用域所属的存储和 Context。`rootScope()` 始终返回该存储的根作用域，`currentScope()` 返回本实例的当前位置。不同模块应进入各自的成员作用域，同一 Context 的根绑定对其子作用域可见。
- `ScopeStore::memberScope(Owner)` 和 `definitionScope(Declaration)` 查询长期保存的作用域，未登记时返回空指针；非 const 存储返回 `Scope *`，可用于构造新的 resolver。const 存储返回 `const Scope *`。
- `enterScope()` 创建并进入当前作用域的新子作用域；`exitScope()` 回到父作用域并返回 `true`，在根作用域返回 `false`。退出不销毁作用域或绑定，再次进入会创建新作用域。
- `enterScope(Value &Owner)` 创建并进入实体成员作用域，词法父作用域为调用前的当前作用域。同一实体只允许创建一次，重复创建或传入外来上下文的实体返回空指针且不改变状态。该接口不自动登记实体名字；开放泛型定义的成员分析尚未实现。
- `NameResolver::ScopeGuard(Resolver)` 创建并进入词法子作用域；带 `Owner` 的重载创建并进入成员作用域。`scope()` 返回创建的作用域，成员作用域创建失败时返回空指针。成功的 Guard 在析构时恢复原来的准确位置，失败的 Guard 不修改状态；退出不销毁作用域或绑定。Guard 禁止复制和移动，resolver 及 Context 必须比 Guard 活得更久。
- `bind(Name, Value &)` 登记值；`bind(Name, Decl &)` 登记本上下文的泛型 `FunctionDecl` 或 `ClassDecl`，拒绝 `ModuleDecl`。两者均允许别名。模块名称通过 `Module` 值绑定，泛型形参实例化后的类型或常量也属于值，不使用声明绑定。
- 普通 `Function` 和泛型 `FunctionDecl` 可以同名，分别保存在该作用域的两份有类型候选列表中；即使各自只有一个函数，也标记为重载集合。每份列表保持自身的登记顺序，不提供跨类别的总登记顺序。函数类型的普通表达式结果不作为函数重载实体。
- 泛型类和其他非函数值在同一作用域中占用独占名称，不能与另一类别同名。重绑定同一实体返回 `AlreadyBound`；非法同名返回 `Conflict`，原绑定不变，也不创建空的另一类别绑定。函数签名是否重复及重载选择仍由后续分析判断。
- 成功返回 `Inserted`；无效或越界名称返回 `InvalidName`；外来值、外来声明分别返回 `ForeignValue`、`ForeignDecl`；本上下文内不允许绑定的声明返回 `InvalidDecl`。通过 `Declaration.module().context()` 检查声明归属，Context 不保存声明容器或归属索引；模块摘除或嵌套后仍保留原上下文归属。绑定接口不自行报告诊断。
- `lookup(Name)` 等价于 `lookup<Value *>(Name)`；`lookup<Decl *>(Name)` 查询泛型定义。先从当前作用域向外找到包含任一类别同名绑定的最近作用域，再只返回该层请求类别的绑定。若这一层仅有另一类别，返回空指针，不能继续向外搜索。因此内层名称遮蔽外层两类候选，也不会合并不同层的函数重载。
- `lookupLocal<T>(Name)` 只查当前作用域，模板参数同样默认为 `Value *`。未命中或无效名称返回空指针，查找不驻留名称。
- `lookupMember<T>(Value &Owner, Name)` 只查实体关联的成员作用域，不沿词法父作用域回退、不查子作用域，也不改变当前作用域。模板参数默认为 `Value *`。实体未关联成员作用域或名称未命中时返回空指针；别名指向同一实体时共享成员作用域。
- `definitionScope(const Decl &)` 返回泛型定义在同一 `ScopeStore` 中首次成功绑定的作用域，未成功绑定时返回空指针。调用方应先在定义处登记，再建立别名；其他 resolver 或其他层中的后续别名不会覆盖定义环境。退出作用域或销毁 resolver 后，作用域指针仍有效；这是作用域关联，不是完整的可序列化定义环境或可见性快照。
- 名称何时可见由调用方选择登记时机。模块导入、继承查找、访问控制、条件声明激活及完整泛型定义环境仍需后续实现。

示例：

```cpp
Resolver.bind(FunctionName, FunctionValue);
Resolver.bind(FunctionName, GenericDefinition);
const Binding<Value *> *Functions = Resolver.lookup(FunctionName);
const Binding<Decl *> *GenericFunctions = Resolver.lookup<Decl *>(FunctionName);
```

作用域地址在 `SemanticContext` 存活期间保持稳定；绑定在仍有目标时保持地址稳定。`targets()` 返回 `std::span<const T>`：分别为 `std::span<Value *const>` 和 `std::span<Decl *const>`，只读的是指针列表；添加候选或销毁目标后须重新获取视图。`ir::Value`、`ir::Decl` 析构时通知 ScopeStore，由 ScopeStore 通过反向绑定位置索引移除自身在所有作用域中的候选和别名，并清理成员作用域或定义作用域索引；空绑定被删除，此时指向该绑定的旧指针失效，其他重载候选继续保留。模块摘除或转移不改变绑定，实际销毁时才清理；作用域对象本身继续由 Context 保存。

绑定、成员及定义作用域索引不拥有目标，清理也不负责修复 IR 操作数或 Builder 插入点。Context 必须比 resolver 和所有借用它的对象活得更久，借用的 AST 单元必须比 Module 活得更久。共享存储不提供并发写入同步。`Name` 必须来自同一上下文的名称池，紧凑名称索引不能识别另一池中数值相同的名称。

当前对象模型能力、缺口和后续架构见 [Semantic 模块设计](Ink-Semantic-Design.md)。
