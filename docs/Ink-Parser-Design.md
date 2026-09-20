# Ink Parser 设计文档

采用 ASTKind 继承与 Visitor 的可恢复语法分析器

版本 1.0　日期 2026 年 9 月 20 日

本文规定 Ink Parser 的类结构、节点数据模型、解析算法、诊断与错误恢复契约，以及分阶段实现和验收方式。设计基于 Ink-grammar-Rules(3).bnf，适用于以 C++ 实现的编译器前端。主要决策是采用 ASTKind 标识节点种类，通过单继承组织节点，通过 Visitor 访问节点，并由 ASTContext 统一管理节点生命周期。

Parser 直接生成 AST。普通语法错误产生诊断和可恢复节点，后续独立声明仍继续解析。项目已有的 DiagnosticEngine 和 SourceLocation 作为集成边界复用；本文不重新定义这两个类型。示例代码表达设计契约，辅助名称按项目现有工具类适配。

## 1 设计决策与边界

| 项目 | 决策 |
| --- | --- |
| 语句与声明 | 手写递归下降，一个 Parser 实例共享游标和恢复上下文 |
| 表达式 | Pratt 解析中缀优先级，独立处理 unary、postfix 和条件表达式 |
| AST 表示 | ASTKind 加单继承，具体节点使用明确的字段 |
| 节点访问 | ASTKind 分派加 CRTP Visitor，默认不使用虚函数和 dynamic_cast |
| 节点生命周期 | ASTContext 中稳定地址的 Arena 分配，按需登记精确类型的析构回调 |
| 源码与 Token | TokenBuffer 持有不可变源码快照，AST 通过 SourceLocation 和 Token 引用定位 |
| 错误恢复 | 缺失节点、错误节点、局部插入与删除、按上下文同步 |
| 分支试探 | 只在少数公共前缀处使用事务，确认分支后恢复该分支 |
| 诊断 | 使用已有 DiagnosticEngine，试探诊断先缓冲再提交 |

第一版支持整文件解析，不依赖增量语法树、通用回溯框架或完整 CST。源码和 Token 序列保留，因此错误定位和源码片段显示仍然可用。AST 的 dump 是结构表示，不承担逐字还原或无损格式化的职责。

接受有错误的输入不等于允许生成代码。ParseStatus 与 DiagnosticEngine 的结果由编译驱动层汇总；存在阻止编译的错误时停止代码生成，编辑器分析仍可以使用保留下来的 AST。

## 2 主要类及其依赖

| 类 | 核心职责 | 依赖 |
| --- | --- | --- |
| TokenBuffer | 保存源码快照和不可变 Token 序列 | 项目源码与词法设施 |
| TokenCursor | 查看、消费 Token，记录和恢复位置 | TokenBuffer |
| ASTContext | 分配节点和数组，管理生命周期及分配检查点 | Arena |
| Parser | 选择语法分支并构造 AST | 游标、ASTContext、诊断适配与恢复上下文 |
| RecoveryContext | 保存结束符归属、恢复集合与嵌套状态 | TokenKind、SourceLocation |
| ParseTransaction | 提交或恢复局部试探状态 | Parser、ASTContext、诊断缓冲 |
| ParserDiagnosticEmitter | 将解析诊断送入缓冲或 DiagnosticEngine | 现有 DiagnosticEngine |
| ParsedUnit | 拥有一次解析的输入、ASTContext 和根节点 | TokenBuffer、ModuleAST |
| ASTVisitor | 根据 ASTKind 分派单个节点 | AST 节点声明 |
| ASTWalker | 按定义的子节点顺序遍历 | 子节点枚举接口、Visitor 或回调 |
| ASTVerifier | 检查节点种类、必需子节点和范围等不变量 | ASTWalker |

ParserDiagnosticEmitter 是轻量适配器，其作用是处理试探期间的缓冲；它不替代、重命名或复制项目的 DiagnosticEngine。若现有 Engine 已支持诊断事务，可以直接复用该能力。

同一 Parser 的函数分布在不同实现文件中，各文件共享同一份状态。声明、语句和表达式解析之间可以正常调用，无须建立各自独立的 TokenCursor。

## 3 源码与解析结果的生命周期

接口示例采用 C++20 的 std::span；项目已有 ArrayRef 等只读数组视图时可直接复用。

Token 使用项目已有的 SourceLocation 记录位置。需要获取拼写、构造节点覆盖范围或表示插入点时，调用项目的源码工具接口。以下 cover、insertionPoint、spelling 是适配操作的概念名，不要求 SourceLocation 具有指定成员或内存布局。

```cpp
struct Token {
    TokenKind kind;
    SourceLocation location;
};

class TokenBuffer {
    SourceSnapshotHandle Source;
    std::vector<Token> Tokens;
public:
    const Token& token(TokenId id) const;
    std::string_view spelling(TokenId id) const;
};
```

空白和注释可以保存在词法层的旁路信息中，TokenCursor 只遍历参与语法分析的 Token。EOF 是可观察的终止标记；越过末尾的 peek 始终返回 EOF，bump 在 EOF 处不推进。

```cpp
enum class ParseStatus {
    Completed,
    LimitExceeded,
    Cancelled
};

class ParsedUnit {
    std::shared_ptr<const TokenBuffer> Input;
    std::unique_ptr<ASTContext> Context;
    ModuleAST* Root;
    SyntaxRecoveryInfo RecoveryInfo;
};

struct ParseResult {
    std::unique_ptr<ParsedUnit> unit;
    ParseStatus status;
    bool hasSyntaxErrors;
};
```

