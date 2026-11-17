# IR Module 文本格式

`ink/ir/text/module_text.h` 提供两个无异常接口：

```cpp
std::expected<std::string, core::Diagnostic> writeModuleText(const Module& ModuleValue);
std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> readModuleText(std::string_view Text);
```

读取成功后，通过 `(*Result)->module()` 访问 Module。`ParsedModule` 同时拥有 `IRContext` 和 Module；`context()` 提供类型池、常量池和默认 Builder，`types()`、`constants()` 保留为上下文的转发入口。文本读取使用该 Builder，成功返回时已清除插入点；调用方可重新设置插入点继续构建。使用读取后的任何节点时都必须保留该 owner。失败返回 Core 的 `IRTextInvalid`（`INK-I0002`）诊断，不触发 fatal IR verifier。读取错误的 `SourceRange` 是输入文本中的字节范围；调用方可补充 `SourceId` 后交给诊断引擎。

`ink/ir/verification/verifier.h` 与 `Module::verify` / `Function::verify` 同时提供非致命校验入口（`checkModule`、`checkFunction`、`verifyModule(Module, &Error)`、`verifyFunction(Function, &Error)`）与触发 `INK-I0001` ICE 的致命校验入口（`verifyModule(Module, Diagnostics)`、`verifyFunction(Function, Diagnostics)`），文本读写与运行时校验共享同一套校验实现。

## 文本示例

这是参考 LLVM IR 设计的 Ink 正式文本 IR，采用顶层 `source_filename` 与声明布局、`;` 行注释、无行尾分号、显式操作数类型、符号 ID、SSA 值和基本块，并保留记录类型的字段、方法及完整方法体：

```text
source_filename = "Demo"
%t0 = type struct
{
  field public instance i32 "X"
  method static define public i32 @f0 "sum"(i32 %a0 "a", i32 %a1 "b")
  {
    %bb0 "entry":
      %v0 = add i32 %a0, %a1
      ret i32 %v0
  }
}
%t1 = type class
{
  field private instance i32 "Value"
}
```

`class` 对应 `ObjectType`，`struct` 对应 `StructType`。文本 ID 与声明名称独立，允许无名、同名及包含任意字节的名称；当名称为空（`NAME_None`）时规范输出省略紧跟在 ID 或字段类型后的引号名称。所有定义的文本 ID 在整个 Module 内唯一；函数和全局变量使用 `@`，记录类型、参数、指令结果和基本块使用 `%`。

## 声明与引用

- 模块头：`source_filename = "Name"`。
- 记录类型：`%t0 = type struct { ... }` 或 `class`。可在类型种类后添加 `extends(Type, ...)`。可达但没有注册到 Module 的记录类型添加 `borrowed` 标记；读取后仍由 owner 持有，但不加入 Module 的注册列表。
- 字段：`field Visibility MemberKind Type ["Name"]`。可见性为 `public/private`，成员种类为 `instance/static`。
- 方法：`method MemberKind` 后接完整的函数声明或定义。方法仍由所属记录类型拥有。
- 函数：`declare Visibility [Linkage] [ABI] [Convention] ReturnType @Id ["Name"](Parameters)`；定义使用 `define`，末尾带基本块区域 `{ ... }`。固定参数写成 `[positional|named] Type %Id ["Name"]`，默认 `positional` 在规范输出中省略；可变参数在参数列表末尾写 `...`。
- 全局变量：`@Id ["Name"] = Visibility [Linkage] global Type`，其中 `Type` 是全局变量存储的元素类型，变量值本身具有 `ptr<Type>` 类型。
- `Linkage` 为 `none/import/export`（默认 `none` 省略），`ABI` 为 `ink/c`（默认 `ink` 省略），`Convention` 为 `default/cdecl/stdcall/fastcall/vectorcall`（默认 `default` 省略）。C ellipsis（`...`）标志不产生额外 Argument。
- 基本块：`%Id ["Name"]:` 后接指令；基本块顺序、参数顺序、字段顺序和方法顺序分别保留。

解析支持前向引用、递归 nominal 类型、非拓扑排列的基本块、phi 回边，以及类型位置对 type-valued Argument/Instruction 的引用。函数和全局变量的声明顺序及同一记录内的方法顺序保留，因此声明依赖必须能在保留这些顺序的前提下构造。

