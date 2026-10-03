# 语义分析接口

按需分析导入函数时使用定义模块的作用域和帧，不捕获调用处的临时块变量。编译期调用若依赖尚未完成的函数体，或要求越过仍在分析的定义、初始化语句推进模块初始化，分别报告 `INK-S0038`、`INK-S0039`、`INK-S0040`，避免执行不完整 IR 或改变编译期快照。

单模块入口为 [`Analyzer::analyze`](../src/include/ink/semantic/analyzer/analyzer.h)，多模块入口为同一类的 `analyzeModules()`；头文件和实现分别位于 `src/include/ink/semantic/analyzer` 和 `src/lib/semantic/analyzer`：

```cpp
class Analyzer
{
  public:
    ir::Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
};
```

通过 `Analyzer` 实例调用。模型和构建器统一位于 `ink::ir`；`SemanticContext` 组合独立的 `IRContext` 与 `ScopeStore`，分析器使用 `IRBuilder(Context.irContext())`。当前已实现可编译的流程骨架：检查解析结果和源码归属，通过 `IRBuilder` 创建 Context 拥有的模块、模块独立拥有的声明根及入口块。每次调用的 `AnalysisState` 直接拥有独立 `NameResolver`，从 Context 的共享根作用域开始，再进入与本次模块关联的成员作用域，按源码顺序逐条分析顶层语句。成功返回上下文拥有的 `Module *`，失败返回 `nullptr`；调用结束时销毁 resolver，作用域及绑定仍保存在 Context 中。

`analyzeModules(Context, Inputs, EntryModuleName)` 接收带模块名的已解析源码集合，先建立各模块及其成员作用域，预声明全部顶层函数签名，再解析导入并检查函数体。这样运行时调用可以引用后声明的函数或其他模块中的函数，并支持模块间相互递归。编译期调用按需分析依赖函数体；签名预声明不代表函数体已经完成，遇到编译期依赖正在分析的函数体时报告用户错误。CLI 统一调用 `analyzeModules()`，无导入的单文件也预声明顶层函数签名，保证模块独立编译与作为导入依赖时的结果一致；原 `analyze()` API 仍保留源码顺序作为兼容入口。CLI 负责按 `--module-root` 发现和读取源码依赖，Analyzer 不自行搜索文件系统；无导入的单文件解释使用虚拟模块名 `main`，不限制文件名。

`analyzeStmt()` 与 `analyzeDecl()` 从 Parser 的 `ASTNodes.def` 按 `Category` 生成严格分派。每种语句和声明均有手工声明、定义的 `analyzeXXX()`，不存在自动生成的空处理函数或基类回退；新增 AST 种类后缺少处理函数会导致编译失败，只有声明没有定义则导致链接失败。`DeclStmt` 转发到声明分派，`BlockStmt` 使用 `BlockDepthGuard` 管理嵌套深度、`NameResolver::ScopeGuard` 管理子作用域，并递归处理子语句；正常结束或提前返回时均自动恢复进入前的深度和作用域。块嵌套上限由 `INK_SEMANTIC_BLOCK_DEPTH_LIMIT` 配置，默认 256，模块和函数的成员作用域也使用 `ScopeGuard`。

当前支持空模块、块作用域、普通定参 Ink 函数定义、`import "C"` 声明、`export "C"` 定义和 `[abi("C")]` 本地定义。普通 Ink 函数必须有函数体，缺少函数体报告 `SemanticFunctionRequiresBody`（`INK-S0028`）；只有原生导入允许无体声明；`export "C"` 和 `[abi("C")]` 本地函数必须有函数体。函数签名通过检查后创建 `Function`，保存形参名称、类型、C 调用约定、Ink/C 语言链接、Local/Import/Export 原生方向及 Public/Private 可见性，并登记到当前词法作用域；成功的函数由当前 IR 块拥有。函数成员作用域绑定形参，函数体另建词法块作用域并逐条分派语句，嵌套函数继承外层查找环境，但不支持捕获外层形参。当前函数体支持显式 return、返回值类型检查、嵌套块及 if 分支的返回传播；只有所有分支都返回时，后续语句才因该 if 不可达。非 void 函数存在到达结尾的路径时报错，void 函数在可达结尾补充无值返回。源码循环的控制流分析仍待实现。函数在分析主体前临时绑定，失败时通过 RAII 销毁函数及子节点并撤销相关绑定，不影响后续同级声明的作用域或插入点。

函数内的普通 `if (Condition) Statement [else Statement]` 已支持 bool 参数、局部变量、函数返回值和常量条件，以及 else-if、嵌套和单语句分支。条件必须是 bool，不接受整数或指针的隐式真假转换；可使用 `!`、`&&`、`||` 及返回 bool 的比较表达式。普通 if 的两个分支均进行语义检查，即使条件是 bool 常量；`comptime if` 保留只分析选中分支的规则。每个运行时分支有独立词法作用域，单语句声明不会泄漏到另一分支或后续语句。

两分支从相同的入口初始化状态开始分析，汇合时仅对仍能继续执行的路径取确定初始化交集；缺少 else 时，未执行 then 的入口状态也参与交集。已经 return 的分支不参与后续初始化判定，也不会生成到汇合块的跳转；两边均 return 时不创建空汇合块。局部变量继续通过 alloca/store/load 传递，无需 phi。普通运行时分支不能通过 break/continue 条件性地控制外层编译期循环的静态展开。

逻辑非 `!` 和短路运算 `&&`、`||` 的操作数严格要求 bool，结果也为 bool。普通函数在定义处检查两侧表达式，包括因常量条件而在执行时跳过的右侧；执行时左侧仅求值一次，`&&` 只在左侧为 true 时求值右侧，`||` 只在左侧为 false 时求值右侧。lowering 先将左值保存到临时 bool 存储，再生成右侧块及汇合块，必要路径更新该存储，最后在汇合块加载结果；嵌套短路继续连接其实际结束块。`LogicalNotInstruction` 实现逻辑非，IR 的 `LogicalAndInstruction` 和 `LogicalOrInstruction` 对已经求值的两个 bool 执行运算，不承担跳过先前求值的职责。