Completed 表示本次语法扫描完成，并不表示源码没有错误。hasSyntaxErrors 来自当前解析调用的错误记录，不能通过检查共享 DiagnosticEngine 的全局历史错误数直接推断。词法错误由编译驱动层一并汇总。

ParsedUnit 先拥有输入，再拥有 ASTContext，销毁时先释放节点、最后释放输入。节点中引用源码的 string_view、TokenId 和 SourceLocation 因此在整个 ParsedUnit 生命周期内有效。取消或达到预算时可以返回部分树，但必须明确标记状态，并禁止把它当成完整输入继续代码生成。

## 4 ASTKind 与继承结构

ASTNodeBase 保存节点种类和源码位置。抽象分类基类只约束接口与种类范围，不创建实例；具体节点均为 final。分类关系如下。

| 分类基类 | 代表性具体节点 | 说明 |
| --- | --- | --- |
| Expr | NameExpr、BinaryExpr、CallExpr、MatchExpr | 表达式 |
| Stmt | BlockStmt、IfStmt、DeclStmt、SimpleStmt | 语句 |
| Decl | VarDecl、FieldDecl、FunctionDecl、ClassDecl | 声明 |
| SimpleItem | ExprItem、AssignmentItem | 简单语句与 for 步进中的条目 |
| BindingPattern | NameBindingPattern、TupleBindingPattern | 声明和 for 绑定 |
| MatchPattern | NameMatchPattern、OrMatchPattern | match 匹配模式 |
| ASTNodeBase 直接子类 | ModuleAST、TypeSyntax | 文件根与类型位置包装 |

BindingPattern 与 MatchPattern 可以再共同继承 Pattern。两者保持独立的语法入口和具体类型集合，避免把 match 的常量或 or-pattern 无意间允许到变量绑定位置。

```cpp
class ASTNodeBase {
    ASTKind Kind;
    SourceLocation Location;
protected:
    ASTNodeBase(ASTKind kind, SourceLocation location);
    ~ASTNodeBase() = default;
public:
    ASTKind getKind() const noexcept;
    const SourceLocation& getLocation() const noexcept;
    ASTNodeBase(const ASTNodeBase&) = delete;
    ASTNodeBase& operator=(const ASTNodeBase&) = delete;
};

class Expr : public ASTNodeBase {
protected:
    using ASTNodeBase::ASTNodeBase;
public:
    static bool classof(const ASTNodeBase* node);
};

class BinaryExpr final : public Expr {
    Expr* Left;
    TokenKind Op;
    SourceLocation OperatorLocation;
    Expr* Right;
public:
    static constexpr ASTKind Kind = ASTKind::BinaryExpr;
    BinaryExpr(SourceLocation whole, Expr* left,
               TokenKind op, SourceLocation opLoc, Expr* right);
    static bool classof(const ASTNodeBase* node);
    Expr* left() noexcept;
    const Expr* left() const noexcept;
    Expr* right() noexcept;
    const Expr* right() const noexcept;
};
```

BinaryExpr 的构造函数固定传入自身 Kind，不接受调用者任意指定种类。其左右操作数是必需子节点，必须指向 Expr；缺失时使用 MissingExpr。基类析构为 protected 且非虚，禁止通过 ASTNodeBase 指针 delete。节点生命周期只由 ASTContext 管理，析构时按分配的具体类型调用析构函数。

所有具体节点在 ASTNodes.def 中登记，至少记录节点名称、直接基类和分类。

```cpp
AST_NODE(NameExpr,       Expr, Expr)
AST_NODE(BinaryExpr,     Expr, Expr)
AST_NODE(MissingExpr,    Expr, Expr)
AST_NODE(BlockStmt,      Stmt, Stmt)
AST_NODE(AssignmentItem, SimpleItem, SimpleItem)
```

由该表生成 ASTKind、种类名称、分类映射、Visitor 分派和节点覆盖检查。ASTKind 是进程内实现细节，不直接作为持久化文件格式或 ABI。抽象分类不依赖枚举值连续，categoryOf 使用生成的映射判断。

叶节点 classof 判断精确 Kind，Expr::classof 判断分类。提供 isa<T>、dyn_cast<T> 和 cast<T>：isa 与 dyn_cast 对空指针返回 false 或空；cast 要求非空且种类匹配，违反时属于内部错误。const 重载保留 const，不允许转为可写指针。LLVM 的类型转换接口可作为接口风格参考，项目不必因此引入 LLVM 依赖。[1]

## 5 Visitor 与遍历协议

采用外部分派的 CRTP Visitor。节点不提供 virtual accept，访问器根据 ASTKind 进行一次 switch，再静态调用具体处理函数。Clang StmtVisitor 使用了按节点种类分派和基类回退的类似组织方式；Ink 的节点层次和处理规则以本文为准。[2]

以下展示可写访问器的核心形态。实际分派覆盖 ASTNodes.def 中的所有具体节点，switch 不使用吞掉新增节点的 default 分支。

