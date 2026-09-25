# 初版语义分析

入口为 [`semantic::analyze`](../src/include/ink/semantic/analyzer.h)，输入是成功的 `parser::ParseResult`，输出是由 `SemanticContext` 拥有的 `Module *`。输入和上下文必须使用同一个 `CompilationContext`。

```cpp
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer.h"
#include "ink/semantic/model/context.h"

ink::core::CompilationContext Compilation;
ink::core::FrontendContext Frontend(Compilation);
ink::semantic::SemanticContext Semantic(Compilation);
auto Parsed = ink::parser::parse(Frontend, ink::tokenizer::tokenize(Frontend, R"(
func main(): int32
{
  var a: int32 = 2;
  var b: int32 = 4;
  print("hello， world");
  return a+b;
}
)"));
ink::semantic::Module *ModuleValue = ink::semantic::analyze(Semantic, Parsed, "example");
```

空指针表示词法、语法或语义失败，诊断通过 Core 的 DiagnosticEngine 交给注册的 consumer。词法/语法失败时不重复报告语义错误；语义分析遇到第一个错误即停止，不暴露半成品模块。失败过程中已经分配的对象保留到上下文销毁，不会污染下一次分析的符号表。

成功模块不借用 AST：名称已驻留，字符串已复制，操作数引用上下文对象。Parsed 可以先于 Semantic 销毁；手动创建的泛型 Decl、ExprValue 仍遵守原来的借用生命周期约束。

## 目标程序的对象图

下面是对象关系示意，并非已经实现的文本 IR 格式：

```text
Module("example")
  EntryBlock
    Function("print", (string) -> void, 无函数体)
    Function("main", () -> int32)
      EntryBlock
        A  = AllocaInstruction(int32)
             StoreInstruction(A, IntegerConstant(2))
        B  = AllocaInstruction(int32)
             StoreInstruction(B, IntegerConstant(4))
             CallInstruction(Print, [StringConstant("hello， world")])
        A1 = LoadInstruction(A)
        B1 = LoadInstruction(B)
        S  = AddInstruction(A1, B1)
             ReturnInstruction(S)
```

A、A1、S 是说明标签；实际通过 `const Value &` / `const Value *` 引用对象，不通过变量名或数值 ID 查找。源码名称仅参与分析期间的作用域查找，同名遮蔽绑定对应不同地址。常量不插入基本块，通过指令操作数引用。ReturnInstruction 只保存可选返回值；function() 从 outer → BasicBlock → Function 取得所属函数，未挂接时为空。加入块时检查返回类型，移除后再挂接会重新检查目标函数。

## 支持范围

- 模块级普通函数定义和无函数体声明。先登记所有签名，再分析函数体，支持前向调用和递归引用。暂不支持重载或合并声明与定义，同模块重名报错。
- 固定数量、显式类型的参数。FunctionParameter 通过 outer 取得所属函数，保存 Name 类型的 ParameterName、零起始索引、值类型和 ParameterKind，name() 返回驻留名称；当前分析器创建 Positional 参数，模型也可通过 createFunction 的种类列表创建 Named、Variadic 参数，命名绑定与变参展开仍待实现；参数是只读传入值，需要可变副本时写 `var local = parameter;`。
- int8/16/32/64、uint8/16/32/64、bool、string 和返回位置的 void。函数必须显式声明返回类型；string 表示只读 uint8 切片。
- 整数及正负整数字面量、解码字符串、true/false、名称、括号、整数加法和位置参数调用。整数常量检查目标类型范围，不截断溢出值，支持 tokenizer 接受的各进制。
- 声明、赋值、参数和返回提供期望类型，整数常量据此定型；没有上下文时默认为 int32。加法从上下文或左操作数确定类型，并约束右操作数。已有类型的值不进行隐式数值转换。AddInstruction 的行为定义为模 2^bitWidth 加法，带符号和无符号都按位宽回绕。
- 局部 var、类型推断、显式类型的未初始化变量、简单赋值、嵌套词法块、表达式语句。先分析初始化表达式，再引入新绑定，允许读取被遮蔽的外层变量。同作用域不能重复定义。直线代码检查读取前初始化。
- 每个函数生成一个基本块，词法块只影响作用域。非 void 函数须显式返回匹配类型的值；void 函数允许无值 return，并在自然结束时补 return。return 后的语句报不可达错误。
- 每个模块预置 `print(string): void` 声明，模块级名称 print 不可重定义。这里只构建调用关系，尚未提供宿主绑定或执行输出。

const、泛型、comptime、属性、复合类型表达式、全局变量、成员访问、浮点/字符字面量、其他运算符、复合/链式赋值、命名/展开实参、默认/变长参数和控制流等尚未实现，遇到时明确诊断。递归分析限制为 256 层。按 Core 规则，实现缺口和资源限制归 InternalCompilerError；名称、类型、范围、初始化和返回错误归 User。

示例采用现有 Ink 文法 `func main(): int32` 和普通双引号。`void main()` 不是当前函数文法；`func main(): void { return a+b; }` 会报告返回类型错误。

## 后续边界

当前交付止于生成 Module，没有新增编译器 CLI、解释器、LLVM lowering 或 print 运行库实现。验证器、控制流、目标布局等缺口见 [可执行 IR 状态](Ink-Executable-IR-Status.md)。

旧 src/include/ink/ir、src/lib/ir、src/include/ink/execution、src/lib/execution 及相应测试目录已删除。原有 backend、interpreter 工具仍是未接入构建的旧接口调用方，需要基于新 Module 重写，不能作为当前执行能力使用。

测试见 [analyzer_test.cpp](../src/testcase/semantic/analyzer_test.cpp) 和 [arithmetic_return_test.cpp](../src/testcase/semantic/arithmetic_return_test.cpp)。
