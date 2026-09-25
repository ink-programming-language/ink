# 语义分析接口

入口为 [`Analyzer::analyze`](../src/include/ink/semantic/analyze/analyzer.h)，头文件和实现分别位于 `src/include/ink/semantic/analyze` 和 `src/lib/semantic/analyze`：

```cpp
class Analyzer
{
  public:
    Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
};
```

通过 `Analyzer` 实例调用。当前已实现可编译的流程骨架：检查解析结果和源码归属，通过 `SemanticContext` 创建模块及入口块，构造 `NameResolver` 的根作用域，再进入与模块关联的成员作用域，逐条分析顶层语句。分析状态属于本次调用，成功返回上下文拥有的 `Module *`，失败返回 `nullptr`。

`analyzeStmt()` 与 `analyzeDecl()` 从 Parser 的 `ASTNodes.def` 按 `Category` 生成严格分派。每种语句和声明均有手工声明、定义的 `analyzeXXX()`，不存在自动生成的空处理函数或基类回退；新增 AST 种类后缺少处理函数会导致编译失败，只有声明没有定义则导致链接失败。`DeclStmt` 转发到声明分派，`BlockStmt` 创建子作用域并递归处理子语句，退出后恢复原作用域。块嵌套上限为 256。

当前仅空模块和只包含空块的模块可成功分析。其余具体语句、声明（包括泛型定义）报告现有的 `SemanticUnsupported` 诊断，包含节点种类、源码身份及范围；一处失败不会阻止后续同级语句的诊断。恢复节点也有显式处理函数，但公开入口拒绝带词法或语法错误、取消、超限或缺少根节点的输入。模块名无效时报告 `SemanticConstructionFailed`。失败时已分配的模型对象仍由上下文回收，不对调用者发布成功模块。

内建名称登记、声明预登记、类型/表达式分析、泛型实例化、编译期执行及完整结果验证尚未实现。辅助类的泛型绑定能力可以独立使用，不表示 Analyzer 已支持泛型源码。

## 名字绑定与作用域

[`NameResolver`](../src/include/ink/semantic/name_resolve/name_resolver.h) 是独立的语义分析辅助类，借用 `SemanticContext`，拥有作用域树和名字绑定。它不遍历 AST，也不执行类型检查、实例化或重载选择。

名字解析的头文件位于 `src/include/ink/semantic/name_resolve`，实现位于 `src/lib/semantic/name_resolve`。[`Binding<T>`](../src/include/ink/semantic/name_resolve/binding.h)、[`Scope`](../src/include/ink/semantic/name_resolve/scope.h) 和 `NameResolver` 是独立类型。`Binding<T>` 仅接受 `Value *` 或 `Decl *` 两种模板参数，保存名称和该类别的候选指针。同一头文件中的 `BindingTable<T>` 是 `std::unordered_map<Name, Binding<T>>` 的受约束别名，供 `Scope` 保存 `BindingTable<Value *>` 和 `BindingTable<Decl *>`。两表共同组成一个词法名字空间，复用 `NameResolver` 的表内插入、去重及查找模板；跨类别冲突、遮蔽和定义作用域记录仍由同一个 `NameResolver` 处理。

- 构造后当前作用域为根作用域；`rootScope()` 返回根作用域，`currentScope()` 返回当前作用域。
- `enterScope()` 创建并进入当前作用域的新子作用域；`exitScope()` 回到父作用域并返回 `true`，在根作用域返回 `false`。退出不销毁作用域或绑定，再次进入会创建新作用域。
- `enterScope(Value &Owner)` 创建并进入实体成员作用域，词法父作用域为调用前的当前作用域。同一实体只允许创建一次，重复创建或传入外来上下文的实体返回空指针且不改变状态。该接口不自动登记实体名字；开放泛型定义的成员分析尚未实现。
- `bind(Name, Value &)` 登记值；`bind(Name, Decl &)` 登记本上下文的泛型 `FunctionDecl` 或 `ClassDecl`，拒绝 `ModuleDecl`。两者均允许别名。模块名称通过 `Module` 值绑定，泛型形参实例化后的类型或常量也属于值，不使用声明绑定。
- 普通 `Function` 和泛型 `FunctionDecl` 可以同名，分别保存在该作用域的两份有类型候选列表中；即使各自只有一个函数，也标记为重载集合。每份列表保持自身的登记顺序，不提供跨类别的总登记顺序。函数类型的普通表达式结果不作为函数重载实体。
- 泛型类和其他非函数值在同一作用域中占用独占名称，不能与另一类别同名。重绑定同一实体返回 `AlreadyBound`；非法同名返回 `Conflict`，原绑定不变，也不创建空的另一类别绑定。函数签名是否重复及重载选择仍由后续分析判断。
- 成功返回 `Inserted`；无效或越界名称返回 `InvalidName`；外来值、外来声明分别返回 `ForeignValue`、`ForeignDecl`；本上下文内不允许绑定的声明返回 `InvalidDecl`。`SemanticContext::owns(const Decl &)` 使用独立归属索引，`Decl` 本身不增加上下文字段。绑定接口不自行报告诊断。
- `lookup(Name)` 等价于 `lookup<Value *>(Name)`；`lookup<Decl *>(Name)` 查询泛型定义。先从当前作用域向外找到包含任一类别同名绑定的最近作用域，再只返回该层请求类别的绑定。若这一层仅有另一类别，返回空指针，不能继续向外搜索。因此内层名称遮蔽外层两类候选，也不会合并不同层的函数重载。
- `lookupLocal<T>(Name)` 只查当前作用域，模板参数同样默认为 `Value *`。未命中或无效名称返回空指针，查找不驻留名称。
- `lookupMember<T>(Value &Owner, Name)` 只查实体关联的成员作用域，不沿词法父作用域回退、不查子作用域，也不改变当前作用域。模板参数默认为 `Value *`。实体未关联成员作用域或名称未命中时返回空指针；别名指向同一实体时共享成员作用域。
- `definitionScope(const Decl &)` 返回泛型定义在此 resolver 中首次成功绑定的作用域，未成功绑定时返回空指针。调用方应先在定义处登记，再建立别名；后续在其他层绑定别名不会覆盖定义环境。退出作用域后指针仍有效，但只在 resolver 的生命周期内有效；这是作用域关联，不是完整的可序列化定义环境或可见性快照。
- 名称何时可见由调用方选择登记时机。模块导入、继承查找、访问控制、条件声明激活及完整泛型定义环境仍需后续实现。

示例：

```cpp
Resolver.bind(FunctionName, FunctionValue);
Resolver.bind(FunctionName, GenericDefinition);
const Binding<Value *> *Functions = Resolver.lookup(FunctionName);
const Binding<Decl *> *GenericFunctions = Resolver.lookup<Decl *>(FunctionName);
```

作用域及各 `Binding<T>` 的地址在 `NameResolver` 存活期间保持稳定。`targets()` 返回 `std::span<const T>`：分别为 `std::span<Value *const>` 和 `std::span<Decl *const>`，只读的是指针列表；继续添加候选后须重新获取视图。绑定、成员及定义作用域索引不拥有目标；上下文、目标实体及泛型定义借用的 AST 必须覆盖相应使用期。`Name` 必须来自同一上下文的名称池，紧凑名称索引不能识别另一池中数值相同的名称。

当前对象模型能力和缺口见 [可执行 IR 状态](Ink-Executable-IR-Status.md)，后续架构见 [Semantic 模块设计](Ink-Semantic-Design.md)。