```cpp
template<class Derived>
class ASTVisitor {
    Derived& impl() { return static_cast<Derived&>(*this); }
public:
    void visit(ASTNodeBase* node) {
        assert(node != nullptr);
        switch (node->getKind()) {
#define AST_NODE(Name, Base, Category) \
        case ASTKind::Name: \
            return impl().visit##Name(static_cast<Name*>(node));
#include "ASTNodes.def"
#undef AST_NODE
        }
        unreachableInternalError();
    }

#define AST_NODE(Name, Base, Category) \
    void visit##Name(Name* node) { impl().visit##Base(node); }
#include "ASTNodes.def"
#undef AST_NODE

    void visitExpr(Expr* n) { impl().visitASTNodeBase(n); }
    void visitStmt(Stmt* n) { impl().visitASTNodeBase(n); }
    void visitDecl(Decl* n) { impl().visitASTNodeBase(n); }
    void visitSimpleItem(SimpleItem* n) {
        impl().visitASTNodeBase(n);
    }
    // Pattern 等其余抽象分类提供同样的父类回退。
    void visitASTNodeBase(ASTNodeBase*) {}
};
```

同一份注册表生成 ConstASTVisitor，入口和具体处理函数均接收 const 指针。实现时可抽取 const 指针策略减少重复代码，但不能使用 const_cast 复用可写接口。

只读指针数组只保证数组元素不可替换，并不自动使所指节点只读。const 节点的数组访问与子节点枚举接口应输出 const 节点指针，可使用转换迭代视图；不要把 span 中的指针类型通过重解释强行转换。

visit 只访问一个节点，不自动访问其子节点。递归遍历由 ASTWalker 或具体分析过程控制。这个约定避免 Visitor 覆盖某个 visit 方法后意外漏遍历或重复遍历。

ASTWalker 默认采用显式工作栈，支持 enter、leave、SkipChildren 和 Stop。forEachChild 按源码顺序枚举结构性子节点，包括参数类型、默认值、属性实参、match guard 和 for 头部中的子表达式。

| 接口或访问器 | 责任 |
| --- | --- |
| ASTVisitor | 对当前节点做一次动态种类分派 |
| ConstASTVisitor | 对只读节点进行同样分派 |
| forEachChild | 枚举当前节点的直接结构性子节点 |
| ASTWalker | 管理遍历顺序、显式栈和提前停止 |
| ASTDumpVisitor | 输出 ASTKind、字段、源码位置和恢复状态 |
| ASTVerifier | 验证必需子节点、类型标签和列表约束 |
| StrictExprVisitor | 用于需要完整覆盖所有表达式的分析，缺少处理函数即编译失败 |

默认基类回退适用于统计、索引和筛选。需要完整处理所有表达式时，应使用没有叶节点默认回退的 StrictExprVisitor，不能让新节点静默成为空操作。需要返回值时，使用单独的带返回类型访问器；该接口不隐式用默认构造值代表未处理情况。

第一版在 ASTChildren.cpp 中集中实现子节点枚举。新增节点的分派覆盖可由编译器检查；新增子字段是否被遍历则需要节点契约测试。Visitor 仅枚举 AST 的拥有关系，不沿父节点索引遍历。

## 6 ASTContext 与事务内存

ASTContext 提供 make<T> 创建节点和 copyArray<T> 保存最终列表。节点地址在该上下文生命周期内稳定，子节点使用不拥有对象的指针。解析中的列表先保存在临时 vector 或 SmallVector 中，完成后复制进 ASTContext，再作为只读数组视图存入节点。

```cpp
template<class T>
using ASTArray = std::span<const T>;

class ASTContext {
public:
    template<class T, class... Args>
    T* make(Args&&... args);

    template<class T>
    ASTArray<T> copyArray(std::span<const T> values);

    ASTCheckpoint checkpoint() const;
    void rollback(ASTCheckpoint checkpoint);
};
```

Arena 释放内存不会自动执行节点析构。make<T> 对需要析构的类型登记精确类型回调；copyArray 对非平凡元素登记数组析构。不能假设项目的 SourceLocation 或未来所有节点字段永远是平凡类型，也不能把 std::vector、std::string 放入 Arena 后直接整体释放而跳过析构。

回滚和销毁遵守以下顺序：先逆序执行检查点之后的析构记录，再回收对应的 Arena 空间，最后恢复分配元数据。节点析构只释放自身拥有的资源，不递归销毁子节点。这样既避免重复释放，也避免长表达式链在析构时耗尽调用栈。

事务内新建节点不能提前挂到检查点之前的持久节点或列表上。解析函数先完成子结构，再构造父节点；提交前只保存在局部变量中。必须修改既有结构时，需要显式撤销日志，第一版优先通过延迟发布避免这种情况。

make<T> 还需要正确处理构造失败或析构记录登记失败：局部构造保护先清理已经构造的对象，再恢复分配状态。内存耗尽属于运行资源失败，由项目统一的失败策略处理，不转换成普通语法错误。

## 7 Parser 接口与返回约定

```cpp
class Parser {
    TokenCursor Cursor;
    ASTContext& AST;
    ParserDiagnosticEmitter Diagnostics;
    RecoveryContext Recovery;
    ParseLimits Limits;

public:
    ModuleAST* parseModule();

private:
    Stmt* parseStmt();
    Decl* parseDecl();
    BlockStmt* parseBlock();
    Expr* parseExpr(unsigned minBP = 0);
    Expr* parseUnary();
    Expr* parsePostfix();
    TypeSyntax* parseTypeSyntax();
    Expr* parseTypeExprForm();
    SimpleItem* parseSimpleItem();
    BindingPattern* parseBindingPattern();
    MatchPattern* parseMatchPattern();
    ExpectResult expect(TokenKind kind, RecoveryPolicy policy);
    RecoveryResult recover(RecoveryPolicy policy);
};
```

