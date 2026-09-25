# 可执行 IR 实现状态

当前对象模型位于 semantic/model。旧 IR 和 execution 的头文件、实现和测试目录已删除；未接入构建的旧 backend、interpreter 调用方需要重新适配。

## 当前入口与对象模型

`tokenize → parse` 已实现；`semantic::Analyzer::analyze` 仅保留空接口，始终返回 `nullptr`，尚未接通从 AST 到 `semantic::Module` 的分析链路。接口状态见 [语义分析接口](Ink-Semantic-Analysis.md)。

`Analyzer` 当前只提供入口占位方法。独立的 `NameResolver` 已提供词法作用域、实体成员作用域、名字绑定、直接成员查找、遮蔽和重载候选集合；AST 遍历、类型、初始化、调用与返回检查尚未接入，也不预置 print 声明。对象模型仍可通过工厂独立构建以下节点。

| 节点 | 操作数与结果 | 约束 |
| --- | --- | --- |
| FunctionParameter | ParameterName、outer 函数、索引、值类型、ParameterKind | createFunction 根据签名自动创建，种类为 Positional/Named/Variadic；function() 从 outer 取得函数，无重复 Owner |
| AllocaInstruction | 对象类型 → 可写指针 | 每次执行分配一个未初始化对象，存储持续到本次函数调用结束 |
| LoadInstruction | 地址 → 元素类型 | 接受可写或只读指针 |
| StoreInstruction | 地址、写入值 → void | 地址可写，元素和值类型完全相同 |
| AddInstruction | 两个同型整数 → 同型整数 | 模 2^bitWidth 加法 |
| CallInstruction | 函数值、实参 → 返回类型 | 数量和类型与签名完全匹配 |
| ReturnInstruction | 可选返回值 → void | function() 从 outer → BasicBlock → Function 查询，未挂接时为空；appendValue 校验函数签名，void 函数没有返回操作数 |

工厂只构造节点，appendValue 安排块内顺序；操作数直接引用稳定地址的对象。工厂检查上下文归属、类型等局部约束，不等价于完整图验证器。存储工厂支持 bool、整数、浮点、指针、引用、切片及这些类型的数组；源码分析尚未实现。

## 距离执行仍缺少什么

| 优先级 | 能力 | 待补内容 |
| --- | --- | --- |
| 首先 | 源码语义分析 | 将已有 NameResolver 接入 AST 声明登记，补充类型解析、表达式和语句检查，以及从 AST 构建 Module。Analyzer::analyze 当前为空实现。 |
| 首先 | 执行与后端 | 新模型的解释器或 LLVM lowering、入口选择和驱动接线。生成 Module 不会执行程序。 |
| 首先 | 外部调用 | 外部函数的声明与宿主绑定、外部链接身份、调用约定和字符串 ABI。声明不能产生输出。 |
| 首先 | 完整图验证器 | 定义先于使用、跨函数操作数、归属、支配关系、终结后指令和运行时值合法性；BasicBlock 仍允许任意符合基础条件的 Value，removeValue 可拆下仍被引用的定义。 |
| 首先 | 目标内存与生命周期 | 大小、对齐、调用栈存储、无效地址和对象存活期。源码未初始化读取检查、手动构建图和运行时检查均待实现。 |
| 随后 | 控制流 | 分支、循环、前驱/后继、到达性和完整返回路径分析；SSA 汇合需 phi 或块参数，初期可用局部内存表达状态。 |
| 随后 | 更多表达式 | 其他整数运算、浮点、比较、转换、短路求值、地址计算和成员/元素操作。 |
| 随后 | 完整语言语义 | const、泛型、comptime、用户类型、全局对象、模块/import、重载及默认/命名参数等。 |
| 随后 | 复合值和内存功能 | 聚合常量、空指针与全局地址常量、动态分配、字段/数组寻址、原子/volatile。 |
| 优化阶段 | 使用关系和变换 | 操作数遍历、use-def、替换使用、受控删除、常量折叠和内存提升。 |
| 工程阶段 | 调试和持久化 | 指令源码位置、变量调试映射、文本打印/解析、序列化和兼容性；当前引用仅在上下文内有效。 |

泛型 Decl 仍借用 AST，其所属 ParsedUnit 必须保持存活；运行时值由具体常量、形参和指令节点表示。现有 semantic 测试验证对象建模及名字绑定，不代表完整源码分析或运行时行为。