比较运算 `==`、`!=`、`<`、`<=`、`>`、`>=` 对完全同型的整数生成 `CompareInstruction`，结果为 bool；bool 仅支持 `==` 和 `!=`。整数比较按其类型的有符号或无符号含义执行，IR 支持任意整数位宽；源码使用当前已支持的 i/u8、16、32、64、128 类型。两侧已具有类型时不隐式改变位宽或符号属性；整数文字可以按另一侧的整数类型定型，无类型约束时使用 i32。浮点、指针和其他类型比较尚未支持。比较的两个操作数从左到右各求值一次，bool 结果不将嵌套整数算术的期望类型改成 bool。

```ink
func Select(Flag: bool): i32
{
  var Result: i32;
  if (Flag)
  {
    Result = 1;
  }
  else
  {
    Result = 2;
  }
  return Result;
}
```

`analyzeType()` 支持 `void`、`bool`、`i8/i16/i32/i64/i128`、对应的无符号整数、`f16/f32/f64`、括号类型及 `*T`/`&T`；未限定指针和引用暂按 `ReadWrite` 构造，允许 `*void`，拒绝 `&void` 和 `*type`。类型名字先查词法绑定，再回退到内建类型；值不能用于类型位置。形参不能是 void 或元类型；未知类型、重名形参和冲突函数报告源码诊断。本地定义包括 `[abi("C")]` 函数，可按不同参数类型列表形成重载集，不能仅按返回类型重载；同一作用域的原生导入导出名称不能形成重载，也不能与同名本地定义混用。类型表达式递归上限由 `INK_SEMANTIC_TYPE_DEPTH_LIMIT` 配置，默认 256，每次分析开始时读取一次；深度从 0 计数，达到上限即报告 ICE 并 panic，0 会拒绝任何类型分析。

同一函数的形参名必须唯一；每个重复出现的名字使用专用诊断 `SemanticDuplicateParameterName`（`INK-S0015`），定位到该次形参名的 token，消息明确说明函数内形参名不得重复。普通符号冲突仍使用 `SemanticDuplicateName`。

`analyzeFunctionLinkage()` 返回 `std::optional<LanguageLinkage>`：没有原生导入导出前缀和 `abi` 属性时返回 Ink，完整解码字符串精确为 `"C"` 时返回 C；不截断内嵌 NUL。`[abi("C")]` 必须只有一个位置字符串字面量参数，不接受命名参数、展开参数、表达式或重复 ABI 设置。未知 ABI 和无效属性报告源码诊断；`link` 属性暂未实现。`import "C"` 与 `export "C"` 分别映射为 `FunctionBinding::Import/Export`，其他定义为 Local，且该字段独立于 ABI 和 Public/Private。

原生导入不能有函数体，导出必须在模块顶层提供函数体；普通函数和 `[abi("C")]` 本地 C ABI 函数同样必须有函数体。`private export` 合法，其声明不能被其他 Ink 模块通过源码名字访问，但仍提供原生导出名。导出名目前固定为声明名，多模块分析和字节码链接都检查重复原生导出名，多模块语义诊断为 `INK-S0049`。同一程序的 `import "C"` 按原生名和完整签名匹配已纳入的导出，不受该导出的 public/private 限制；同名签名不匹配报告 `INK-S0050`，无同名导出时保留宿主 FFI 导入。原生导入不会自行发现或读取 Ink 模块。`checkFunctionConflicts()` 拒绝相同参数列表的重复声明或定义，以及同一作用域中涉及原生导入导出名称的重载；仅设置 C ABI 的本地函数仍可按参数列表重载，重复声明合并尚未实现。

本地 C ABI 与导出定义的签名必须能由当前 ABI 实现表示：bool、8/16/32/64 位整数、f32/f64、指针和仅用于返回值的 void；不支持的定义签名在语义阶段报告 `INK-S0048`。原生导入沿用调用阶段的签名封送检查。Import 优先匹配当前程序的原生导出，未匹配时走宿主 FFI；本地 C ABI 和 Export 执行自己的字节码函数体；当前没有原生 DLL/SO 生成或回调地址桥接。

`analyzeExpr()` 支持名字、导入模块的函数成员访问、括号、整数和字符串字面量、整数常量的一元正负号、取地址 `&X`、指针解引用 `*P`、bool 逻辑非与短路、整数及 bool 比较、固定位置参数调用；未绑定的 true/false 名字作为 bool 常量。整数在参数或返回类型确定后按目标位宽解析，支持 2/8/10/16 进制和完整 128 位范围；无期望类型时默认为 i32。已具有类型的值只接受完全同型传递，不隐式窄化、改变符号或把 bool 当整数。调用先分析实参，再选择重载，不在候选试探中生成调用；逐参数比较转换等级，整数常量优先 i32，其余能容纳该常量的整数类型同级，无法唯一选择时诊断歧义。未知名字、非可调用对象、实参数量或类型不符均报告用户错误。表达式深度由 `INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT` 控制，默认 256，达到上限报告 ICE。

`&X` 要求 X 为当前函数内已确定初始化的可变局部变量，也接受括号左值和解引用结果，例如 `&*P`；临时值、函数、值形参和 const 绑定不能直接取地址。`*P` 要求非 void 指针，`*P = Value` 按指针指向类型检查写入值；赋值目标在右侧之前求值一次，`&*Call()` 同样只调用一次。`const P = &X` 固定的是指针绑定，仍可通过 `*P` 修改可变的 X；`&P` 则因 P 是 const 而拒绝。暂不支持只读指针的源码限定语法。取地址前必须完成初始化，外部函数参数尚不提供用于确定初始化分析的输出契约。