parseX 用于已经决定进入的必需结构，普通语法错误返回 X 的正常、缺失或错误节点，不通过 nullptr 表示失败。可选结构由调用者先判断起始符，合法省略时保存空指针或 optional。试探接口使用独立的 NoMatch 结果，不能与缺失节点混为一谈。

解析函数消费属于自身结构的 Token，并在调用者的结束符之前返回。参数、实参和泛型列表中的表达式遇到同层逗号或结束括号时停止。恢复策略通过 RAII RecoveryScope 入栈，返回或回滚时恢复，避免临时上下文泄漏到下一条语句。

返回具体类型的入口必须保持其类型契约。parseBlock 返回 BlockStmt 指针，缺少括号时构造带恢复标记的合成 BlockStmt 并保留可识别内容；不能返回并非 BlockStmt 的 MissingStmt。MissingStmt 用于返回 Stmt 指针的通用入口。其他具体节点入口采用同样的规则。

ParseLimits 至少覆盖解析嵌套深度、诊断数量、总工作量和分配预算。长左结合链可能只占较浅的解析调用栈，却产生很深的 AST；因此 Walker 也需要独立的深度或显式栈策略。限制达到后返回明确状态；仅诊断数量达到上限时可抑制输出并继续有界的解析工作。

## 8 表达式与赋值模型

Pratt 使用下表的优先级，数字越大结合越紧。普通左结合运算符使用 leftBP 等于 p、rightBP 等于 p 加 1；右结合运算符使用两者都等于 p。循环只消费优先级不低于当前 minBP 的运算符。

| 优先级 | 运算符或结构 | 结合方式 |
| --- | --- | --- |
| 1 | ?: | 右结合，专用处理 |
| 2 | ?? | 右结合 |
| 3 | 双竖线逻辑或 | 左结合 |
| 4 | && | 左结合 |
| 5 | 单竖线按位或 | 左结合 |
| 6 | ^ | 左结合 |
| 7 | & | 左结合 |
| 8 | == != | 左结合 |
| 9 | < <= > >= | 左结合 |
| 10 | << >> | 左结合 |
| 11 | + - | 左结合 |
| 12 | * / % | 左结合 |
| 前缀层 | 一元运算符、comptime、函数类型 | 按 unary 规则处理 |
| 后缀层 | 调用、索引、成员、泛型应用、递增递减 | 按 postfix 顺序构造 |

三元运算的中间部分调用完整 parseExpr，冒号后保持条件表达式的右结合。冒号的结束符归属由当前三元结构负责，嵌套三元表达式优先消费自己的冒号。comptime 表达式的操作数由 parseUnary 读取，因此 comptime a + b 对应 BinaryExpr(ComptimeExpr(a), +, b)。

postfix 从 primary 开始循环构造 CallExpr、IndexExpr、MemberExpr、GenericApplyExpr 和 PostfixUpdateExpr。普通调用与空值调用通过节点字段区分；成员访问保留 Dot、Arrow、OptionalDot 三种方式。函数类型位于 unary 的独立分支，其返回类型可以包含后缀操作；不能把它擅自提升成可无条件继续解析后缀的 primary。

assignment_op 和逗号不进入 Pratt 表。当前 simple_stmt_item 的右递归由独立节点表达。

```cpp
class ExprItem final : public SimpleItem {
    Expr* Expression;
};

class AssignmentItem final : public SimpleItem {
    Expr* Left;
    TokenKind Op;
    SourceLocation OperatorLocation;
    SimpleItem* Right;
};

class SimpleStmt final : public Stmt {
    ASTArray<SimpleItem*> Items;
};
```

a = b = c 形成右侧嵌套的 AssignmentItem。a, b = c, d 形成三个条目 a、b = c、d。工程实现可收集各段表达式和赋值运算符，随后从右向左折叠，避免极长赋值链导致递归过深。

foo(a = 1) 的等号属于 named_argument；foo((a = 1)) 和 var x = a = b 在当前文法中不合法。赋值左侧的语法是任意 expr，不能在 Parser 中只允许名称。

## 9 类型与函数构造

TypeSyntax 表示一个类型位置，其内部保存 Expr 指针。parseTypeExprForm 复用适用的表达式节点，但严格遵循 type_expr 的入口和优先级；parseTypeSyntax 只在外侧创建包装节点。

| 内容 | AST 表示与限制 |
| --- | --- |
| 类型标注 | TypeSyntax 包装语法形式，缺失时内部使用 MissingExpr |
| 函数类型 | FunctionTypeExpr，保存函数类型参数和必需返回类型 |
| 函数声明 | FunctionDecl，保存名称、泛型参数、参数、返回类型与函数体形式 |
| 匿名函数 | LambdaExpr，保存泛型参数、参数、可选返回类型和必需块体 |
| 普通参数 | Parameter 记录，名称和类型必需，默认值与省略号互斥 |
| 函数类型参数 | FunctionTypeParameter 记录，名称可选，类型必需，可带省略号 |
| 前向函数声明 | 明确的 DeclarationOnly body kind，与缺失函数体区分 |

