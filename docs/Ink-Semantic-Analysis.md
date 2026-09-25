# 语义分析接口

入口为 [`Analyzer::analyze`](../src/include/ink/semantic/analyze/analyzer.h)，头文件和实现分别位于 `src/include/ink/semantic/analyze` 和 `src/lib/semantic/analyze`：

```cpp
class Analyzer
{
  public:
    Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
};
```

通过 `Analyzer` 实例调用。当前方法仍为空实现，始终返回 `nullptr`，不检查输入、不创建模块、不注册内置函数，也不报告诊断。

## 名字绑定与作用域

[`NameResolver`](../src/include/ink/semantic/name_resolve/name_resolver.h) 是独立的语义分析辅助类，借用 `SemanticContext`，拥有作用域树和名字绑定。它不遍历 AST，也不执行类型检查或重载选择。

名字解析的头文件位于 `src/include/ink/semantic/name_resolve`，实现位于 `src/lib/semantic/name_resolve`。[`Binding`](../src/include/ink/semantic/name_resolve/binding.h)、[`Scope`](../src/include/ink/semantic/name_resolve/scope.h) 和 `NameResolver` 是 `ink::semantic` 命名空间下的三个独立类型：`Binding` 保存名字及候选值，`Scope` 保存词法父作用域及绑定表，`NameResolver` 管理作用域生命周期、当前作用域、实体成员作用域关联和查找。

- 构造后当前作用域为根作用域；`rootScope()` 返回根作用域，`currentScope()` 返回当前作用域。
- `enterScope()` 创建并进入当前作用域的一个新子作用域，返回该作用域的引用；`exitScope()` 回到父作用域并返回 `true`。在根作用域调用 `exitScope()` 返回 `false`，当前作用域保持不变。退出不会销毁作用域或已有绑定，再次进入会创建新的子作用域。
- `enterScope(Value &Owner)` 创建并进入实体的成员作用域，返回作用域指针；词法父作用域为调用前的当前作用域。同一实体只允许创建一次，重复创建或传入外来上下文的实体返回空指针，当前作用域和已有绑定保持不变。该接口不自动登记实体的名字，调用方通过 `bind()` 单独登记；无参数的 `enterScope()` 用于创建不关联实体的词法作用域。
- `bind(Name, Value &)` 在当前作用域登记值，可使用别名；绑定内部直接保存 `Value *`，不接受 `Decl`。非 `Function` 的值在同一作用域中只能绑定一个实体；函数类型的表达式结果也是普通值绑定。
- 多个 `Function` 可以同名，按登记顺序组成候选集合。单个函数同样表示重载集合。不同函数实体即使签名相同也保留，由后续声明检查判断重声明或非法重复，由调用分析选择重载。
- 重复登记同一实体返回 `AlreadyBound`，不重复添加；非函数同名冲突返回 `Conflict`，保留已有绑定。成功返回 `Inserted`；无效或越界名称、外来上下文的值分别返回 `InvalidName`、`ForeignValue`。这些接口不自行报告诊断。
- `lookup(Name)` 从当前作用域沿父作用域查找最近的绑定；内层同名绑定遮蔽整个外层集合，不合并外层重载。`lookupLocal(Name)` 只查当前作用域。兄弟和子作用域不可见，退出后恢复父作用域的可见绑定。未命中或无效名称返回空指针，查找不驻留新名称。
- `lookupMember(Value &Owner, Name)` 只查实体关联的成员作用域，不沿词法父作用域回退，也不查其子作用域，不切换当前作用域。实体未关联成员作用域、名称无效或没有对应成员时返回空指针。关联以 `Value *` 身份为键，实体的别名共享成员绑定；不同实体即使名称相同，成员作用域也各自独立。函数成员返回完整的重载候选集合。
- 限定名由调用方逐段解析：`A.B.C` 先用 `lookup(A)` 找到 A，再依次调用 `lookupMember(AValue, B)` 和 `lookupMember(BValue, C)`；在 A 内解析 `B.C` 时先用 `lookup(B)`。没有遮蔽且 B 指向同一实体时，两条路径取得同一个 C 绑定。接口不拆分名字字符串，AST 成员访问分析尚未接入。
- 名称在何时可见由调用方选择登记时机。模块导入、继承成员查找、访问控制和条件声明激活仍需后续实现。

作用域及 `Binding` 的地址在 `NameResolver` 存活期间保持稳定，退出后实体与成员作用域的关联仍然有效。`Binding::targets()` 返回 `std::span<Value *const>`，候选指针列表只读，目标值可修改；继续向同一绑定添加候选后须重新获取视图。绑定和成员作用域索引不拥有 `Value`；上下文、绑定目标和成员作用域所属实体必须比 NameResolver 活得更久。`Name` 必须使用该上下文的名称池，紧凑名称索引不能识别来自其他池但数值相同的名称。

`semantic/model` 的对象模型和工厂接口仍可独立使用。当前模型能力和后续缺口见 [可执行 IR 状态](Ink-Executable-IR-Status.md)，后续分析器架构见 [Semantic 模块设计](Ink-Semantic-Design.md)。