直接对 AST 编译期变量取地址或求值 `comptime &X` 会报告 `SemanticInvalidAddressOperand`（`INK-S0026`），避免将编译期绑定地址写入运行时 IR；`comptime func` 的函数体仍可通过 Alloca/Load/Store 执行指针操作，并将最终标量结果冻结为常量。非法解引用报告 `SemanticInvalidDereference`（`INK-S0027`）。函数调用期间可将局部地址传给其他 Ink 函数或 C 函数；地址不延长存储生命周期，当前运行时局部存储存活到所属函数返回，返回后继续访问报告失效地址。

字符串常量保持只读 u8 切片类型。在直接调用 C 链接函数时，字符串常量可传给可写 `*u8` 形参：`CStringInstruction` 表示每次执行都创建独立可写副本，包含完整 UTF-8 字节和额外终止 NUL，存活到调用者函数返回。不同调用的副本不共享可写存储；副本指针不能在该函数返回后继续使用。该转换仅用于 C 调用实参，不允许从函数返回字符串时隐式创建副本，也不适用于普通 Ink 函数、其他指针类型或一般切片。含内嵌 NUL 的字符串报告 `SemanticEmbeddedNull`，避免静默截断；宿主 `tryGetCString()` 只用于检查，不作为目标程序地址。调用者必须处于函数体内。

以下程序能完成语义分析，main 的基本块依次包含 `CStringInstruction`、`CallInstruction` 和返回 i32 零的 `ReturnInstruction`：

```ink
import "C" func printf(msg: *u8): i32;
func main(): i32
{
  printf("hello, world");
  return 0;
}
```

这里按源码声明将 printf 视为定参外部函数；实际 libc printf 是变参函数，当前没有 C 变参原型和实参提升支持，不能用这个声明代表完整可执行的 printf ABI。普通 IR 执行及编译期定参外部调用已接入下述 libffi 通用路径；目标代码的 ABI lowering、字符串存储 lowering 及运行时链接仍待后端实现。泛型、默认参数、变参、命名/展开实参、声明合并、数组/切片/函数类型语法以及其他未支持表达式和控制流仍报告 `SemanticUnsupported` ICE 并终止。属性目前仅支持上述 `abi`；未知 ABI、无效或未实现属性报告源码诊断。

一处可恢复的用户错误不会阻止后续同级语句的诊断；ICE 会立即终止。恢复节点有显式处理函数，但公开入口拒绝带词法或语法错误、取消、超限或缺少根节点的输入。模块名无效时报告 `SemanticConstructionFailed`。分析失败时模块中已成功分析的同级函数仍由 Context 拥有，但不会返回成功模块。

顶层函数默认 `public`，显式 `private` 仅允许定义文件内访问；局部函数始终私有，显式 `public` 局部函数报告用户错误。`Function::visibility()` 保存语义可见性，Parser 的未修饰 `Default` 在这里映射为 `Public`。`import math as library;` 绑定模块，随后可调用 `library.answer()`；`from math import answer as localAnswer;` 绑定该模块的公开函数重载集合，别名不改变原函数的模块归属和符号身份。未写别名时，直接导入使用路径最后一段作为模块名，from 导入使用原函数名。`from .math import answer;` 支持包内相对导入。导入仅在模块顶层有效，当前目标限于导入模块自身定义的函数；私有函数、缺失模块或成员、名称冲突及不支持的导入目标均报告用户诊断。

`analyze()` API 保持源码顺序，CLI 使用的 `analyzeModules()` 对单个或多个输入统一预声明顶层函数签名。内建名称的统一登记、完整类型/表达式分析、泛型实例化及完整结果验证尚未实现；辅助类的泛型绑定能力可以独立使用，不表示 Analyzer 已支持泛型源码。

诊断通过 Core 的 `DiagnosticEngine::report<Kind>(SourceId, SourceRange, Arguments...)` 直接构造并报告，保留参数数量和类型的编译期检查。`AnalysisState::report<Kind>(SourceRange, Arguments...)` 自动使用本次分析的 Context 和 Source；在分析状态创建前，入口直接调用 Engine 的重载。诊断报告与失败返回分别处理。

实现按职责分组在 `src/lib/semantic/analyzer` 的 `expr`（表达式与转换）、`stmt`（语句与控制流）、`decl`（声明）和 `type`（类型）子目录中；入口、公共求值辅助和共享头文件保留在父目录，所有处理方法仍属于同一个 `Analyzer` 类：