type_expr 不直接包含顶层二元运算，但 grouped_expr 内部是完整 expr。因此冒号后写 A + B 与写 (A + B) 的语法接受情况不同。match、do、数组和字面量出现在类型位置时保留对应的语法节点。

func 的分流规则如下。语句入口看到 func 后紧跟 IDENTIFIER 时进入声明；表达式入口解析共享的函数头部，结合泛型参数、参数形式、返回类型和后续块体确定 LambdaExpr 或 FunctionTypeExpr；类型入口只按函数类型规则读取。共享读取逻辑不改变各分支的参数约束。

匿名函数参数支持默认值和泛型，而函数类型参数支持无名类型。可以先构造临时函数头部并记录特征，随后选择合法分支。确认匿名函数后发现块体损坏，继续恢复该匿名函数；不能因为错误而回退成已不满足约束的函数类型。

## 10 语句 声明与成员

ModuleAST、BlockStmt 以及类声明的主体都保存有序 Stmt 列表。DeclStmt 包装 Decl，使声明能够作为语句出现在模块、块和类体中。函数位于类中时仍使用 FunctionDecl。

| 结构 | 数据模型 |
| --- | --- |
| IfStmt | condition、thenBranch、可选 elseBranch |
| WhileStmt | condition、body |
| ClassicForStmt | 可选初始化、可选条件、步进条目列表、body |
| ForInStmt | BindingPattern、迭代表达式、body |
| SwitchStmt | 控制表达式、有序 SwitchClause 列表 |
| SwitchClause | case 或 default 标签、可选标签表达式、有序语句 |
| ReturnStmt | 可选返回表达式 |
| BreakStmt 与 ContinueStmt | 关键字和语句范围 |
| YieldStmt | Value 或 Return 模式、必需表达式 |
| DeferStmt | simple_stmt 或 block_stmt 子节点 |
| ComptimeStmt | 关键字位置和被修饰的 Stmt |

open_stmt 与 closed_stmt 不需要对应 AST 类型。parseIfStmt 递归解析完整 then 语句，再判断 else，落实最近未匹配 if 的绑定规则。恢复过程中要把可能属于外层 if 的 else 留给其所有者。

ComptimeStmt 只能包裹当前 BNF 允许修饰的语句类别，不能通过通用包装函数使 comptime return 等额外结构成为合法语法。进入普通表达式分支的 comptime 则生成 ComptimeExpr。

VarDecl 保存 var 或 const、绑定模式、可选类型和可选初始化器，同时记录源语法形式。当前规则只允许无初始化的 var IDENTIFIER；解构声明和 const 必须有初始化器。FieldDecl 保存名称和尾部种类，四种尾部分别是 None、Typed、InitializerOnly、Payload；对应的类型、初始化器和荷载列表按种类满足各自约束。

declaration 的核心解析函数不消费结尾分号。普通 var_decl 由声明语句入口消费分号，经典 for 的 for_init 复用同一核心函数，再由 for 头部消费分隔分号，避免重复消费或错误恢复跨过头部边界。

ClassDecl 保留 Forward 或 Definition 形式。EnumDecl 与 InterfaceDecl 当前要求主体；三者均保留泛型参数、base_spec 列表和结尾分号信息。base_spec 记录是否带 implements 及对应 TypeSyntax。类体里的 comptime 语句保持源码结构和顺序。

## 11 模式 属性 导入与列表

绑定与匹配使用两套节点族。NameBindingPattern 保存绑定名称；NameMatchPattern 保存路径，并区分没有构造参数括号与存在空括号两种形式。

| 绑定模式 | match 模式 |
| --- | --- |
| NameBindingPattern | NameMatchPattern |
| WildcardBindingPattern | WildcardMatchPattern |
| TupleBindingPattern | TupleMatchPattern |
| ArrayBindingPattern | ArrayMatchPattern |
| 数组末尾的 RestBinding 记录 | 数组末尾的 RestBinding 记录 |
| MissingBindingPattern 与 ErrorBindingPattern | MissingMatchPattern 与 ErrorMatchPattern |
| 不接受字面量或 or-pattern | LiteralMatchPattern、GroupedMatchPattern、OrMatchPattern |

RestBinding 保存名称或通配符以及省略号位置，只允许出现在当前规则允许的数组末尾。NameMatchPattern 的路径由分段名称及每段可选泛型实参组成，不可以简化成一个字符串。

属性保存限定名称，以及 None、Arguments 或 Value 三种后缀。Argument 使用带标签记录区分 Named、Positional 和 SpreadPositional，value 是必需 Expr；不要用一个可同时表示命名与展开的松散布尔组合。函数调用、属性调用和 generic_args 共用实参读取工具，但调用者分别管理自己的结束符。

导入保存直接模块导入或 from 导入两种形式，保留每个模块或名称、别名及源码位置。相对导入前缀按点数计算层级，其中一个 ... Token 贡献三个点，同时保留原始 Token 位置；不能把相对层级仅记成 Token 数量。

列表辅助函数接收结束符、是否允许空列表、最少元素数和尾逗号规则。至少覆盖以下差异。

