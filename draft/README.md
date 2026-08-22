# Ink 语言设计议题

Ink 采用逐项讨论、逐项落稿的设计方式。每个议题独立保存；只有全部议题完成并经过一致性检查后，才合并生成 `language-draft.md`。

基础语言前端与模块规则在 [`tokenizer/README.md`](./tokenizer/README.md)、[`parser/README.md`](./parser/README.md) 与 [`modules/README.md`](./modules/README.md) 中独立讨论；本表继续保存此前已经确认的语义议题。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 运行时检查、底层契约与 UB | 已确认，议题 04、32、51、54 与 Parser 议题 25 修订 | [`topics/01-safety-model.md`](./topics/01-safety-model.md) |
| 02 | 值、复制与不可复制类型 | 已确认，议题 27、32、43—45、54、60、61、69 与 Parser 议题 26 补充 | [`topics/02-values-copy-noncopyable.md`](./topics/02-values-copy-noncopyable.md) |
| 03 | 析构、RAII 与 `defer` | 已确认，析构声明改为 `func ~ClassName()`；议题 27、32、48、51、54 修订；Parser 议题 09、18、25、26、31 确认 block、表达式、跳转、`defer` 与函数声明边界 | [`topics/03-raii-destructor-defer.md`](./topics/03-raii-destructor-defer.md) |
| 04 | 指针、引用、数组与切片 | 已确认，议题 31、32、43、44、54、61 与 Parser 议题 17、19、25、29、30 补充 | [`topics/04-pointers-references-arrays-slices.md`](./topics/04-pointers-references-arrays-slices.md) |
| 05 | 基础类型与 `ptrsize` | 已确认 | [`topics/05-primitive-types-ptrsize.md`](./topics/05-primitive-types-ptrsize.md) |
| 06 | 构造函数、隐式构造与字面量初始化 | 已确认，构造声明改为 `func ClassName(...)`，并采用 C++ 风格构造初始化列表；议题 54、72 修订，Parser 议题 30、31、35、38 确认相关语法 | [`topics/06-constructors-implicit-initialization.md`](./topics/06-constructors-implicit-initialization.md) |
| 07 | 整数溢出与回绕算术 | 已确认 | [`topics/07-integer-overflow-wrapping.md`](./topics/07-integer-overflow-wrapping.md) |
| 08 | 平台相关行为（PDB） | 已确认，议题 61 补充编译期执行 | [`topics/08-platform-dependent-behavior.md`](./topics/08-platform-dependent-behavior.md) |
| 09 | 整数除法与移位 | 已确认 | [`topics/09-integer-division-shifts.md`](./topics/09-integer-division-shifts.md) |
| 10 | 整数转换与 `cast` | 已确认，Parser 议题 06 确认导入别名中的 `as`，其他用途待定 | [`topics/10-integer-conversions-cast.md`](./topics/10-integer-conversions-cast.md) |
| 11 | 位模式转换与指针转换 | 已确认，议题 31、32 补充 | [`topics/11-bitcast-ptrcast.md`](./topics/11-bitcast-ptrcast.md) |
| 12 | 浮点语义与显式 fast-math | 已确认，细分规则待定 | [`topics/12-floating-point-fast-math.md`](./topics/12-floating-point-fast-math.md) |
| 13 | 整数与浮点转换 | 已确认 | [`topics/13-integer-floating-conversions.md`](./topics/13-integer-floating-conversions.md) |
| 14 | 浮点运行环境与 subnormal | 已确认 | [`topics/14-floating-environment-subnormal.md`](./topics/14-floating-environment-subnormal.md) |
| 15 | fast-math 与有限值契约 | 已确认函数声明作用域；用户定义数值类型目标待定 | [`topics/15-fast-math-finite-contract.md`](./topics/15-fast-math-finite-contract.md) |
| 16 | 内建属性与函数装饰器 | 已确认，decorator 是编译期实例化的强类型签名保持包装模板，可跳过或重复调用 continuation；声明不接受 meta-decorator；Parser 议题 15、29、31、33、36、37 补充 | [`topics/16-attributes-function-decorators.md`](./topics/16-attributes-function-decorators.md) |
| 17 | 模块版本、强类型注册记录与热更新 | 已确认，不提供用户模块加载或卸载钩子；议题 18、22、44、45、55、58、61 与 Module 议题 03 补充 | [`topics/17-module-lifecycle-decorator-registration.md`](./topics/17-module-lifecycle-decorator-registration.md) |
| 18 | 模块初始化、注册与清理顺序 | 已确认，不提供用户生命周期钩子排序；Module 议题 01 确认单文件 module，Module 议题 03 禁止实际依赖环 | [`topics/18-module-lifecycle-hook-order.md`](./topics/18-module-lifecycle-hook-order.md) |
| 19 | 编译期反射、动态反射与自定义元数据 | 已确认，议题 20—26、28、31、32、57、61、63、66 与 Parser 议题 15 补充 | [`topics/19-reflection-runtime-metadata.md`](./topics/19-reflection-runtime-metadata.md) |
| 20 | 反射访问权限与封装 | 已确认，议题 61、66 补充 | [`topics/20-reflection-access-control.md`](./topics/20-reflection-access-control.md) |
| 21 | 动态反射值传递与调用 ABI | 已确认，议题 22、23、25、28、57、65 补充，精确二进制布局待定 | [`topics/21-dynamic-reflection-value-abi.md`](./topics/21-dynamic-reflection-value-abi.md) |
| 22 | 基于名称的动态反射身份 | 已确认，议题 23、31 补充；Module 议题 01、02 确认 package/module 路径身份 | [`topics/22-name-based-reflection-identity.md`](./topics/22-name-based-reflection-identity.md) |
| 23 | 反射成员唯一性与继承覆盖 | 已确认，议题 24、25、28—30 补充 | [`topics/23-reflection-member-inheritance.md`](./topics/23-reflection-member-inheritance.md) |
| 24 | `class`、单继承与多接口 | 已确认，支持 `static`、只读接收者、`final`、统一继承类型列表、显式字段、嵌套类型、成员区域、class 类型表达式和 C++ 风格构造初始化列表；议题 25—31、54—58 与 Parser 议题 10、31—33、38、40 补充 | [`topics/24-class-inheritance-interfaces.md`](./topics/24-class-inheritance-interfaces.md) |
| 25 | 虚函数与动态反射调用 | 已确认，函数级 `final` 封闭虚槽；议题 26、28、55、57、58、60、65 与 Parser 议题 31 补充 | [`topics/25-virtual-functions-reflection.md`](./topics/25-virtual-functions-reflection.md) |
| 26 | vptr、vtable 与接口胖引用 ABI | 已确认，议题 24、27—31、55、56 补充 | [`topics/26-vtable-interface-abi.md`](./topics/26-vtable-interface-abi.md) |
| 27 | 自动动态销毁与存储释放分离 | 已确认，拥有型多态容器 API 待定 | [`topics/27-dynamic-destruction.md`](./topics/27-dynamic-destruction.md) |
| 28 | 接口动态反射 | 已确认，议题 29—31、56、57 补充 | [`topics/28-interface-reflection.md`](./topics/28-interface-reflection.md) |
| 29 | 接口默认方法与冲突消解 | 已确认，议题 30、56、65 补充 | [`topics/29-interface-default-methods.md`](./topics/29-interface-default-methods.md) |
| 30 | 接口继承、重新抽象与接口转换 | 已确认，支持泛型接口并复用统一泛型参数语法；议题 31、56、60、66 与 Parser 议题 33 补充 | [`topics/30-interface-inheritance.md`](./topics/30-interface-inheritance.md) |
| 31 | 安全下转型与动态接口转换 | 已确认，议题 32 补充 | [`topics/31-safe-dynamic-cast.md`](./topics/31-safe-dynamic-cast.md) |
| 32 | 枚举、判别联合与通用 niche 优化 | 已确认，统一类型前缀与泛型声明头、无继承子句、枚举分支无尾随逗号；议题 61 与 Parser 议题 07、23、33、34 补充 | [`topics/32-enums-tagged-unions.md`](./topics/32-enums-tagged-unions.md) |
| 43 | 可重复等待的 `Task` 与结果约束 | 已确认，议题 44、45、48—52、54—60、69 补充 | [`topics/43-repeatable-task-results.md`](./topics/43-repeatable-task-results.md) |
| 44 | 惰性异步任务与 `await` 驱动执行 | 已确认，议题 45、48—52、54—59、65 修订；高层调度属于库，底层任务驱动 ABI 待定 | [`topics/44-lazy-async-task-start.md`](./topics/44-lazy-async-task-start.md) |
| 45 | 并发组合等待 `all` | 已确认，议题 48、50、52、62、68、69 与 Parser 议题 10、23 补充；动态任务集合属于库 | [`topics/45-concurrent-await-all.md`](./topics/45-concurrent-await-all.md) |
| 48 | 协作式取消请求 | 已确认，议题 49、50、52 与 Parser 议题 25 补充；底层取消感知 I/O ABI 待定 | [`topics/48-cooperative-cancellation-request.md`](./topics/48-cooperative-cancellation-request.md) |
| 49 | `Task.request_cancel()` 请求接口 | 已确认，议题 50—52、61 补充；批量取消令牌待定 | [`topics/49-task-cancellation-request-api.md`](./topics/49-task-cancellation-request-api.md) |
| 50 | 显式取消请求传播等待 | 已确认，议题 51、52 补充；底层取消感知 I/O ABI 和组合器中间对象 ABI 待定 | [`topics/50-explicit-cancellation-propagation-await.md`](./topics/50-explicit-cancellation-propagation-await.md) |
| 51 | `Task` 析构状态规则 | 已确认，议题 52 补充 | [`topics/51-task-destruction-state-rules.md`](./topics/51-task-destruction-state-rules.md) |
| 52 | 任务调度与后台执行属于库 | 已确认，底层任务驱动 ABI 待定 | [`topics/52-library-task-scheduling.md`](./topics/52-library-task-scheduling.md) |
| 54 | 异步成员函数的 `this` 是非拥有原始指针 | 已确认，议题 55—57 补充异步虚函数、接口与反射；Parser 议题 31 确认声明与只读接收者语法 | [`topics/54-async-member-this-pointer.md`](./topics/54-async-member-this-pointer.md) |
| 55 | 异步虚函数在任务创建时完成动态分派 | 已确认，议题 56—58、60 补充异步接口、反射、装饰器与结果不变型；Parser 议题 29 补充异步函数类型 | [`topics/55-async-virtual-dispatch-at-task-creation.md`](./topics/55-async-virtual-dispatch-at-task-creation.md) |
| 56 | 异步接口方法在任务创建时完成接口分派 | 已确认，议题 57、58、60 补充异步反射、装饰器与结果不变型 | [`topics/56-async-interface-dispatch-at-task-creation.md`](./topics/56-async-interface-dispatch-at-task-creation.md) |
| 57 | 异步动态反射调用返回惰性任务 | 已确认，议题 58、60、61 补充；拥有型动态任务 API 待定 | [`topics/57-async-dynamic-reflection-call.md`](./topics/57-async-dynamic-reflection-call.md) |
| 58 | 异步函数装饰器包围任务执行体 | 已确认，异步 continuation 可零次或多次进入且不生成模块生命周期代码；议题 24、59 补充；Parser 议题 36、37 同步装饰器声明与全参数转发语法 | [`topics/58-async-decorators-wrap-task-body.md`](./topics/58-async-decorators-wrap-task-body.md) |
| 59 | Ink v0 不提供任务构造装饰器 | 已确认，创建期自定义行为使用显式同步任务工厂 | [`topics/59-no-task-construction-decorators.md`](./topics/59-no-task-construction-decorators.md) |
| 60 | 异步覆盖不支持协变结果 | 已确认，`Task::<T>` 对结果类型不变型 | [`topics/60-no-covariant-async-results.md`](./topics/60-no-covariant-async-results.md) |
| 61 | `comptime` 泛型、部分求值与编译期执行 | 已确认，议题 62—72 补充泛型、静态声明展开、重载、元组元值与统一函数阶段；Parser 议题 31、32 确认泛型函数声明与统一区域控制 | [`topics/61-comptime-generics-partial-evaluation.md`](./topics/61-comptime-generics-partial-evaluation.md) |
| 62 | 编译期参数包是普通编译期序列 | 已确认，议题 64、66、68—71 补充绑定、反射、运行时包、元组与重载；Parser 议题 29、31 统一展开与包声明 | [`topics/62-comptime-parameter-packs.md`](./topics/62-comptime-parameter-packs.md) |
| 63 | Ink v0 不提供字符串到代码生成 | 已确认，议题 67 确认只选择或重复静态源码声明且不提供 Identifier splice；Parser 议题 40 定义 class 类型表达式；允许静态候选集条件导入 | [`topics/63-no-string-to-code-generation.md`](./topics/63-no-string-to-code-generation.md) |
| 64 | Ink v0 不支持泛型实参推导 | 已确认，议题 65、70 补充默认值与重载候选规则 | [`topics/64-no-generic-argument-inference.md`](./topics/64-no-generic-argument-inference.md) |
| 65 | 普通参数与编译期参数统一使用默认实参 | 已确认，议题 70 补充默认实参与重载候选关系；Parser 议题 13、15、29、31 确认求值、调用、函数类型与默认形参语法 | [`topics/65-unified-default-arguments.md`](./topics/65-unified-default-arguments.md) |
| 66 | 开放泛型声明是一等编译期值 | 已确认，议题 67、70、71 补充静态声明展开、泛型重载与元值元组；Parser 议题 40 定义 class 类型表达式 | [`topics/66-first-class-generic-declarations.md`](./topics/66-first-class-generic-declarations.md) |
| 67 | `comptime` 只选择和展开源码声明，不公开 Builder | 已确认，v0 不提供 `field(...)`、动态声明名称或其他声明构造器；声明区通过 Parser 议题 32 的 `RegionKind` 统一支持 block、if、for、while | [`topics/67-comptime-structured-declarations-no-builders.md`](./topics/67-comptime-structured-declarations-no-builders.md) |
| 68 | 运行时参数包与编译期类型包一一对应 | 已确认，议题 69、70 补充元组与闭合重载；Parser 议题 29、31 确认列表展开与包形参 | [`topics/68-runtime-parameter-packs.md`](./topics/68-runtime-parameter-packs.md) |
| 69 | 元组是内建匿名结构值类型 | 已确认，议题 71 补充异构编译期元值与静态遍历；Parser 议题 07、10、14、23、26、29、30 确认相关语法 | [`topics/69-tuples-structural-values.md`](./topics/69-tuples-structural-values.md) |
| 70 | 显式泛型实参与重载解析 | 已确认，先闭合候选签名再执行普通重载，不提供 SFINAE 回退 | [`topics/70-explicit-generic-overload-resolution.md`](./topics/70-explicit-generic-overload-resolution.md) |
| 71 | 现有元组直接承载异构编译期值 | 已确认，不增加元值容器或类型擦除，由 `comptime for` 逐项展开；Parser 议题 30 确认类型值元组的默认解释 | [`topics/71-comptime-heterogeneous-tuples.md`](./topics/71-comptime-heterogeneous-tuples.md) |
| 72 | 不设置 `comptime func` 函数类别 | 已确认，`comptime` 只直接修饰表达式、块与结构化控制流；Parser 议题 31 不提供该函数修饰符，议题 32 统一全部区域 | [`topics/72-no-comptime-function-category.md`](./topics/72-no-comptime-function-category.md) |

“讨论中”的议题不会提前写入设计文件。后续议题在开始讨论时追加到本表，在确认后创建对应文件。