| 文件 | 职责 |
| --- | --- |
| `analyzer.cpp` | 模块分析入口、未支持诊断，以及 `MissingStmt`、`ErrorStmt`、`MissingDecl`、`ErrorDecl` 的未支持处理 |
| `analyzer_modules.cpp` | 多模块签名预声明、导入解析和函数体分析入口 |
| `analyzer_module_functions.cpp` | 按需准备跨模块编译期函数依赖，检查未完成函数体的循环依赖 |
| `analyzer_evaluation.cpp` | 公共编译期直接求值入口、函数执行桥接与执行状态诊断 |
| `analyzer_internal.h` | 各实现文件共享的私有 `AnalysisState`、`ExpressionResult` 定义与辅助函数声明 |
| `expr/analyzer_expr.cpp` | 表达式分派与嵌套深度检查 |
| `expr/analyzer_binary.cpp` | 二元运算与布尔短路 |
| `expr/analyzer_call.cpp` | 重载选择、实参分析与调用 |
| `expr/analyzer_conversion.cpp` | 整数常量解析、表达式转换与类型诊断描述 |
| `expr/analyzer_literal.cpp` | 整数与字符串字面量 |
| `expr/analyzer_name.cpp` | 表达式名字解析、可寻址左值及赋值目标解析、初始化与捕获检查 |
| `expr/analyzer_member.cpp` | 导入模块的公开函数成员查找与访问检查 |
| `expr/analyzer_paren.cpp` | 括号表达式 |
| `expr/analyzer_unary.cpp` | 一元表达式、取地址及指针解引用 |
| `expr/analyzer_update.cpp` | 前后缀递增、递减的共享语义 |
| `stmt/analyzer_stmt.cpp` | 语句分派、简单语句、块作用域及声明语句转发 |
| `stmt/analyzer_if.cpp`、`stmt/analyzer_switch.cpp` | 条件与 switch 语句 |
| `stmt/analyzer_while.cpp`、`stmt/analyzer_classic_for.cpp`、`stmt/analyzer_for_in.cpp` | 循环语句 |
| `stmt/analyzer_return.cpp` | 返回语句与返回值类型检查 |
| `stmt/analyzer_break.cpp`、`stmt/analyzer_continue.cpp`、`stmt/analyzer_yield.cpp`、`stmt/analyzer_defer.cpp` | break、continue、yield 与 defer 语句 |
| `stmt/analyzer_import.cpp` | 直接导入与 from 导入 |
| `decl/analyzer_decl.cpp` | 声明分派与字段声明 |
| `decl/analyzer_variable.cpp` | 编译期变量和运行时局部变量、初始化与存储绑定 |
| `stmt/analyzer_assignment.cpp` | 直接及间接赋值顺序、复合赋值和变量可写性检查 |
| `decl/analyzer_function.cpp` | 函数签名、链接方式、名称冲突、形参作用域及函数体分析 |
| `decl/analyzer_class.cpp` | 类声明 |
| `decl/analyzer_enum.cpp` | 枚举声明 |
| `decl/analyzer_interface.cpp` | 接口声明 |
| `type/analyzer_type.cpp` | 基础类型名称、括号类型、指针与引用类型解析 |

comptime 复用普通 AST 节点上的 `isComptime()` 标记。`SemanticContext::comptimeState()` 持有 `ink::execution::ExecutionEngine`、编译期变量的稳定绑定描述和函数的编译期专用标记及诊断来源。`analyze()` 从首句到末句完成分析及编译期执行；CLI 使用的 `analyzeModules()` 先建立函数签名和导入绑定，再分析主体，模块对象在此期间持续可写；每次计算得到的常量保存当时快照，后续修改不会改变已生成的常量。分析帧保存局部编译期变量，实际编译期调用另建调用帧；普通运行时局部仅生成 alloca/store/load，不分配编译期对象。

当前支持整数/bool 编译期变量与常量、显式编译期表达式和块、赋值及复合赋值、前后缀递增递减、整数运算/比较和 bool 短路。`comptime if` 选择活动分支，`comptime while/for` 按每轮独立帧展开；完整编译期块中的普通 if/while/for 按实际路径执行，支持 break/continue。运行时函数支持 bool 条件的普通 if/else/else-if 及嵌套分支，源码循环仍未实现。编译期变量可变，`const` 不可写；`var B = comptime Expr` 的 B 仍是运行时变量，其值不能被后续编译期表达式读取。

编译期变量也可保存浮点常量和字符串常量，用于保存外部函数结果或后续传参。浮点常量目前可来自外部函数；这不增加浮点字面量解析或浮点算术支持。字符串变量保存不可变常量，其外部指针参数转换使用本次调用的独立副本。

所有非泛型 Ink 函数都检查并生成 IR 函数体，包括未调用的显式 `comptime func`。`analyze()` 在定义位置完成此过程；CLI 使用的 `analyzeModules()` 先预声明签名，再完成函数体分析。编译期调用将已求值的实参转换为 `ExecutionValueRef`，通过 `ExecutionEngine::execute()` 按需准备并执行该函数的字节码，再将可表示的返回值转换为常量。每次调用在 `ExecutionMachine` 的显式调用栈上建立独立帧，用固定槽位保存形参和中间结果，并记录本次局部存储；不保存供调用时重新解释的函数 AST、定义作用域快照或函数体回调。名称绑定、重载选择和函数体中的编译期常量已在生成 IR 时确定，后续声明及模块对象修改不会改变已生成的 IR。`import "C"` 调用由同一执行入口进入外部函数适配。

显式 `comptime func` 要求有 Ink 函数体，即使未调用也检查函数体；运行时调用或把它作为运行时值使用会报告 `SemanticComptimeFunctionAtRuntime`。缺少函数体和非 Ink 语言链接分别报告 `SemanticComptimeFunctionRequiresBody` 和 `SemanticComptimeFunctionLinkage`。普通函数与编译期函数共同支持 IR 的参数读取、局部存储、取地址及指针读写、整数加法、bool 逻辑与短路、整数及 bool 比较、bool 条件分支、嵌套调用及返回；bool 形参可作为普通 if 条件。普通 while/for、其余未接入的算术运算、复合赋值及参数写入仍按普通函数规则诊断。显式 `comptime if/while/for` 可以在定义处选择或展开代码，其条件不能依赖尚未取得实参的普通形参。跨模块编译期调用在执行前通过 `prepareComptimeFunctions()` 准备被调用函数及其函数引用依赖；按需分析某个函数体前，按源码顺序处理其定义之前尚未处理的模块语句，并先完成更早的待分析函数体，以保留定义处的编译期快照；每条语句只处理一次。依赖正在分析的函数体时报告 `SemanticComptimeBodyDependency`，已完成函数体之间的普通递归仍由执行预算约束。泛型函数保留 AST 并延迟实例化的流程仍待实现。

```ink
comptime func AddOne(X: i32): i32
{
  return X + 1;
}

func Value(): i32 { return comptime AddOne(5); }
```