| 列表 | 当前文法约束 |
| --- | --- |
| 普通调用参数与泛型实参 | 可空，不允许尾逗号 |
| 泛型形参 | 不可空，不允许尾逗号 |
| 普通参数与函数类型参数 | 可空，不允许尾逗号 |
| 字段荷载参数 | 不可空，不允许尾逗号 |
| 数组表达式与数组模式 | 可空，不允许尾逗号；模式另有末尾 rest |
| 元组表达式与元组匹配模式 | 可空；单元素必须带逗号；多元素无尾逗号 |
| 元组绑定模式 | 不可空；单元素必须带逗号；多元素无尾逗号 |
| match 分支 | 可空，不允许尾逗号 |

当前没有独立的空语句规则，因此单独的分号也应作为非法语法处理；不能因为复用列表或块解析器而自动接受它。

## 12 DiagnosticEngine 与错误节点

解析诊断使用项目已有的错误码、SourceLocation、相关位置和修复建议格式。ParserDiagnosticEmitter 负责当前解析调用的计数，以及试探期间的临时存储。正常解析的诊断送入 DiagnosticEngine；嵌套试探提交到上层缓冲，最外层提交时才送入 Engine。

诊断参数必须拥有需要延迟使用的数据，或引用生命周期稳定的 TokenBuffer。不要把事务 Arena 中即将回滚的 AST 指针、临时字符串视图或格式化对象保存为待发诊断参数。

| 情况 | 表示方式 |
| --- | --- |
| 可选结构合法省略 | 空指针、optional 或明确的形式标签 |
| 必需表达式缺失 | MissingExpr |
| 已消费损坏的表达式片段 | ErrorExpr，保存范围及必要的恢复信息 |
| 必需语句缺失 | MissingStmt |
| 无法解释的语句片段 | ErrorStmt |
| 声明局部缺失 | 尽量保留声明类型，缺失字段使用相应占位 |
| 声明整体缺失或无法分类 | 通用 Decl 入口返回 MissingDecl 或 ErrorDecl |
| 类型语法缺失 | TypeSyntax 包含 MissingExpr |
| 标点缺失 | ExpectResult 标记插入，必要时保存到恢复旁表 |

错误和缺失节点都是注册的具体 ASTKind，Visitor 和 Walker 必须处理。必需子节点在恢复后保持非空；可选子节点为空不自动表示错误。SyntaxRecoveryInfo 可按节点指针记录缺失标点和跳过片段，避免为每个正常 AST 节点永久增加完整 Token 子列表。

## 13 错误恢复协议

expect 的返回结果区分实际匹配、补入缺失 Token 和经删除多余 Token 后匹配。SourceLocation 的插入点由项目源码设施构造；补入只影响 AST 恢复记录，不修改输入 TokenBuffer。

```cpp
enum class ExpectStatus { Matched, Inserted, Recovered };

struct ExpectResult {
    TokenKind expected;
    std::optional<TokenId> actual;
    SourceLocation location;
    ExpectStatus status;
};
```

expect 首先尝试直接匹配。若当前 Token 已是合法后继或明确的外层边界，则报告缺失并返回 Inserted。否则可以在安全条件下删除一个多余 Token，例如下一 Token 正好符合预期；不得删除调用者拥有的结束符。仍无法继续时，按当前 RecoveryPolicy 同步，再匹配或补入所需 Token。

恢复策略包含当前结构的结束符、局部同步点、外层保护边界和进入时的括号信息。仅在相应嵌套层识别局部逗号、case 等；遇到失配括号时允许在明确的外层边界结束恢复。Clang 的括号跟踪工具可参考其界定符归属和关闭恢复接口。[3]

| 结构 | 局部同步点 | 必须保护的外层内容 |
| --- | --- | --- |
| 参数与实参 | 同层逗号、右圆括号 | 外层块结束、新声明等明确边界 |
| 数组与泛型实参 | 同层逗号、右方括号 | 所属语句结束或外层结束符 |
| match 模式与 guard | 对应的 => 及分支边界 | 所属 match 的右花括号 |
| match 分支表达式 | 同层逗号、右花括号 | 外层结构的结束符 |
| switch 子句 | 所属 switch 的 case、default、右花括号 | 外层语句边界 |
| 语句 | 分号、所属块结束、强语句起始符 | 所属 if 的 else 等 |
| 文件 | EOF | 没有归属的结束符应消费并诊断 |

恢复不是简单地一直跳到下一个分号。数组重复初始化、for 头部及 do 块都可能合法包含内层分号；同样，表达式中的 do 会引入合法块，不能把所有左花括号一律视为错误边界。

parseExpr 在需要操作数的位置遇到当前结束符时，生成 MissingExpr 并停在结束符之前；外层参数、声明或条件函数负责消费。parseStmt 在遇到属于调用者的边界时可返回 MissingStmt 并让上层退出当前结构；在模块顶层遇到无归属的右花括号则消费它并生成 ErrorStmt。

每个重复解析循环必须保证实际输入进度。插入缺失 Token 不算消费输入。若一轮之后既未推进，也未到达本层结束位置或返回上层，则把至少一个实际 Token 纳入错误片段后继续。工作量预算在试探回滚时不回退，避免恶意输入通过反复试探绕过限制。

## 14 公共前缀与事务

ParseTransaction 使用 RAII，默认回滚，显式 commit 后发布结果。Checkpoint 覆盖游标、ASTContext、诊断缓冲长度、恢复上下文、临时状态和恢复旁表。