## 类型和常量

| 内容 | 写法 |
| --- | --- |
| 元类型、布尔、空类型 | `type`、`bool`、`void` |
| 有符号/无符号整数 | `i8/i16/i32/i64`、`u8/u16/u32/u64` |
| 浮点类型 | `f32/f64` |
| 指针 | `ptr<Type>` |
| 数组 | `[Count x Type]` |
| 函数类型 | `fn[<ABI Convention>]([positional\|named] Type, ..., [...]) -> ReturnType` |
| nominal 类型或 type-valued SSA 引用 | `%Id` |
| 整数原始位模式（在带整数类型的操作数位置） | `4294967295` 或 `0xffffffff` |
| 浮点原始位模式（在带浮点类型的操作数位置） | `2147483648` 或 `0x80000000` |
| 字节数组字符串常量（在 `[Count x u8]` 操作数位置） | `c"Bytes"` |

数字接受十进制和 `0x` 十六进制无符号 64 位值；规范输出使用十进制。整数和浮点常量保留原始位模式，因此有符号整数、负零和 NaN payload 不经过有损转换。字符串常量使用 LLVM IR 风格的 `c"..."`，保留字节长度，不添加 NUL 终止符。

所有名称（`"..."`）和字节字符串常量（`c"..."`）均使用双引号，反斜杠后跟两个十六进制数字表示一个字节，例如 `\00`、`\22`、`\5C`。规范输出会转义控制字节和非 ASCII 字节。读取也接受 `\"` 和 `\\`。空白不影响语义，`;` 引入行注释。

## 指令

```text
%v0 = add Type Value, Value
%v1 = alloca AllocatedType
%v2 = load ResultType, ptr<ResultType> Source
store ValueType Source, ptr<ValueType> Destination
%v3 = phi ResultType [Value, %Predecessor], [Value, %Predecessor]
%v4 = call ReturnType Callee(ArgType Value, ArgType Value)
call void Callee()
ret void
ret ValueType Value
br label %Target
br bool Condition, label %TrueTarget, label %FalseTarget
```

`Value` 在已知操作数类型下可以是引用（`%Id`/`@Id`）、无符号位模式字面量、字节字符串 `c"..."`，或在 `type` 类型位置下的类型表达式。有结果的指令显式记录结果类型，无结果指令不分配 ID。phi incoming 对保留顺序和重复 CFG 边；所有读入的完整图都要通过统一的类型、CFG 和 SSA 校验。

`call` 先记录 `ReturnType`，后跟 `Callee` 及圆括号内的带类型实参列表。callee 必须直接具有 `FunctionType`，可以是函数声明、方法的函数，或函数类型的 SSA 值；`ptr<fn<...>>` 需要先 `load`。`void` 调用不分配结果 ID；返回类型必须与 callee 签名一致。固定实参数量和类型必须与形参精确匹配，variadic 调用允许在全部固定实参之后追加非 void 实参。实例方法的 receiver 是签名及实参中的第一个显式参数，call 不自动添加 receiver，也不执行方法名查找或动态派发。模块外函数仍受下面的序列化边界限制。

## 共享实现与往返约定

`src/lib/IR/text/module_text_format.h` 定义正式语法约定、枚举转换和字符串转义辅助函数；`module_text_writer.cpp` 通过 `SlotTracker`、`TypeVisitor` 和 `InstructionVisitor` 直接遍历 `ir::Module` 输出文本；`module_text_reader.cpp` 采用流式 `Lexer` 与递归下降解析构建 `ParsedModule`，完整图校验统一调用 `ink/ir/verification/verifier.h`（实现在 `src/lib/IR/verification/verifier.cpp`）。

保证 `read(write(Module))` 保留图结构和语义，并使再次规范输出的文本一致。空白、注释、文本 ID 和不同种类声明之间的交错排版不作为原始文本保存；字段与方法分别输出，与当前 IR 的两份有序列表一致。超出支持范围的深度或模块外函数、参数及指令依赖会显式返回诊断；不会保存宿主地址或静默丢弃依赖。