显式编译期表达式由语义层按源码顺序和实际控制流求值，其中的函数调用执行已生成 IR 对应的字节码。声明到达初始化位置时取得结果，后续转换、存储和产物生成使用该结果；初始化结果存入对象后，读取变量取得对象当前值，不重新执行初始化表达式。每次函数调用及每轮编译期循环建立独立帧，执行各自的指令或静态展开路径。预算耗尽或取消会停止对应引擎，避免在已发生部分副作用的状态上重放请求。`ExecutionLimits` 的全部默认值通过 `ConfigManager::getSize()` 读取：`INK_EXECUTION_MAX_STEPS` 默认 100000、`INK_EXECUTION_MAX_OBJECTS` 默认 16384、`INK_EXECUTION_MAX_STORAGE_BYTES` 默认 67108864（64 MiB）、`INK_EXECUTION_MAX_CALL_DEPTH` 默认 256、`INK_EXECUTION_MAX_EVALUATION_DEPTH` 默认 64。Limits 构造时读取快照，非法配置回退到 `config.def` 默认值，显式设置字段优先；求值深度共同约束语义求值和嵌套函数调用，Ink 调用本身使用显式 VM 栈，不为每层调用递归进入 C++ 解释函数。存储字节预算统计累计分配的 Cell/Buffer 对象及其直接拥有的字节缓冲区，释放不退还预算；不可变值快照由 Heap 单独管理。语义 `endFrame()`、VM 调用帧清理和求值深度退出在失败后仍允许执行。

执行引擎和帧直接保存状态，不使用 Impl。项目自有的 `ExecutionInteger` 提供精确位宽整数运算，使用标准无符号运算实现进位、借位、乘除和移位，不依赖 LLVM APInt 或宿主有符号溢出行为；常量池边界仍使用 `ir::IntegerBits`。

### IR 执行入口与执行值

execution 的公共头与实现按 `bytecode/`、`runtime/`、`bridge/`、`engine/`、`memory/`、`value/`、`support/`、`ffi/` 对称组织。`runtime/` 定义不含 IR 引用的值、类型 ID 和存储布局；`bridge/` 负责语义与运行时表示的转换；`bytecode/` 编译指令，`engine/` 管理链接、预算与执行。语义绑定帧 `ExecutionFrame` 的构造和析构位于 `engine/execution_frame.cpp`，函数执行由 `engine/execution_machine*.cpp` 管理独立的 VM 调用帧；执行状态的 Core 诊断适配位于 `support/execution_diagnostic.cpp`。完整执行表示、缓存与预算见 [Ink 执行字节码](Ink-Execution-Bytecode.md)。

[`ExecutionObject`](../src/include/ink/execution/support/execution_object.h) 提供稳定地址的执行对象基类；[`ExecutionValue`](../src/include/ink/execution/value/execution_value.h) 是带 IR 类型的抽象值基类，同一头文件保留 `ExecutionValueRef` 与 `ExecutionValueResult`。七种不可变子类各有独立头文件和实现：

| 值子类 | 公共头 |
| --- | --- |
| `ExecutionVoidValue` | [`value/execution_void_value.h`](../src/include/ink/execution/value/execution_void_value.h) |
| `ExecutionBoolValue` | [`value/execution_bool_value.h`](../src/include/ink/execution/value/execution_bool_value.h) |
| `ExecutionIntegerValue` | [`value/execution_integer_value.h`](../src/include/ink/execution/value/execution_integer_value.h) |
| `ExecutionFloatValue` | [`value/execution_float_value.h`](../src/include/ink/execution/value/execution_float_value.h) |
| `ExecutionStringValue` | [`value/execution_string_value.h`](../src/include/ink/execution/value/execution_string_value.h) |
| `ExecutionPointerValue` | [`value/execution_pointer_value.h`](../src/include/ink/execution/value/execution_pointer_value.h) |
| `ExecutionFunctionValue` | [`value/execution_function_value.h`](../src/include/ink/execution/value/execution_function_value.h) |

整数与字符串拥有自己的内容，不借用常量池载荷。`ExecutionValueRef` 通过 RAII 引用计数共享只读值对象，复制引用不深拷贝载荷，最后一个引用释放时销毁值对象；这是显式所有权机制，不是 GC。标量和字符串结果可以越过创建它们的帧或引擎生命周期，但值所借用的 IR 类型仍须保持有效，函数值还要求被引用的 IR 函数保持有效。

[`ExecutionHeap`](../src/include/ink/execution/memory/execution_heap.h) 作为值和存储的统一创建入口，工厂校验类型、上下文和载荷，失败返回空 `ExecutionValueRef` 并提供显式状态。`ExecutionValueResult` 保存状态和一个值引用，默认结果为失败；`Success` 配空引用会规范化为 `InvalidArguments`。非空引用与当前有效性分开：指针结果可以在返回后因存储结束而失效，此时仍能查询其种类和失效状态。运行中间结果不进入 `ConstantPool`。

`ExecutionEngine::execute(const ir::Function &, std::span<const ExecutionValueRef>)` 执行指定具体函数，返回 `ExecutionValueResult`。`ExecutionCompiler` 将该函数的 IR 降低为连续指令和相对槽位，`ExecutionMachine` 按需缓存产物，并在每次调用的独立 VM 帧中执行。当前支持 `AllocaInstruction`、`StoreInstruction`、`LoadInstruction`、整数 `AddInstruction`、`LogicalNotInstruction`、`LogicalAndInstruction`、`LogicalOrInstruction`、`CompareInstruction`、`CStringInstruction`、`CallInstruction`、`BranchInstruction`、`ConditionalBranchInstruction` 和 `ReturnInstruction`，包括嵌套 Ink 调用及 C 外部调用；基本块内的 Function 声明降低为函数值操作，不执行函数体。整数位宽与符号在编译时选定操作码，满足局部地址使用约束的 Alloca/Load/Store 使用专门的 Local 指令。Branch 和 ConditionalBranch 降低为已经定位 PC 的 `Jump` 和 `JumpIf`，不把基本块存储顺序当作控制流。普通执行与编译期函数调用共享这一执行路径，未选中的分支不会执行。源码循环 lowering 仍未实现。