| 判定位置 | 判据与提交时机 |
| --- | --- |
| 属性或数组开头的左方括号 | 平衡前瞻或严格试探属性；确认后续声明起始形式即提交 |
| func | 根据所在语法位置和函数头部特征选择；保留各分支的参数限制 |
| for 头部 | 严格试探完整绑定模式；随后匹配 in 即提交 |
| comptime | 确认允许被修饰的语句形式，否则按一元表达式读取 |
| 命名实参 | 实参入口的 IDENTIFIER 后接等号即可判定 |
| 索引或泛型应用 | 左方括号与 :: 左方括号已有明确区分 |

试探期间使用严格匹配，不允许通用 expect 通过插入 Token 将任意候选修补成成功。NoMatch 表示尚未选择该分支。确认分支后发生错误则进入普通恢复，并保留已选的节点种类。例如确认属性后接 var，变量初始化器损坏仍恢复为 VarDecl。

属性声明的起始符判断要识别 func IDENTIFIER，而不能把任意 func 都当成声明。若属性右方括号缺失，但后面出现强声明起始形式，可以通过明确的恢复策略把它作为声明候选；这属于错误输入的恢复选择，应有独立测试。

第一版优先使用前瞻和临时头部结构，减少试探时的 AST 分配。必要的试探可以使用 ASTContext 检查点。避免在所有产生式外层统一套用回溯，也不能使用固定很小的 Token 数量作为合法长前缀的语法限制。

## 15 端到端错误恢复示例

输入如下。

```cpp
func test(): int {
    var a = 1
    var b = foo(2, , 4);
    if (b > 0 {
        return b;
    } else {
        return 0;
    }
}
```

解析 var a 时需要分号，当前 var 明确开始下一条声明。解析器诊断缺少分号，记录插入点并保留该 var。解析 foo 的实参时，第二个逗号出现在需要表达式的位置，创建 MissingExpr；由参数列表消费逗号后继续读取 4。解析 if 条件后，左花括号是条件后块体的起始形式，补入右圆括号并继续解析 then 和 else。

| 应保留的结构 | 预期结果 |
| --- | --- |
| FunctionDecl test | 返回类型 int 和完整的块体结构 |
| VarDecl a | 初始化器为整数 1，记录缺少分号 |
| VarDecl b | CallExpr 的实参为 2、MissingExpr、4 |
| IfStmt | 条件 b > 0，记录缺少右圆括号 |
| then 分支 | 保留 return b |
| else 分支 | 保留 return 0，正确归属于此 if |

这个输入应形成三个主要语法诊断，不应因为同一个缺失项在多层函数中被重复报告。ASTVerifier 检查的是恢复后结构契约，而不是要求该树等同于无错误程序。

## 16 文件组织与实现顺序

| 建议文件 | 内容 |
| --- | --- |
| ASTNodes.def、ASTKind.h | 节点注册、种类枚举和分类 |
| ASTNode.h、Expr.h、Stmt.h、Decl.h、Pattern.h | 节点基类与具体声明 |
| ASTContext.h 与 cpp | 分配、数组存储、析构和检查点 |
| ASTVisitor.h、ASTWalker.h 与 cpp | 种类分派与遍历 |
| ASTChildren.cpp、ASTVerifier.cpp、ASTDump.cpp | 子节点枚举、结构验证与稳定 dump |
| TokenCursor.h 与 cpp | Token 访问和游标检查点 |
| Parser.h、Parser.cpp | 公共接口、模块入口和共享状态 |
| ParserExpr.cpp、ParserType.cpp | 表达式与类型语法 |
| ParserStmt.cpp、ParserDecl.cpp、ParserPattern.cpp | 语句、声明与模式 |
| ParserRecovery.cpp、ParserTransaction.cpp | 恢复策略和试探事务 |
| ParserDiagnosticEmitter.h 与 cpp | 连接已有 DiagnosticEngine |

第一阶段完成 ASTKind、ASTContext、Visitor、Walker、TokenCursor 和诊断适配，验证对象生命周期与分派契约。第二阶段完成名称、字面量、前缀、后缀、二元、条件、列表和赋值条目。第三阶段完成块、控制流、声明与函数。第四阶段完成类型位置、模式、属性和导入等公共前缀处理。

错误恢复随每个语法家族一起实现，不能等全部合法语法完成后再统一补上。最后阶段进行完整文法覆盖、错误样例回归和资源限制测试。

## 17 验收与测试矩阵

测试以输入源码、预期 AST 结构、诊断代码与范围、恢复后的剩余结构为主要断言。测试业务契约，避免把内部函数调用顺序固定成测试依赖。

| 类别 | 代表性用例与验收点 |
| --- | --- |
| 优先级 | a + b * c、a - b - c、a ?? b ?? c；验证树形与结合方向 |
| comptime | comptime a + b、comptime(a + b)、comptime if；验证作用范围 |
| 赋值 | a = b = c 与 a, b = c, d；验证 SimpleItem 层次 |
| 实参 | foo(a = 1) 合法，foo((a = 1)) 语法非法 |
| 元组 | ()、(x)、(x,)、(x,y)、(x,y,)；按对应语法位置区分 |
| 类型 | 顶层 A + B 与分组 (A + B)，函数类型及返回类型内后缀 |
| 函数 | 声明、匿名函数、函数类型、默认参数、无名参数与缺失块体 |
| 前缀判定 | 属性与数组、经典 for 与 for in、comptime 的两种入口 |
| 模式 | 绑定与匹配的允许集合、数组末尾 rest、构造参数括号的存在性 |
| 恢复 | 缺少分号、结束括号、操作数、参数分隔符、match 箭头 |
| 边界保护 | 错误后仍保留下一声明、else、case、default 和所属右括号 |
| 事务 | 失败试探无节点或诊断残留，嵌套提交正确，分配指针不逃逸 |
| 内存 | 带析构资源的节点及数组，回滚和释放无泄漏或重复析构 |
| 遍历 | 每个子字段被访问，默认 Visitor 不递归，Strict Visitor 覆盖完整 |
| 鲁棒性 | 每个 Token 边界截断、插入删除 Token、随机输入、深层嵌套 |

对长二元链和赋值链分别测试 Parser、Walker 及销毁过程；对非法编码与非法 Token 验证词法层始终推进，Parser 不重复输出已明确定位的词法根因。模糊测试应断言终止、无越界、预算有效，并将发现的最小失败输入固定成回归用例。

设计验收的核心结果是：ASTKind 与实际节点类型一致，必需子节点始终满足恢复契约，合法语法没有因公共前缀处理被误拒绝，错误输入不会吞掉可恢复的独立后续结构，整个 ParsedUnit 的对象和源码引用具有明确生命周期。

## 附录 A 主要节点与文法对应

| 文法范围 | 节点或字段映射 |
| --- | --- |
| module | ModuleAST 的有序 Stmt 列表 |
| stmt、basic_stmt、open_stmt、closed_stmt | 语句分流，不建立包装层节点 |
| decl_stmt、decl、attributes | DeclStmt、Decl 公共属性列表 |
| class_decl、enum_decl、interface_decl | 各自 Decl 类型与共享主体字段 |
| function_decl、callable_signature、callable_body | FunctionDecl、签名记录与 body kind |
| generic_params、param_list、param | 泛型或普通 Parameter 列表 |
| declaration、var_decl | VarDecl，保留声明形式 |
| field_decl 及各尾部 | FieldDecl 与 FieldTailKind |
| base_clause、base_spec | BaseSpec 列表与 implements 标记 |
| block_stmt、class_body | BlockStmt 或成员主体的有序 Stmt 列表 |
| simple_stmt_list、simple_stmt_item | SimpleItem 列表、ExprItem、AssignmentItem |
| for_classic_header、for_in_header | ClassicForStmt 与 ForInStmt 的头部字段 |
| switch_stmt、switch_clause、switch_label | SwitchStmt、SwitchClause 及标签种类 |
| import_stmt 及路径和别名规则 | DirectImportStmt、FromImportStmt 和导入记录 |
| conditional | ConditionalExpr |
| null_coalescing 至 mul | BinaryExpr，使用运算符种类区分 |
| unary_op、comptime_expr | UnaryExpr、ComptimeExpr |
| 调用后缀、索引后缀、成员后缀 | CallExpr、IndexExpr、MemberExpr |
| generic_args | GenericApplyExpr；路径和属性中的实参记录复用列表结构 |
| 后缀递增递减 | PostfixUpdateExpr |
| literal | LiteralExpr，保留字面量类别和原始 Token |
| grouped_expr、tuple_expr | ParenExpr、TupleExpr |
| array_expr | ArrayExpr 与 ArrayRepeatExpr |
| anonymous_function_expr | LambdaExpr |
| function_type_expr | FunctionTypeExpr |
| type_expr、type_primary | TypeSyntax 外部包装，内部复用受约束的 Expr |
| match_expr、match_arm、match_guard | MatchExpr、MatchArm 记录、可选 guard |
| block_expr | BlockExpr，包含 BlockStmt |
| binding_pattern 族 | BindingPattern 派生节点 |
| pattern 族 | MatchPattern 派生节点，竖线组合为 OrMatchPattern |
| qualified_name、pattern_path | 路径分段记录，保留每段及分隔符的位置 |

没有必要把每个辅助产生式都变成 ASTNodeBase 子类。参数、实参、属性、路径分段和分支记录可以作为拥有 SourceLocation 的值类型，内部需要遍历的 AST 指针由父节点的子节点枚举接口暴露。错误恢复所需的信息不能因为采用值类型而丢失。

## 附录 B 参考资料

本文的节点设计和解析策略以 Ink 文法及上述项目约定为准。以下官方资料用于对应的实现机制参考。

[1] [LLVM Programmer’s Manual 中的 isa、cast 与 dyn_cast 接口说明。](https://llvm.org/docs/ProgrammersManual.html#the-isa-cast-and-dyn-cast-templates)

[2] [Clang StmtVisitor 源码，展示按节点种类分派、CRTP 与基类回退。](https://clang.llvm.org/doxygen/StmtVisitor_8h_source.html)

[3] [Clang BalancedDelimiterTracker 接口，展示括号跟踪与关闭恢复。](https://clang.llvm.org/doxygen/classclang_1_1BalancedDelimiterTracker.html)

文法基线为 Ink-grammar-Rules(3).bnf。SHA256 为 dc2264f70be5c6b90e4390294b4587e19095c8eca45c700c2da5f19f79ea2392。