字节码操作码及 `Operands[0..3]` 各项的操作数种类统一登记在 [`instruction.def`](../src/include/ink/execution/bytecode/instruction.def)。VM 分派直接更新当前帧的 PC 和结果槽；Call 压入被调用者帧，Return 弹出该帧并将返回值写回调用者的目标槽，失败通过 `ExecutionStatus` 传播。对外语义入口和退出结果保留 `ExecutionValueRef` / `ExecutionValueResult`，由 `SemanticValueBridge` 转换；VM 帧、受控存储和原生调用路径使用独立的 `RuntimeValue`，常用标量直接保存在 `Bits` 中。执行镜像、存储布局和调用计划不保存 IR 指针。

`ExecutionLinker` 按 `FunctionId` 持有执行函数并按需链接调用；类型与函数身份在桥接层登记，执行产物只保存 ID。`ExecutableFunction` 的 `InitialSlots` 拥有预编码常量，`ConstantData` 拥有字符串字节，`Layouts` 共享有主的 `RuntimeTypeTable`。整数内存读写使用 `LoadI8/I16/I32/I64` 和 `StoreI8/I16/I32/I64`；满足地址不逃逸条件的 Local 指令访问固定帧单元，再次分配时重置该单元，仍按动态分配次数计入预算。布局保留权限、指向类型和完整函数签名，类型身份不因存储大小相同而合并。语义层的泛型实例化和 comptime 顺序求值保持原有职责。`ExecutionLinker(ExecutionImage)` 另支持不持有桥接层的自有镜像执行；代码和描述准备完整后，源 IR 可以销毁，VM 继续执行函数调用、存储及外部调用。按需编译入口仍通过语义桥接访问存活的 Context。

[`ExecutionInstructionResult`](../src/include/ink/execution/support/execution_instruction_result.h) 及其 `support/execution_instruction_result.cpp` 实现保留为公共兼容类型；字节码 VM 不通过它表达继续、跳转或返回，也不再使用旧的逐 IR 指令处理函数目录。

函数形参与指令结果保存在 `ExecutionMachine` 当前调用帧的固定槽位数组中，不再通过 `ExecutionFrame` 的 IR 对象哈希表查找。`resolveValue(Value, Frame)` 只服务于语义值解析：将常量、函数身份或当前/父语义帧的绑定转换为执行值，没有可用绑定时返回 `RuntimeValue`；它既不读取 VM 槽位，也不执行指令。一次 Call 的结果被后续多个操作数使用时只读取其结果槽，不重新调用函数；再次调用函数使用新的槽位内容。代码缓存不保存跨调用复用的计算结果。

`ExecutionStorage` 与语义值对象同属 `ExecutionObject` 体系，Heap 将可观察地址的存储交给 [`ExecutionMemoryManager`](../src/include/ink/execution/memory/execution_memory_manager.h) 独占管理。管理器直接接收 `StorageLayout`，不依赖 IRContext，负责 Cell/Buffer 的分配、释放、存储身份、代次、数量及字节预算，以及原生返回地址到已管理存储的反向查找。`ExecutionCell` 保存运行时类型 ID、布局、初始化、可写性和运行时占位状态；本机 ABI 下的 bool、i/u8、i/u16、i/u32、i/u64、f32、f64 使用稳定且正确对齐的真实标量内存，其他类型保存 `RuntimeValue` 载荷，不具有可导出给 C 的原生布局。`ExecutionBuffer` 保存固定大小的字节数组。VM 不逃逸局部变量位于固定帧单元，仍通过同一管理器收取累计分配预算。

`allocateValue()`、`loadValue()` 和 `storeValue()` 通过 Cell 检查类型、初始化、可写性及生命周期。具有原生布局的 Cell 以真实内存为当前值的唯一依据：Ink store 和 C 写入修改同一位置，后续 load 从内存取得新的值快照；VM 内部可保存为标量槽位载荷，对外值接口返回不可变 `ExecutionValueRef`，已读取的结果不随之后的写入改变。具有语言可见地址的 Alloca 仍以 `ExecutionPlace` 记录存储身份，重复取地址得到同一内存地址；导出的是 Cell 的标量数据地址，不是 Cell 管理对象的 C++ 地址。指针变量等尚无原生布局的 Cell 可在 Ink 内部被取地址和读写，但不能作为 `T**` 输出参数传给 C。

`ExecutionPlace` 和 `ExecutionPointer` 的 Buffer 分支保存非拥有的 `ExecutionStorageRef`。该句柄以弱控制块区分不同内存管理器的生命周期，并记录槽位及代次；帧退出或显式 `Heap.release()` 经管理器真正销毁存储，槽位重用时递增代次，旧句柄不会访问新对象，即使引擎在同一宿主地址重建也不会混淆身份。释放不返还累计存储分配预算，Cell 和 Buffer 均受 `INK_EXECUTION_MAX_OBJECTS` 和 `INK_EXECUTION_MAX_STORAGE_BYTES` 限制。管理器分别提供当前存活量与累计分配量的统计。保留或复制 Pointer 值只保留弱句柄，不延长存储寿命。

`ExecutionPointer` 还支持空指针和不透明原生地址。空指针是合法指针值；缺少存储身份或偏移越界返回 `InvalidPlace`，曾有效但已释放的存储返回 `ExpiredPlace`。Cell 和 Buffer 指针保存字节偏移，允许表示尾后指针，但不能解引用尾后位置。原生函数返回已管理存储的起始、内部或尾后地址时，管理器恢复原有身份和偏移；按 Cell 原类型读取仅允许零偏移，原生标量的内部地址可经 `*u8` 在范围内按字节读写，写入仍检查可写性和初始化状态。Buffer 读写限于 8 位整数。上述偏移用于表示原生返回地址，不增加源码指针算术语法；无法匹配管理器存储的不透明原生地址只在 FFI 参数或返回值间流转，引擎不执行任意宿主地址读写。

`CStringInstruction` 创建的缓冲区由 Heap 分配，释放时机归执行该指令的函数帧管理；该函数返回时真正释放缓冲区，FFI 返回别名仍只是指向同一分配身份的弱句柄，再访问报告 `ExpiredPlace`。例如被调用的 Ink 函数创建 CString 并返回其内部地址，调用者不能继续通过该地址读取或调用原生函数。执行对象继承及 Heap 管理目前不包含 Ink class 实例、字段位置或聚合值的执行语义。

`allocate()`、`load()` 和 `store()` 是语义分析中常量边界的适配入口，负责常量与执行值之间的转换，并分别复用 `allocateValue()`、`loadValue()`、`storeValue()` 的存储实现。`load()` 返回 `ExecutionResult`；函数调用统一使用 `execute()` 并返回 `ExecutionValueResult`。`ExecutionValueRef::toConstant()` 转发到值对象，仅将同一上下文的 bool、整数、浮点和字符串冻结为常量；指针和函数拒绝冻结，void 表示为成功且无常量结果。语义层在编译期调用的返回边界完成冻结，普通执行保留执行值引用，不要求结果一定可表示为 IR 常量。

逻辑和比较的源码回归使用 `src/testcase/execution/programs` 中 6 个独立 `main` 程序，共执行 56 项成功结果检查，覆盖 bool 真值表、全部整数比较、符号及位宽、短路与嵌套汇合、普通及编译期调用。每项输出用例名，核对辅助函数的 i32 结果并打印 `PASS`；不匹配时打印 `FAIL`、退出 1，全部通过退出 0。操作数副作用输出位于用例名与结果之间，验证从左到右、右侧被跳过及每次只执行一次。8 份类型错误与 3 份普通 Ink 无体声明的输入另放在 `src/testcase/execution/cli/inputs`；类型错误仍覆盖不可执行右侧须通过定义处检查。[`source_program_tests.cmake`](../src/testcase/execution/cli/source_program_tests.cmake) 统一登记 HelloWorld 和逻辑源码测试，在 Windows/Linux 注册 6 项成功程序和 11 项负向测试，共 17 项 `ExecutionSourceTest.Logical.*` CTest，共用 `source_program_test.cmake` 以默认 `main` 运行真实 `inkc`，通过 `OUTPUT_FILE` 捕获并核对完整 stdout、退出码和诊断；常规输出归一化 CRLF，HelloWorld 另外用 `STDOUT_HEX` 校验原始换行字节。

### 编译期外部调用

`ExecutionEngine::execute()` 对本地 C ABI 和原生导出函数执行其字节码函数体。原生导入先匹配当前程序已纳入的同名同签名导出，未匹配时使用外部调用路径，在当前宿主进程的已加载符号中查找声明原名，不加载新的动态库、不改写符号名。Linux 使用 `dlsym(RTLD_DEFAULT, ...)`；Windows 使用 `GetProcAddress`，依次搜索当前可执行文件、当前 CRT 和其他已加载模块。Windows 源码直接声明 `_write`，Linux 源码声明 `write`。此执行路径不生成原生 DLL/SO，也不能提供可传给 C 的本地回调地址。

`ffi/external_function.cpp` 保留语义值公共调用条件与符号查找接口；`ffi/native_symbol.cpp` 单独封装平台 API。VM 使用 `runtime_ffi_type.cpp`、`runtime_ffi_argument.cpp` 和 `runtime_ffi_call.cpp`：根据运行时布局一次准备 libffi 签名、参数封送描述和返回值描述，缓存 `NativeCallPlan`，每次直接封送 `RuntimeValue`，不访问 IR 类型。原有 `ffi_type.cpp`、`ffi_argument.cpp`、`ffi_call.cpp` 保留语义值适配接口；本次调用的临时缓冲区仍由调用者 Heap 中的内存管理器唯一拥有并按返回指针规则释放。execution 链接仓库已有的 `ink::libffi`，不定义针对某个函数的 C++ 签名，也没有函数名称白名单。外部调用要求本机兼容目标，符号不存在返回 `SymbolNotFound`，尚无法封送的签名返回 `UnsupportedExternalSignature`。

`callExternalFunction()` 和 `callWithLibffi()` 显式接收调用者的 `ExecutionHeap &`、`std::span<const ExecutionValueRef>` 并返回 `ExecutionValueResult`。具体封送在 `FfiArgument::prepare()` 中检查上下文、类型和有效载荷；IR 兼容重载仅接受已有常量，同型但尚未求值的 IR 节点返回 `RuntimeValue`，需要先由执行引擎取得结果。每个参数对象禁止复制和移动，参数数组在取地址前完成分配，其原生值及缓冲区在调用期间保持稳定；Heap 必须比这些参数对象长寿。准备失败时 `address()` 为空，不进入原生调用，已经准备的临时缓冲区按 RAII 清理。

目前支持定参 C 调用：bool、i/u8、i/u16、i/u32、i/u64、f32、f64、指针参数与返回值，以及 void 返回。指针实参可为 null、当前内存管理器内活动且已初始化的原生标量 Cell、活动缓冲区地址或原生不透明地址。Cell 借用检查所属管理器、类型、偏移、初始化和访问权限，通过后将真实标量内存直接传给 C；相同局部变量的别名共享地址，调用后无需临时副本写回。没有原生布局的 Cell（包括指针变量、i128 和 f16）不能借此传入 FFI。字符串执行值可转换为 `*u8`/`*void` 的独立副本。零参数函数和超出寄存器参数数量的函数使用同一路径；聚合、f16、其他整数位宽和变参仍在调用前拒绝。原生指针返回可以继续用于普通 IR 执行和 FFI 调用，但不能穿过编译期常量冻结边界。源码浮点字面量的语义分析仍未接入。

Ink 声明须与实际 C 函数的 ABI 一致；动态符号查找不提供原生函数的参数类型信息。当前仓库的 libffi 构建支持 Windows x64 和 Linux x86-64。以下写入声明中的第二个参数也可以使用 `*void`：

| 平台 | Ink 声明 |
| --- | --- |
| Linux x86-64 | `import "C" func write(Fd: i32, Buffer: *u8, Count: u64): i64;` |
| Windows x64 | `import "C" func _write(Fd: i32, Buffer: *u8, Count: u32): i32;` |

```ink
import "C" func abs(Value: i32): i32;
import "C" func strlen(Text: *u8): u64;
comptime var Magnitude: i32 = abs(-7);
comptime var Length: u64 = strlen("hello");

// Windows x64；Linux 使用上表中的 write 声明。
import "C" func _write(Fd: i32, Buffer: *u8, Count: u32): i32;
comptime var Written: i32 = _write(1, "hello\n", 6);
```

直接传入 FFI 的字符串执行值按原始 UTF-8 字节复制到调用者 Heap 中本次调用独有的可写 `ExecutionBuffer`，保留内嵌 NUL，并追加一个结尾 NUL。若原生函数返回该缓冲区内或尾后的地址，结果保存弱存储身份及偏移，调用层通过 `FfiArgument::promoteBuffer()` 明确撤销参数对象的临时清理责任，改由 Heap 保留副本直到显式 `release()` 或 Heap 销毁；返回指针本身不保活缓冲区。其余副本随参数对象在调用结束时立即释放，独立使用 `FfiArgument` 时则在重新准备或销毁时释放。原字符串和常量池内容不受写入影响，外部函数仅保存裸指针不会触发生命周期提升。已有缓冲区指针保留原有释放规则，FFI 返回别名不会提升 CString 等帧缓冲区的生命周期。直接从语义求值进入 FFI 的字符串适配不生成 `CStringInstruction`；已生成的函数 IR 中仍使用该指令。通用调用层不推断长度参数、输入输出方向或错误码的含义，不执行函数专属的长度检查、CRT handler 安装或 SIGPIPE 处理；这些行为遵循实际原生函数的契约。源程序调用产生的输出属于其外部副作用，编译器自身日志仍使用 spdlog。

外部调用遵守相同的执行预算和调用深度限制。编译期调用按源码顺序和实际控制流执行，后续读取已初始化对象不会重跑其外部初始化调用；函数调用结果的多次使用读取当前 VM 帧中的结果槽。编译期测试从源码经过 tokenizer、parser 和 Analyzer 验证外部调用；[`entry_execution_test.cpp`](../src/testcase/semantic/entry_execution_test.cpp) 另覆盖“源码 → tokenizer/parser AST → semantic IR → 查找普通 Entry 函数 → `Engine.execute()` → `_write`/`write` → 管道断言”，检查运行时参数、嵌套调用、UTF-8 字节、返回值及同一调用结果多次使用不重复写入。[`address_of_test.cpp`](../src/testcase/semantic/address_of_test.cpp) 覆盖取地址、间接赋值、多级指针、别名、初始化和 const 检查、comptime 边界、求值次数，以及 C 修改局部变量、返回受管理地址和地址逃逸后的失效。独立 ABI 测试覆盖标量、指针、void、零参数、多参数和字符串副本。

执行引擎及底层算术、存储操作通过 `ExecutionStatus` 返回结果，不直接输出诊断。[`makeExecutionDiagnostic()`](../src/include/ink/execution/support/execution_diagnostic.h) 将失败状态、源码位置和调用上下文转换为 Core 的具体 `Execution*` 诊断，由语义或 CLI 调用边界交给 `DiagnosticEngine` 报告，已报告的失败在传播时不重复报告。诊断编号、默认原因文案和格式模板统一定义在 Core；原因文案不绑定编译期或运行时，调用方分别提供编译期求值或入口执行上下文。`Success` 和 `Cancelled` 不生成诊断，取消仍作为失败状态向上传播。

运行时值读取、除零、非法移位、整数溢出、只读对象写入、未初始化或失效对象读取、类型和实参不符、缺失函数体、外部符号缺失及不支持的外部函数签名分别报告对应的用户错误。无效帧、绑定、位置或 Context、未支持的执行操作、宿主 ABI 不匹配及执行预算耗尽属于 ICE；底层仍返回状态，边界报告 ICE 时遵循 Core 的立即 panic 策略。旧通用诊断 `SemanticComptimeFailure` 已移除，编号 `INK-S0021` 保留而不复用。

语义层提前发现的非法赋值保留 `SemanticInvalidAssignment`。完整编译期块不能通过 return 越过运行时函数边界，违反时报告 `SemanticComptimeReturnAcrossRuntimeBoundary`。类型元值、聚合、通用指针算术与目标布局、defer/yield、泛型及完整闭合验证仍未接入；函数导入和按需准备跨模块编译期函数依赖已支持；不支持的编译期执行操作显式失败，不调用运行时后端。

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
- 名称何时可见由调用方选择登记时机。Analyzer 在名字绑定之上实现模块函数导入及 Public/Private 访问检查；NameResolver 自身不决定导出权限。继承查找、聚合成员访问控制、条件声明激活及完整泛型定义环境仍需后续实现。

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
