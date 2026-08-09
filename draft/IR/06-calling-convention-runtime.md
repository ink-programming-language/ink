# InkIR 调用约定、运行时边界与 TargetABI

> 状态：已确认设计
>
> 确认日期：2026-08-08
>
> 本文规定 InkIR 的逻辑调用约定、异常和任务控制边、动态分派、反射调用、热更新入口及 C ABI 边界。具体寄存器分配、结构体字节布局、平台异常头和运行时内部算法属于私有 TargetABI，不构成稳定的 Ink-to-Ink 二进制 ABI。

## 1. 范围与分层

InkIR 必须同时服务于 RuntimeWorld 参考解释器和 LLVM AOT 后端。两者共享本文规定的逻辑语义，不共享宿主平台偶然形成的 C++ 调用约定或对象布局。

本文“规范操作”指中央注册的 opcode/schema 及其语义集合；本章代码块统一是 noncanonical schema/CFG pseudocode，允许 `!tName`、`N`、`DIGEST`、说明性 payload、省略共同 envelope及为展示 successor 而换行，不可直接交给 parser。逐 opcode 的准确单行格式见 [`11-instruction-reference.md`](./11-instruction-reference.md)，最终 canonical renderer 与 envelope 见 10 §15.5 和 [`08-text-binary-format.md`](./08-text-binary-format.md)。

本文区分两层 ABI：

1. 规范性语义 ABI；
2. 私有 TargetABI。

规范性语义 ABI 决定：

- 函数的逻辑参数、接收者、结果和函数种类；
- 哪些值以标量 SSA 传递，哪些值必须在调用者提供的最终存储中构造；
- 正常返回、异常展开、任务失败、可报告trap和fatal termination的不同控制语义；
- virtual、interface、reflection 和 async 调用的动态选择时机；
- constructor、destructor、dynamic destroy、ExceptionBox 和 Task 的所有权边界；
- hot reload 的稳定入口、版本固定和同 ABI 更新规则；
- verifier 必须拒绝的非法 IR。

私有 TargetABI 决定：

- 标量使用哪些寄存器或栈位置；
- 隐藏结果地址位于第几个物理参数；
- vptr 的精确偏移、vtable 表头和 interface table 的字节布局；
- Task、ExceptionRecord、ExceptionBox、DynamicRef 和反射句柄的物理表示；
- Itanium EH、Windows funclet 或其他目标异常机制的具体 lowering；
- 版本固定使用引用计数、epoch、hazard pointer 还是等价机制。

私有 TargetABI 必须带有编译器和目标版本标识。它可以在编译器版本之间改变，不得被源码、持久化数据或第三方二进制当作稳定公共 ABI。

Ink 源码模块以源码分发。本文不建立稳定的 Ink-to-Ink 二进制模块 ABI。唯一面向独立外部二进制的稳定边界是显式声明并满足约束的外部 ABI，例如 C ABI。

本文出现的规范 operation 必须使用稳定命名空间。调用使用 call.*，对象生命周期使用 obj.*，异常使用 eh.*，动态转换使用 cast.*，反射使用 reflect.*，异步运行时使用 async.*，装饰器结构使用 decorator.*，热更新运行时使用 rt.*，外部 ABI bridge 使用 abi.*。无命名空间的 call、invoke、destroy、await 等拼写只用于自然语言，不是合法 opcode。

## 2. 术语

本文使用以下术语：

- 逻辑签名：源码语义分析完成后的函数种类、接收者、参数传递方式、逻辑结果类型和结果传递模式；
- 物理签名：TargetABI lowering 后实际交给 LLVM 或本机调用点的参数与结果表示；
- 标量 SSA：可以作为单个 InkIR SSA 值产生、传递和返回的标量；
- address-only 值：必须存在于具有类型、大小、对齐和初始化状态的存储中，不能作为普通 SSA 聚合结果存在；
- caller destination：调用者为 address-only 结果提供的最终存储及其唯一初始化事务；fresh storage 初始为 AllocatedUninitialized，转发时携带已经 active 的 ResultDestination；
- stable entry：热更新模式中代表逻辑函数身份、在每次新调用时选择当前实现版本的稳定入口；
- version-local entry：只属于某个模块版本、不得作为跨版本逻辑身份逃逸的实现入口；
- pin：阻止某个模块版本的代码、只读表、描述符或销毁入口在使用期间被卸载的内部所有权；
- may-unwind：操作可以产生 Ink 异常展开边；
- nothrow：异常不得越过该同步入口或异步执行体的公开契约。

## 3. 值的调用 ABI 分类

### 3.1 单标量 SSA 与 unit 特例

以下逻辑类型默认属于单标量 SSA：

- bool；
- 固定位宽整数和 ptrsize；
- 浮点数；
- 裸指针；
- 普通引用；
- 函数入口或函数值的非拥有句柄；
- TargetABI 明确声明为单标量表示的无载荷 enum。

源码 `()` 规范化为 Core builtin `unit`；`unit` 是唯一的零载荷 SSA 特例。它具有唯一语义值，可以作为 function result、block argument 和 SSA use 出现，但不属于单标量 ABI 分类；TargetABI 必须消除其物理参数与结果载荷。canonical text 中 `()` 只表示零 channel，不是类型名。`void` 不产生 SSA 结果，`never` 不具有正常返回值，二者都不得与 `unit` 合并。

标量 SSA 分类是 InkIR 语义分类，不授权后端使用宿主 C++ 类型代替目标位宽。RuntimeWorld 和 AOT 都必须依据 TargetContext 解释位宽、字节序、地址空间和浮点语义。

### 3.2 address-only 值

以下逻辑类型默认属于 address-only：

- class；
- 带载荷 enum；
- 非空 tuple；
- array；
- slice 和其他多字视图；
- interface 引用；
- Task 和其他由中央 registry 声明为 `runtime.object` 的 sealed runtime storage；
- 任何包含 address-only 子对象的聚合；
- 任何因 noncopyable、析构、部分初始化或目标布局要求而不能作为单标量 SSA 表示的类型。

即使 TargetABI 能够把一个小型可复制聚合拆进若干寄存器，规范性语义 ABI 仍把它视为 address-only。TargetABI 只有在证明不产生额外的语言级复制、移动、析构或地址变化时，才可以在私有 lowering 中进行寄存器拆分和重新物化。

Task 始终是 address-only 和 noncopyable。Task 的物理大小不能反向决定其语义分类。v0 `Task<T>` 只允许 `T = void`、`T = never` 或闭合 runtime-representable、`Copyable(T)` 且 `CopyConstructionEnabled(T)` 的结果类型，并拒绝 `ContainsNoEscapeValue(T)`；不可复制资源或禁止copy-construction的结果必须改用显式拥有指针/handle，不能形成永远无法合法await的Task后再依赖隐藏移动。

Task::<T>是源码/叙述层名称；canonical type spelling为!runtime-object<task,T>。runtime.object是semantic SizedObjectType，而不是runtime.opaque：

~~~text
runtime.object<kind, args...> {
    AddressOnly = true
    Copyable = false
    StableAddress = true
    LayoutComplete = true
    SealedRuntimeStorage = true
    Size, Alignment, RuntimeStorageAbiHash
}
~~~

Size、Alignment和RuntimeStorageAbiHash只由TargetContext与RuntimeAbiRevision查询，因此!runtime-object<task,T>可以形成!place<access,!runtime-object<task,T>>、进入caller destination并参与明确lifetime；其fields、padding和bytes仍是private，类型性质固定含SealedRuntimeStorage。canonical IR禁止place.addr/deref、place.field/base/tuple projection、普通typed/raw load/store、constant、obj.init.copy/obj.assign.copy、generic destroy、任意用户Copy以及与其active/partial storage重叠的byte-count memory operation作用于runtime.object；只有中央registry中声明支持对应kind的runtime schema可读取、改变或销毁它。runtime.opaque仍是不满足SizedObjectType、不能形成place/sizeof的私有pointee；!runtime-handle仍是不可存储的verifier capability，三者不得互换。interface reference、Optional<ref>与Optional<InterfaceRef>仍是普通可复制address-only类型，不是runtime.object。

reflection snapshot是线性`!runtime-handle`；ExceptionBox属于内部`RuntimeHandleKind=exception_box`并且只能由异常/Task runtime持有。二者都不是可形成普通typed place的address-only用户值，也不是`runtime.object`。revision 1没有把`!runtime-handle<exception_box>`直接物化为用户SSA值的producer opcode，也没有用户可见load/store/retain/release；它只通过active `!exception` token、failed Task sealed state及匹配runtime schema间接拥有。

v0 不提供通用 heap、arena 或 tracing GC 对象模型。Closed IR 必须拒绝未注册的 heap/arena/gc storage kind、通用 allocate/deallocate、collector root、write barrier、safepoint 和 tracing descriptor；`Allocate(task_frame/exception/exception_box)` 等 effect 只描述对应专用 runtime schema，不能据此构造任意用户对象。以后引入上述模型必须同时注册 storage owner token、失败语义、lifetime、root/barrier、effect 和 TargetABI lowering，不能只增加一个 allocation opcode。

### 3.3 address-only 参数

按值传递 address-only 参数时，调用边界必须建立独立的参数对象生命周期：

- 从已有命名对象传参时，只有类型可复制才允许显式 copy initialization；
- 从没有独立源对象的构造表达式传参时，可以直接在参数最终存储中构造；
- noncopyable 命名对象不能通过调用约定获得隐藏移动；
- callee 取得参数 place，并在退出时按普通规则清理该参数对象；
- 引用参数和裸指针参数只传递非拥有地址，不建立参数副本。

TargetABI 可以把可复制 address-only 参数拆分为寄存器，但必须保持上述独立参数对象语义。

`object T` 参数采用以下闭合 handoff 协议；FunctionSignature 中仍保存逻辑参数 `T` 与 passing mode `object`，Core entry 中对应的 `parameter` argument 则固定为当前 Alive 独立参数对象的 owner `!place<rw,T>`，不能编码为普通 `T` SSA：

~~~text
AllocatedUninitialized
    -> Preparing(active transaction, caller owns cleanup)
    -> PreparedAlive(caller owns cleanup)
    -> Transferred(callee activation owns cleanup)
    -> Destroyed
~~~

caller 按源码顺序直接构造或 copy-initialize每个参数对象。在全部实参求值、callee/adapter选择、必要pin取得及调用activation所需资源准备完成之前，所有 `PreparedAlive` obligation仍属于caller；较后实参、dispatch、adapter验证或activation建立失败时，caller必须按已完成参数的逆源码顺序销毁它们。call activation 的建立与全部prepared parameter owner向callee的转移是一个不可部分发生的语义事件：转移后caller不得再读、销毁或重试使用该owner，callee在normal和unwind退出上都恰好清理一次。async construction成功时，owner先转移给具体Task frame/capture；同步construction在转移后失败时由construction cleanup负责，转移前失败仍由caller负责。任何CFG join都不得丢失、复制或同时保留caller/callee两份obligation。

canonical call operand 对 `object T` 编码准确的prepared parameter owner place及passing mode，而不是原始源对象；源对象到独立参数对象的copy/direct-construction必须在call之前显式可验证。TargetABI只有在证明寄存器拆分与重新物化保持同一唯一handoff、相同析构次数和相同失败清理顺序时，才可擦除物理parameter object。

### 3.4 address-only 结果

address-only 结果使用 caller destination：

~~~text
fresh storage owner
    → 提供满足 T 大小和对齐的 AllocatedUninitialized 存储
    → 在首次构造前恰好执行一次 obj.init.begin
    → 建立唯一 active init transaction
intermediate forwarding callee
    → 接收同一 ResultDestination transaction
    → 可以把它原样继续传给内层 to %destination
    → 不重复 begin 或 commit，只传播内层终态证明
final completing/failing callee
    → 在最终存储中完成构造
    → 在 normal return 前恰好执行一次 obj.init.commit
    → outward unwind 前清理已经完成的子对象但不 commit
    → outward-unwind schema postcondition 将 transaction 回滚到 AllocatedUninitialized
~~~

callee 不得先构造一个隐藏 T 临时量，再通过隐式移动或逐字节复制把它送入 caller destination。

该事务协议适用于所有带 to %destination 的 address-only 结果，而不只适用于 constructor：普通同步函数、virtual/interface call、reflection adapter、Task construction 和 async.await_copy 都遵守同一 ownership 模型。fresh storage owner 在 callee 与全部实参求值完成后、首次构造调用前执行 obj.init.begin；从当前 activation 继承的 ResultDestination 已经处于 active transaction，中间转发层不得再次 begin。普通非sealed结果由最终完成者显式执行`obj.init.commit`；Task是sealed runtime storage，成功构造只由`async.call`/`async.invoke`或匹配reflection async adapter的schema原子执行Task-specific commit postcondition，canonical IR不得对Task发出generic `obj.init.commit`。

普通 call/reflect operation 本身不合成 begin、commit 或 rollback。address-only callee activation 接收一个携带 transaction identity、不可存储且不可逃逸的 ResultDestination 能力，而不是普通 pointer 参数。最终完成普通非sealed结果构造的 leaf/callee 在函数体内执行唯一generic commit；最终失败的 leaf/callee 先按 cleanup plan 销毁已完成子对象，其 outward-unwind schema postcondition 再把 transaction 回滚到 AllocatedUninitialized。中间转发 callee 在内层 normal edge 已证明 destination committed 后直接 cf.return，不再 commit；在内层 unwind edge 已证明 destination 已回滚后只传播 exception，不重复状态转换。Task construction与async.await_copy是两个schema特例：前者在async operation的normal postcondition原子执行Task-specific commit，后者由12.5节原子执行typed Copy后的normal commit或failure rollback。上述rollback均是schema后置条件，不存在名为obj.init.abort的InkIR opcode。

callee 的 cf.return 不携带 T，并且只有 destination 已提交时合法。携带 active ResultDestination 的 activation 执行 outward eh.resume 时，必须已经完成部分对象 cleanup，并满足 transaction 回到 AllocatedUninitialized 的 schema postcondition。

caller destination 必须：

- 在 fresh storage 上由唯一 obj.init.begin 从 AllocatedUninitialized 转为 active transaction，或者作为已经 active 的 entry ResultDestination 传入；
- 在每个 to %destination 操作开始时处于未提交的 active transaction，而不是 Alive；
- 满足目标类型的大小、对齐和地址空间要求；
- 在整个调用及其异常边期间保持有效；
- 不与 callee 在构造过程中仍需读取的源对象非法重叠；
- 普通非sealed结果只由最终完成者的obj.init.commit转换为Alive；Task只由async schema的Task-specific commit postcondition转换为Alive；
- 在最终失败 callee 的 outward unwind postcondition 或 async.await_copy failure postcondition 上回到 AllocatedUninitialized，因此不得运行 T 的完整析构。

构造过程中已经成功初始化的内部子对象由 callee 自己在 unwind 前清理。caller 只观察“完整结果已构造”或“完整结果未构造”。

destination operation 的 normal edge只产生“同一 storage 已Alive”的数据流状态证明，不产生新的 schema edge value或新的place SSA。caller必须在该normal-dominated路径显式执行 place.as_alive %destination : !place<init,T> -> !place<ro/rw,T>，才能读取、借用、销毁或继续传递结果。unwind/rollback路径保留原!place<init,T> capability，但transaction identity已被消费；在部分子对象cleanup完成且storage确为AllocatedUninitialized后，它可以由新的obj.init.begin建立fresh transaction，不需要place.as_uninitialized。place.as_uninitialized只用于obj.destroy后的旧ro/rw capability在严格证明下重新绑定为init。两种capability conversion都是verifier proof operation，不开始、提交、回滚或结束生命周期。

每个place及其投影携带verifier-only PlaceCapabilityGeneration。place.as_alive和place.as_uninitialized都是CapabilityRebind：它们消费当前root generation并产生next generation；旧init/ro/rw capability及由其派生的field/base/element/borrow view在rebind后全部非法，ancestor root rebind也使构造期descendant views失效。CapabilityRebind没有runtime memory effect，但具有线性、不可复制、不可CSE、不可推测且不可越过其状态证明移动的schema trait；printer/binary不必编码机器字段，但verifier必须可重建generation def-use。

## 4. 函数逻辑签名

每个可调用声明至少具有以下逻辑属性：

~~~text
FunctionSignature {
    callable_kind
    function_kind
    receiver_kind
    ordered_parameters
    parameter_passing_modes
    logical_result_type
    result_passing_mode
    calling_convention
}
~~~

v0 callable_kind 闭集为：

~~~text
function
constructor
destructor_body
runtime_thunk   // reserved；revision 1 Function record拒绝
~~~

constructor 与 destructor_body 都是同步 entry kind。它们不因为共享 sync 控制边而获得普通函数的取地址、反射或直接调用能力。`runtime_thunk`只保留numeric tag，revision 1不具有合法Function record；runtime行为必须使用中央专用opcode或declarative RuntimeDeclaration，不能补造一个可调用thunk。

`destructor_body` 的闭合签名矩阵固定为 `function_kind = sync`、`receiver_kind = mutable_instance`、零个 ordered parameter、`logical_result_type = void`、`result_passing_mode = void`、`calling_convention = ink` 和稳定 nothrow contract。它只由准确类型的 compiler-generated destroy plan进入：静态路径属于 `obj.destroy`，动态路径属于 `obj.destroy_dynamic` 选中的 version-local destroy thunk；`call.direct`、`call.indirect`、`call.invoke`、函数值、virtual/interface普通slot、decorator、reflection 和 extern export 都不得直接引用 destructor_body。body 内部可以调用普通 helper，但所有可能失败的内部边必须在析构边界内处理或终止于 `rt.fatal`，不得形成 outward unwind；body normal completion只返回 destroy plan，不自行释放storage或结束完整对象generation。

v0 function_kind 闭集为：

~~~text
sync
async
~~~

同步函数显式返回 Task::<T> 仍是 sync 函数。async func 的 logical_result_type 是 T，调用表达式建立 Task::<T>。两者不能互相赋值、覆盖或替代。

v0 receiver_kind 闭集为：

~~~text
none
mutable_instance
const_instance
static_member
interface_receiver
initializing_instance
~~~

initializing_instance 只用于 constructor。它携带尚未提交为完整对象的最终 destination 能力，不是可存储的普通 this 引用。

v0 parameter_passing_modes 闭集为：

~~~text
value
object
const_reference
mutable_reference
raw_pointer
~~~

对 `CallingConvention=ink`，value 只用于非引用单标量 SSA 或 unit 参数；object 表示语义上按值、物理上由独立参数对象承载的 address-only 参数，并遵守 3.3 节的 copy/direct-construction 与 callee cleanup 规则。`CallingConvention=c` 的唯一 CFunctionType 例外见 14.6，已验证 repr(C) aggregate 也使用 logical `value`。const_reference、mutable_reference 和 raw_pointer 都以对应 typed handle 传递，不建立参数对象副本；ordered parameter type 仍保存精确 pointee、ro/rw 与地址空间信息。

result_passing_mode 必须显式区分：

~~~text
value
result_destination
void
~~~

对 `CallingConvention=ink`，value 表示 logical_result_type 作为一个标量 SSA 或专用 unit 值产生；`CallingConvention=c` 的唯一 CFunctionType 例外见 14.6，已验证 repr(C) aggregate 的逻辑结果同样是 value，即使物理 TargetABI 使用 sret。result_destination 表示 Ink 调用者提供携带唯一 active initialization transaction 的最终结果位置，callee 不产生 T 的 SSA result；void 表示既不产生 SSA result，也不接收结果位置。本文的 result_destination 是类型和值规范中 result-place 结果模式在 FunctionSignature、canonical text 和 binary record 中的规范拼写；object 仍描述语义上的按值对象分类，不形成第四种闭合结果传递方式。

对 `CallingConvention=ink`，logical_result_type 与 result_passing_mode 必须相容：result_destination 只配 address-only 非 void 结果，value 只配单标量 SSA 或 unit，void mode 只配不产生 normal result 的 void 或 never。CFunctionType 改用 14.6 的闭合矩阵，不受上述 Ink-only scalar 限制。never 与 void 共享“无结果载荷”传递模式，但 logical_result_type 仍分别保存 never 和 void；never 表示无可达 normal completion，不得因 transport mode 相同而获得 void 的 normal return。may-unwind never invoke 仍按 terminator schema 保留一个不接收 result 的 normal successor，该 successor 只能进入 cf.unreachable 或 verifier 已证明不可达的终止路径。

对于 sync function，result_passing_mode 直接决定 call.* 的结果通道。对于 async function，它描述 logical_result_type T 在 Task completion 与 await 边上的结果模式；async.call/async.invoke 的即时调用结果固定是在 destination 中构造的 Task::<T>，不能据此把 T 的模式改写成 result_destination。constructor 和 destructor_body 的结果模式是 void；constructor 的 to %destination 承载 initializing_instance receiver，不是普通逻辑结果位置。

result_passing_mode 是函数类型身份、override 匹配、间接函数类型和 SymbolKey 规范签名的一部分。canonical text 的函数签名记录必须显式打印 result_passing_mode；binary FunctionRecord 必须以稳定 enum tag 编码它。parser/decoder 必须只依据签名记录重建该模式，即使函数只有 declaration 而没有 body；不得通过扫描 call opcode、是否出现 to、函数体 return、logical_result_type 的布局或 TargetABI 隐藏参数反推。printer/encoder 的 round-trip 必须保持该字段逐值不变。

v0 canonical `CallingConventionTag` 闭集为 `ink|c|runtime_intrinsic|platform_eh`。ink 使用本文的规范性语义 ABI；c 只允许出现在AbiImport/AbiBridge/AbiExport引用的Type(function) `CFunctionType`逻辑签名，任何Function record都不能使用c；runtime_intrinsic与platform_eh只允许由已注册runtime opcode schema/TargetABI thunk声明，不能用于普通源码函数值或用户declaration。

默认实参不进入 `Type(function)` 或 `FunctionAbiDigest`，但 Function declaration 依 10 中央 schema 保存与 Parameters 等长的 required `DefaultArguments` vector，并把其语义投影纳入 public interface digest。前端/caller 必须在生成任何 decorator、stable entry、virtual slot、interface slot、reflection adapter 或 `call.*` operation 前完成默认实参求值和补全，call payload 始终携带完整实参数。

Closed InkIR 中每个调用点必须具有已经闭合的逻辑签名、calling convention 和 TargetABI tag。不得在 ABI lowering 阶段重新进行重载解析、默认实参补全、泛型实例化或隐式用户构造。

### 4.1 nothrow 不属于函数类型

nothrow 是声明行为契约，不进入普通函数值类型、重载身份或名称修饰。

对于同步函数：

- nothrow 入口不得向调用者产生 unwind edge；
- TargetABI 可以把该入口 lowering 为 LLVM nounwind 或等价保证；
- 覆盖、interface 实现、decorator 展开和热更新版本不得削弱该保证。

对于 async 函数，Ink 采用双通道失败模型：

1. 同步任务构造通道；
2. 异步任务执行通道。

源码中 async 函数的 nothrow 约束异步执行通道：完整装饰后任务执行体不得把异常发布为 failed(ExceptionBox)。它不自动保证同步任务构造入口 nounwind，因为实参捕获、frame 存储取得和版本固定仍可能在 Task 成功构造前同步失败。

因此当前函数发布记录的`BehaviorContracts`必须使用正向、单调加强的bit，不保存或digest反相的current may-unwind/may-fail bool。适用矩阵固定为：

~~~text
sync callable:  nothrow
async callable: construction_nothrow, body_no_fail
stable entry:   hot_reload_stable
extern entry:   extern_no_unwind (sync only, implies nothrow)
~~~

sync callable的construction_nothrow/body_no_fail必须为零；async callable的nothrow必须为零。hot_reload_stable当且仅当EntryIdentityTag=stable；extern_no_unwind只允许EntryIdentityTag=extern，设置时必须同时满足sync nothrow；所有其他不适用bit必须为零。源码`[nothrow] async`只设置body_no_fail，表示Task body不发布failed；它不自动设置construction_nothrow，后者必须由独立公开契约证明。普通async declaration的两个bit都可为false。

Task::<T>类型本身不记录body_no_fail。通过普通Task值执行await时仍按可能失败处理；只有局部、版本固定且由TaskBodyEffect/来源contract证明的优化才能消除对应失败边。

### 4.2 Core entry envelope

FunctionSignature 与 Core CFG entry arguments 是两层记录。前者保持源码和逻辑调用身份，后者显式承载调用/runtime 隐藏通道；body 存在时，entry argument role/type/order 必须可从 Function record 唯一导出：

1. 可选 `receiver`；
2. 逐个 `parameter`；
3. GlobalRecord 指定 initializer/finalizer 的 `global_lifecycle`；
4. sync `result_passing_mode = result_destination` 的 `result_destination`；
5. async body/generated resume 的 `task_self`；
6. async address-only logical result 的 `task_result_storage`。

`global_lifecycle` 只由 module runtime提供给准确 GlobalRecord 唯一指定的 initializer/finalizer，分别是匹配 global 的 owner init place和owner Alive place；普通 `mem.global_place` 与 call edge不产生它。sync `result_destination` 是 owner-authority init place，已经携带 caller 的 active transaction。constructor 使用 initializing `receiver` 表示同一类 caller destination，逻辑结果为 void，因此没有第二个 result argument。async `task_self` 是 runtime 从当前 Alive Task 提供的 borrowed rw place，只能执行 task-body schema 所允许的 state/publish operation；它不能销毁 Task。async `task_result_storage` 是 runtime 从 sealed Task frame 提供的 owner-authority init place，但 entry 状态只是 `AllocatedUninitialized`，尚未 begin；body 在真正构造 address-only return result 前建立 fresh transaction。

`global_lifecycle`、`task_self` 与 `task_result_storage` 都不是普通用户参数，也不能由 branch伪造。Task 隐藏通道不能经普通 sealed-storage projection产生、存储、返回、捕获到用户对象或跨 Task 逃逸；它们可以按 coroutine frame schema跨暂停 spill，前者的 borrow lifetime由当前 Task drive 覆盖，后者只能建立/清理/发布该 Task 的逻辑结果，不能释放 frame storage。非 entry CFG block argument 使用 `phi` role，nested region entry 使用 `region_argument`。

initializer/finalizer lifecycle entry 固定为private `EntryIdentity=version_local` helper；其generated identity绑定当前`versioned_entity_owner(Global.Symbol, ModuleVersionContextKey)`，SemanticOutputOrdinal分别固定为0/1且由该Global字段反向唯一引用，runtime activation/finalization同时持有匹配的module-version owner。helper的Core地址、owner side state与cleanup plan必须从同一version context取得，因此V1的初始化、失败回滚或retire cleanup即使与V2 publish并发，也只能进入V1 helper，绝不能重新选择current版本或跳到V2 body。

## 5. 同步直接调用、间接调用与 invoke

### 5.1 call 与 invoke 的边界

InkIR 使用显式异常控制流：

- call.direct、call.indirect、call.virtual 和 call.interface 只能用于 verifier 已证明不会产生 Ink unwind edge 的调用；
- 所有可能产生 Ink unwind edge 的同步调用统一使用 call.invoke terminator；
- call.invoke 的 callee_kind 属性必须是 direct、indirect、virtual、interface 或由 opcode schema 明确允许的 runtime adapter kind；
- trap、abort 和 fatal 不属于 unwind，不得使用 catch 接收。

这一规则比某些后端 IR 更严格。即使某个 may-unwind 调用当前没有本地 catch 或 cleanup，Closed InkIR 仍必须使用 call.invoke，并把 unmatched unwind 显式交给 eh.resume。这样 RuntimeWorld 和所有 AOT 后端观察同一套清理图。

所有 call.* 操作共用一个结果约定：

- void 结果不产生 SSA result，也没有 destination；
- unit 结果产生唯一 unit SSA semantic result，但没有物理 payload 或 destination；
- 标量结果产生一个 SSA result；
- address-only 结果不产生 SSA result，并使用 to %destination 指定 caller destination；
- callable_kind = constructor 没有普通逻辑结果，但同样使用 to %destination 传递唯一 initializing_instance receiver；
- call.invoke 的标量结果只作为 normal successor argument 产生；
- 每个 to %destination 都要求已有唯一 active init transaction；fresh owner 显式 begin，转发层复用该 transaction；
- call.invoke 的 address-only normal edge 证明 transaction 已由最终完成 callee committed，unwind edge 证明最终失败 callee 的 outward-unwind postcondition 已将其回滚；invoke 本身不再合成 commit 或 rollback。

address-only normal successor不接收result(N)或place edge argument；它只携带状态证明，caller在successor中用place.as_alive物化可访问place。unwind successor只接收exception；已回滚的原!place<init,T>不再关联旧TransactionId，只有在rollback/cleanup证明支配后才能直接执行新的obj.init.begin。

call.invoke 是 terminator，必须恰有一个 normal successor 和一个 unwind successor。canonical text 使用 result(N) 和 exception 表示 schema-produced edge value；它们不在前驱 block 获得 SSA ValueId。normal successor 的首个块参数在存在标量或 unit 结果时接收 result(0)，unwind successor 的首个块参数接收 exception 并具有 !exception 类型。其余 successor arguments 必须在调用前完成求值，并按普通 block-argument 规则显式列出。

所有带 to %destination 的 call/async/reflect schema 都必须显式编码 destination role，不能仅由 callee 名称或 constructed type 猜测：

~~~text
DestinationPayload {
    DestinationValueId
    ConstructedTypeId
    DestinationRoleTag = result | initializing_receiver
}
~~~

普通 address-only 结果和 async 调用构造的 Task 使用 Result；constructor 使用 InitializingReceiver。constructor 的 FunctionSignature 始终是 logical_result_type = void、result_passing_mode = void；call 文本 type signature 中为 constructor 写出的 -> T 只表示 constructed channel type，用来验证 destination 的类型和布局，不是 constructor 的逻辑结果。canonical text 必须显式打印 role，binary opcode payload 必须逐字段编码 DestinationValueId、ConstructedTypeId 和 role。没有 destination 的调用不编码 DestinationPayload。verifier 必须拒绝把 constructor destination 当作 Result，或把普通 result_destination 当作 InitializingReceiver。

### 5.2 标量直接调用

规范操作：

~~~text
%result = call.direct @function(%arguments...)

call.invoke callee_kind = direct @function(%arguments...)
    normal ^normal(result(0))
    unwind ^unwind(exception)
~~~

void 调用不产生 normal result。标量 call.invoke 的结果只在 normal successor 中可用。

### 5.3 address-only 直接调用

规范操作：

~~~text
call.direct @function(%arguments...) to %destination {destination_role = result}

call.invoke callee_kind = direct @function(%arguments...) to %destination {destination_role = result}
    normal ^normal
    unwind ^unwind(exception)
~~~

这两种形式都不产生 address-only SSA 结果。to 是结果通道语法，不把 destination 加入源码可见 ordered_parameters。调用前 destination 必须已经处于 active init transaction；normal edge 证明它已 committed，unwind edge 证明 callee outward-unwind postcondition 已把它回滚到 AllocatedUninitialized。

### 5.4 间接调用

规范操作：

~~~text
%result = call.indirect %callee(%arguments...)

call.invoke callee_kind = indirect %callee(%arguments...)
    normal ^normal(result(0))
    unwind ^unwind(exception)

call.indirect %callee(%arguments...) to %destination {destination_role = result}

call.invoke callee_kind = indirect %callee(%arguments...) to %destination {destination_role = result}
    normal ^normal
    unwind ^unwind(exception)
~~~

普通函数值类型不携带 nothrow 契约，因此普通源码产生的间接调用必须使用 callee_kind = indirect 的 call.invoke。

call.indirect 只允许用于 Closed InkIR 已经持有独立不展开证明的内部情况，例如：

- callee 是编译器生成且经过 verifier 的 nothrow thunk；
- 去虚拟化后 callee 已经重新绑定为直接 nothrow 实现；
- TargetABI 提供一个自身 nothrow、内部把违约转换为 fatal 的边界 thunk。

不得仅因为当前实现体没有 throw 就把跨模块或可热更新的普通函数值当作 nothrow。

### 5.5 调用目标种类

直接调用目标必须标明以下身份之一：

~~~text
stable
version_local
continuation_local
extern
~~~

stable 目标在热更新模式中执行当前版本选择和 pin 获取。version_local 目标只能在已经固定兼容版本的作用域中使用。continuation_local 目标只能由 decorator continuation lowering 使用且不能逃逸。extern 表示在编译/链接期解析、但仍使用 Ink logical ABI 的外部 Ink declaration。

call.direct、call.indirect、call.virtual、call.interface 和 call.invoke 只允许 calling_convention = ink。`CallingConvention=c`的CFunctionType只由AbiImport/AbiBridge/AbiExport引用，不属于`EntryIdentity=extern`的Ink Function，也不得经call.*；外部调用必须使用abi.call或abi.invoke，向C导出的入口由abi.export record指定。由此callee identity与calling convention各有唯一选择，不存在call.*与abi.*都能表示同一C边界的重叠。

### 5.6 尾调用

尾调用只是优化，不是独立源码语义。只有同时满足以下条件时才允许 TargetABI 形成真正 tail call：

- 参数和结果物理 ABI 完全兼容；
- 当前函数没有尚未执行的 cleanup、defer、catch completion 或 pin release；
- caller destination 可以原样转交；
- 不绕过 stable entry 或版本选择；
- 不改变 traceback 和异常边界必须保留的可观察信息。

## 6. this、静态成员与地址调整

### 6.1 同步实例成员

同步实例成员具有一个隐式非拥有 receiver。语义分析保留 mutable 或 const 访问能力；TargetABI lowering 通常把它表示为指向声明类子对象的指针。

直接类调用在编译期完成已知 base-subobject 调整。普通非 virtual 调用不得根据运行时动态类型重新选择实现。

mutable receiver 不得由 const 观察路径建立。const receiver 的物理指针不得在 lowering 中丢失只读访问约束。

### 6.2 static 成员

static 成员没有 this。它使用与同签名顶层函数相同的逻辑调用形状，但名称、可见性和反射声明身份仍属于其 declaring class。

### 6.3 async 实例成员

async 实例成员在同步任务构造阶段求值 receiver，并把调整后的非拥有 raw this 指针保存进具体 task frame。

创建任务不会：

- 复制完整对象；
- 延长对象生命周期；
- 固定对象地址；
- 增加对象引用计数；
- 建立运行时悬空检查。

完整接收对象必须保持在原地址并存活到 Task 到达 succeeded 或 failed。Task 达到最终状态后，其后续销毁不得再访问 receiver 对象。

## 7. constructor、destructor 与 dynamic destroy

### 7.1 constructor

constructor 使用 caller 提供的完整对象存储作为隐式 destination。它没有独立返回值。

fresh destination 的规范操作：

~~~text
obj.init.begin %destination : (!place<init,T>) -> ()
call.direct @constructor(%arguments...) to %destination {destination_role = initializing_receiver}

obj.init.begin %destination : (!place<init,T>) -> ()
call.invoke callee_kind = direct @constructor(%arguments...) to %destination {destination_role = initializing_receiver}
    normal ^normal
    unwind ^unwind(exception)
~~~

转发当前 activation 已有的 ResultDestination 时不得再次 begin：

~~~text
call.direct @constructor(%arguments...) to %forwarded_destination {destination_role = initializing_receiver}

call.invoke callee_kind = direct @constructor(%arguments...) to %forwarded_destination {destination_role = initializing_receiver}
    normal ^normal
    unwind ^unwind(exception)
~~~

constructor entry 的 receiver_kind 必须是 initializing_instance。fresh storage owner 在首次 constructor call 前执行 obj.init.begin；转发层直接复用 entry ResultDestination 的 active transaction。constructor 使用 obj.init、obj.init.copy 等操作建立 base 和字段，并且作为最终完成者时只在完整 constructor body 正常结束后执行一次 obj.init.commit。normal edge 因此只观察到 committed 完整对象；最终失败的 constructor 在清理成功建立的子对象后，由 outward-unwind schema postcondition 把 transaction 回滚到 AllocatedUninitialized，中间转发层只传播这一 rolled-back 证明。obj.init 不携带 constructor symbol，只用于平凡或已经完成求值的标量、子对象初始化。

constructor 必须按语言规定的顺序初始化：

1. 直接 concrete base；
2. 当前 class 字段声明顺序；
3. constructor body。

normal edge 建立完整对象生命周期。unwind edge 只清理已经成功初始化的 base 和字段，不运行完整对象 destructor。

constructor normal completion只建立destination Alive proof并为完整对象创建fresh ObjectLifetimeGeneration；caller随后以place.as_alive取得完整对象capability。constructor的initializing_instance PlaceCapabilityGeneration在commit/rebind后失效，不能作为普通this逃出。

constructor 不能 virtual dispatch，不能作为普通反射 FunctionInfo 动态调用入口，也不能通过隐式结果临时量实现。

### 7.2 destructor

用户 destructor body 是编译器管理的生命周期函数：

- 隐式接收完整、已初始化对象的 mutable this；
- 返回 void；
- 隐式 nothrow；
- 不能作为普通函数直接调用；
- 不能加入普通 virtual slot；
- 不能通过动态反射公开。

规范性 destroy 操作：

~~~text
obj.destroy %place : T
~~~

静态已知完整类型时，destroy 执行：

~~~text
most-derived user destructor
    → fields in reverse declaration order
    → direct base destructor
    → continue along the single concrete base chain
~~~

obj.destroy 完成后必须先按唯一析构顺序终结全部descendant leaf/child ObjectLifetimeGeneration，再终结complete-object generation，并只建立storage为uninitialized的状态证明；旧Alive capability及所有捕获任一旧generation的reference/interface/DynamicRef/slice view不得继续使用。需要重新初始化同一storage时，必须在destroy支配且无active borrow/child lifetime的owner路径上用place.as_uninitialized取得新的!place<init,T>；下一次commit或direct-leaf obj.init产生fresh generation，不复活旧view。对uninitialized、已经销毁、SealedRuntimeStorage或只有borrow LifetimeAuthority的place执行obj.destroy是非法IR。

### 7.3 dynamic destroy

每个 virtual class vtable 逻辑上具有一个 compiler-generated dynamic destroy entry。

规范操作：

~~~text
obj.destroy_dynamic %complete_object, %vtable
~~~

dynamic destroy：

- 根据对象动态完整类型执行与静态 destroy 相同的完整析构链；
- 必须 nothrow；
- 只结束对象生命周期；
- 要求complete object place具有owner LifetimeAuthority，并终结其ObjectLifetimeGeneration；
- 不释放对象存储；
- 不猜测 allocator、allocation base、size 或 alignment；
- 不能作为普通源语言函数取得或反射。

`obj.destroy_dynamic` 的静态 effect 是所选 dynamic-destroy stable envelope的 `CallSummary`、`RuntimeEffect(dynamic_destroy)`、`RuntimeEffect(version_select)`、`EndLifetime` 与 `MayDiverge` 的并集；完整析构链的memory/runtime/allocate/deallocate/trap效果不得被 `nothrow` 擦除。vtable entry必须以原子select+pin选择当前兼容version-local destroy thunk，结束后释放pin；关闭热更新后的合法pass只有证明版本在整个dynamic extent冻结时才可去掉 `version_select`。若destructor违反nothrow或运行时无法完成销毁，operation进入fatal而不是normal、unwind或trap completion。

拥有型多态容器必须在 dynamic destroy 之后，使用自己保存的 allocation base 或 token 和 allocator contract 释放存储。

析构、defer 或 dynamic destroy 让异常逃逸时，运行时进入 fatal，不用第二个异常替换当前展开。

## 8. 异常与展开模型

### 8.1 逻辑异常对象

每个正在传播的异常具有一个 opaque ExceptionRecord 语义对象。它至少承担：

- 平台展开状态；
- 动态异常 descriptor；
- 最派生 payload 最终存储；
- payload destroy entry；
- 可选 cause；
- ThrowSite；
- 可选 traceback；
- 所需 module version pins。

这些职责是规范性的；字段顺序、位宽、分配块数量和平台展开头布局不是规范性的。

### 8.2 exception token

InkIR 使用线性的 exception token 表示当前异常控制权。token：

- producer 闭集为 call.invoke、async.invoke、abi.invoke、decorator continuation 等 schema-declared unwind edge，eh.throw、eh.throw_copy、eh.throw_from 创建的新传播，以及 failed async.await/async.await_copy；其他 opcode 不得产生 token；
- 不能写入普通内存、class 字段、tuple、Task 结果或全局对象；
- 不能复制；
- 必须由 eh.end_catch、eh.resume、eh.rethrow、eh.throw_from ownership transfer 或 `rt.fatal %exception` 消耗；无exception operand的rt.fatal不消费token；
- 不能从一个 handler list 回到同一列表重新匹配。

物理 TargetABI 不要求 token 真的是一个机器指针。

这里的“线性”指每条动态控制路径上的唯一所有权，不等于 SSA value 只能出现一次。eh.payload 等 schema 明确声明的 readonly borrow 可以多次读取 active token，但不能复制或转移其所有权；eh.match successor argument 和最终消费 operation 才形成所有权路径。

eh.rethrow 和 eh.resume 只把现有 token/ExceptionRecord 的所有权转移到外层传播，不创建新 token 或新逻辑异常记录。任何新增 runtime schema 若声明 unwind edge，必须在中央 opcode registry 中明确 exception producer、edge type 与 ownership transfer；自由属性不能扩张 producer 闭集。

### 8.3 throw 操作

规范操作：

~~~text
eh.throw @ExceptionType(%constructor_arguments...)

eh.throw_copy %existing_exception

eh.throw_from @NewExceptionType(%constructor_arguments...), %current_exception

eh.rethrow %current_exception

eh.resume %exception
~~~

所有上述操作都是 terminator，不产生 normal successor。

eh.throw 只接受中央`ExceptionPayloadClass`的concrete、complete、nonabstract class，并由P17绑定准确initializing receiver constructor与参数，直接在新 ExceptionRecord payload 中构造异常。eh.throw_copy的source类型同样必须满足ExceptionPayloadClass、Copyable与CopyConstructionEnabled，并把Alive命名源对象结构化复制到 payload。typed catch则可以选择ExceptionPayloadClass或继承runtime exception marker的ExceptionCatchInterface。

eh.throw_from 必须按以下顺序提交：

1. 取得新记录；
2. 构造新 payload；
3. 构造成功后把 current exception 连接为直接 cause；
4. 开始传播新记录。

如果新 payload 构造自身抛出另一个异常，cause link 尚未提交。operation先rollback/deallocate新record的partial payload，再以`ExceptionDestroySummary`释放当前handler的旧active owner，最后只传播constructor产生的新异常；旧异常不自动成为cause，也不能以“保留旧token”形成第二条活跃ownership。成功路径则原子把旧owner转移为新record的direct cause，两条路径都恰好消费输入token一次。

eh.rethrow 复用同一逻辑记录、payload、cause、throw site 和 traceback，并从当前 handler list 外继续搜索。

### 8.4 catch 与 handler dispatch

规范操作或结构：

~~~text
%active = eh.entry %exception : !exception
eh.match %active [
    class @ConcreteError -> ^handler0(%active)
    interface @IoError -> ^handler1(%active)
    catch_all -> ^handler2(%active)
] unmatched ^outer_unwind(%active)

^handler0(%caught : !exception):
    %payload = eh.payload %caught {as = class @ConcreteError}
    ...
    eh.end_catch %caught -> ^normal
~~~

eh.entry 取得 unwind successor 传入的 !exception 并建立当前 handler list 的 active token。eh.match 是 terminator，并把该 token 线性转移给唯一选中的 handler 或 unmatched successor。eh.payload 只建立受 token 生命周期约束的 readonly borrow，不消耗 token；handler 最终必须由 eh.end_catch、eh.rethrow、eh.throw_from、eh.resume 或 fatal 恰好选择一个所有权终点。

handler 必须严格按源码顺序检查。不得重排可能重叠的 class 和 exception-interface handler。

InkIR 不提供 exception filter 或 multi-catch。handler body 中的普通 if 是普通控制流，eh.rethrow 不会回到同一 eh.match 尝试后续 handler。

类型化 catch 建立：

~~~text
const ConcreteError&
const ExceptionInterface&
~~~

exception-interface 绑定使用普通两字 interface 引用语义，但对象所有权仍属于 ExceptionRecord。

catch-all 绑定建立受限的 const ExceptionView。ExceptionView 不可复制、不可存储、不可跨 handler 生命周期逃逸。

handler 正常完成时使用：

~~~text
eh.end_catch %current_exception -> ^normal
~~~

eh.end_catch 消费当前active token并释放其ActiveUnwindRecord backing owner，然后进入handler的正常后继。普通独占ExceptionRecord的最后owner释放会销毁payload、record和pins；从failed Task建立的ActiveUnwindRecord只释放自己持有的ExceptionBox强引用，只有最后一个box强引用释放时才恰好销毁共享payload、cause、diagnostic state和pins。eh.rethrow转移同一backing owner；eh.throw_from成功连接cause时把旧backing owner转移给新record。上述转移后不得再执行eh.end_catch。

### 8.5 cleanup edge

每个可能展开的作用域必须在 InkIR 中显式表现 cleanup：

- 已成功初始化局部对象逆序 destroy；
- defer 后进先出；
- 部分构造子对象逆序 destroy；
- 活动 enum payload destroy；
- Task、reflection snapshot 和其他 runtime handle 的状态相关释放。

cleanup block 接收 exception token，并最终：

- 进入 eh.match；
- 执行 eh.resume；
- 转换成 task failure；
- 或进入 fatal。

cleanup block 本身不得让第二个异常向外展开。

### 8.6 RuntimeWorld 与 TargetABI

RuntimeWorld 使用逻辑调用栈、handler 栈和 ExceptionRecord 模型实现上述语义，不依赖宿主 C++ exception。

TargetABI 可以分别降低为：

- Itanium 风格 personality、landing pad 和 unwind record；
- Windows EH/funclet；
- 其他目标提供的等价零成本或显式展开机制。

不同目标的 ExceptionRecord 物理布局可以不同，但 handler 顺序、cleanup、cause、rethrow 和只读 catch 语义必须一致。

## 9. class、vtable 与 virtual 调用

### 9.1 对象表示

普通 class 只包含显式状态和必要填充。一个 class 声明或继承 virtual function 时，完整对象具有一个主 vptr。单一 concrete inheritance chain 共享该主 vptr。

vptr 不是普通字段：

- 不能由源码读写；
- 不出现在普通 fields 反射列表；
- 计入 size 和 alignment；
- 复制完整 virtual class 时由编译器为目标完整类型建立正确 vptr。

### 9.2 vtable 语义实体

InkIR vtable 实体必须逻辑关联：

- 当前 class layout version；
- minimal dynamic type descriptor；
- dynamic destroy entry；
- 继承得到的 virtual slots；
- 当前 class 新增的 virtual slots；
- 每个 slot 的精确 logical signature 和 function kind；
- hot reload stable entry 或等价受保护目标。

override 复用根 slot。新 virtual function 按确定性声明顺序追加。

同步 virtual override 的参数和结果类型必须与根 slot 精确一致。Ink v0 不支持同步协变返回。async virtual override 同样要求逻辑结果精确一致。

实现 thunk 可以调整 this，但不能通过不同调用视图改变同步或异步结果类型。

### 9.3 virtual 调用操作

规范操作：

~~~text
%result = call.virtual %receiver, #slot(%arguments...)

call.virtual %receiver, #slot(%arguments...) to %destination {destination_role = result}

call.invoke callee_kind = virtual %receiver, #slot(%arguments...)
    normal ^normal(result(0))
    unwind ^unwind(exception)

call.invoke callee_kind = virtual %receiver, #slot(%arguments...) to %destination {destination_role = result}
    normal ^normal
    unwind ^unwind(exception)
~~~

call.virtual 只用于 slot contract nothrow。may-unwind slot 必须使用 callee_kind = virtual 的 call.invoke。

virtual receiver是checked reference/place时必须携带其complete object当前ObjectLifetimeGeneration，调用和this-adjustment都要求该generation仍匹配Alive object。receiver access只控制读写；来自reference/borrow projection的rw receiver仍为borrow LifetimeAuthority，不能借virtual call、thunk或access调整取得destroy/reinitialize权限。

反射 virtual 调用取得 FunctionInfo 后仍必须通过同一个 virtual slot 进行最终分派；descriptor 来源不等于最终实现。

abstract slot 在任何可实例化动态类型中都必须已经由有效 override 填充。Closed InkIR 不得生成可到达的 abstract-slot call。

## 10. interface 表示与调用

### 10.1 interface 引用

interface 引用的规范语义是：

~~~text
InterfaceRef {
    canonical_complete_object_pointer
    interface_table_pointer
    ObjectLifetimeGeneration (verifier/interpreter side fact)
}
~~~

它是非拥有、可复制的两字逻辑值，但在 InkIR 中属于 address-only 聚合。建立时捕获complete object的当前ObjectLifetimeGeneration；复制保留同一generation，不复制实现对象、不增加对象引用计数、不延长对象生命周期，也不自行取得module pin。每次调用、cast、projection或反射包装都要求该generation仍匹配当前Alive complete object；destroy终结旧generation，同址重建产生fresh generation，因此旧interface reference不会复活。rw只表示可写borrow，不授予obj.destroy、init、commit或reinitialize等lifecycle authority。

canonical object pointer 保持完整实现对象的规范地址。具体 base-subobject 调整由 interface table thunk 完成。

### 10.2 interface table

每个实际使用的 concrete-class/interface 组合具有逻辑上独立的 interface table。父 interface table 不是子 interface table 的内存前缀。

interface table 必须逻辑关联：

- concrete dynamic type descriptor；
- interface identity 和 layout version；
- 按接口有效方法集合确定的 slots；
- 到每个传递 ancestor interface table 的常数时间 conversion entry；
- hot reload 所需 stable entries 或等价受保护目标。

仅实现多个 interface 不给对象增加 interface table pointer。

### 10.3 interface 调用操作

规范操作：

~~~text
%result = call.interface %receiver, #slot(%arguments...)

call.interface %receiver, #slot(%arguments...) to %destination {destination_role = result}

call.invoke callee_kind = interface %receiver, #slot(%arguments...)
    normal ^normal(result(0))
    unwind ^unwind(exception)

call.invoke callee_kind = interface %receiver, #slot(%arguments...) to %destination {destination_role = result}
    normal ^normal
    unwind ^unwind(exception)
~~~

interface method、default method、子接口 override 和 concrete class 实现的参数与结果必须与对应 slot 精确一致。Ink v0 不支持 interface result covariance。

class implementation thunk 调整 canonical object pointer 并进入 class method。default method 接收规范化到其 declaring interface 的胖 receiver，并在方法内部继续通过该 table 调用其他 interface methods。

call.interface/callee_kind = interface在读取table或进入thunk前必须验证receiver捕获的ObjectLifetimeGeneration仍匹配canonical complete object；table转换和this调整保留该generation，从descriptor形成的receiver place只能具有borrow LifetimeAuthority，不能仅凭两字物理descriptor相等接受已经失效的receiver。

子 interface 到 ancestor interface 的静态上转型只替换 table pointer。父到子或横向转换必须使用动态 try_cast 语义。

### 10.4 动态转换

相关规范操作：

~~~text
cast.interface_make
cast.interface_up
cast.class_up
cast.try_class
cast.try_interface
~~~

cast.try_class 和 cast.try_interface 使用 minimal type descriptor、layout version、single concrete base chain 和 interface relation。失败产生 Optional 引用或 null pointer，不抛异常。成功产生checked reference/interface/Optional时保留源complete object的ObjectLifetimeGeneration与non-owning referent语义；由结果形成的place只能是borrow authority。产生raw pointer的variant显式丢弃checked generation，raw pointer本身不携带它。

这两个 cast.try_* 操作只接受generation仍匹配当前Alive complete object的源reference或interface view。它们不是任意地址验证器，也不能让destroy前的旧view在同址重建后重新有效。

## 11. 动态反射调用 ABI

### 11.1 pinned reflection snapshot

运行时反射使用版本固定快照语义。

reflect.lookup_type、reflect.lookup_interface 和 reflect.lookup_function 在 found edge 上原子选择当前已发布 module version、取得 pin，并产生对应 kind 的 owned snapshot handle；missing edge 不产生 handle。选择必须使用10 §9.5的唯一reflection name index：type/interface按`(module, qualified name, descriptor kind)`，function再加入expected FunctionType semantic identity；同key多项不是运行时任选，而是非法artifact。reflect.lookup_member借用既有owned snapshot，以`(snapshot subject, member name, member kind)`查找且只产生受owner lifetime限制的member view；P24没有expected function type，所以v1不允许同一reflected owner下同名同kind重载。snapshot 生命周期内：

- qualified name、metadata、layout、slot、adapter 和声明关系保持该版本；
- hot update 不把旧 snapshot 静默重绑定为新版本；
- 新查询才观察事务发布后的新版本；
- snapshot 持有阻止相关 descriptor、adapter 和代码卸载的 pin；
- snapshot 释放后才能解除该 pin。

旧 FunctionInfo snapshot 的调用进入该 snapshot 对应的版本专用 adapter 和实现，不经过当前版本 stable entry 偷换成新实现。

InkIR reflection snapshot 是线性、不可存储、不可逃逸的 !runtime-handle<*_snapshot> owner capability，不是普通 SSA 数据或 address-only 用户对象。reflect.snapshot.clone 显式产生第二个线性 owner，reflect.snapshot.release 消费一个 owner；普通 Copy、memcpy 和把 handle 写进 Task 都非法。异步 reflection 在构造 Task 时把从 snapshot 导出的 version pin 转移给 Task，而不是保存调用者的 snapshot handle。member view 只借用 snapshot，不得在 owner 释放后使用。v0 不另设 reflect.snapshot.acquire：lookup 的 found transition 已经完成 select+pin，避免同一次查询重复 acquire。

qualified/member String的准确UTF-8 NFC bytes仍是反射语义身份，并分别匹配descriptor的canonical qualified LookupName或末尾logical name segment。descriptor pointer、hash、容器index和snapshot handle都只是当前进程缓存，不能持久化，也不能用locale folding或display alias替代LookupName。

### 11.2 private DynamicRef adapter record

DynamicRef 只是 reflection TargetABI 在单次adapter activation内构造的私有借用描述符，不是canonical Core type、SSA value、block argument或`!runtime-handle`。Closed IR的reflect/call operand仍使用准确普通类型：scalar/unit使用对应SSA值，reference/raw pointer使用对应typed handle，address-only object使用携带generation与authority side fact的`!place<ro/rw,T>`，`object` passing mode使用3.3节的prepared owner place。`reflect.call` 的 P25 payload 编码 `expected_function_type`、`receiver_present`、`argument_count`、`adapter_nothrow` 与 optional destination；普通 `call.invoke callee_kind = reflection` 使用完整 P14 `CallPayload`。两者都由 expected function type、typed operands和显式参数数量唯一恢复每项 passing mode，lowering据此临时形成DynamicRef；不存在独立DynamicRef producer opcode，也不能在binary/text中伪造一个descriptor value。

私有 DynamicRef 至少携带：

- data address；
- type identity；
- object layout version；
- mutable 或 readonly 能力；
- complete object 的 ObjectLifetimeGeneration；
- non-owning referent 语义；
- snapshot 或等价有效期关联。

DynamicRef 不拥有对象、不复制对象、不延长对象生命周期；adapter转发只能保留输入typed operand的同一generation，不能刷新它。每次动态类型检查、receiver/argument适配和调用都要求generation仍匹配当前Alive complete object；destroy后同址重建不会恢复旧descriptor。mutable只允许在当前generation内写对象，不授予lifecycle authority。scalar参数所需的私有ABI临时地址只活到adapter返回，不能变成语言对象或捕获；异常catch payload若经adapter观察只能形成readonly descriptor。DynamicRef数组不得跨CFG、暂停、调用返回、snapshot release或Task construction activation逃逸。

### 11.3 同步 reflection adapter

同步动态调用的逻辑 ABI 为：

~~~text
ReflectionCall {
    optional typed receiver operand, privately lowered to DynamicRef
    ordered typed operands, privately lowered to DynamicRef records
    optional DynamicOut result
    pinned FunctionInfo snapshot
}
~~~

DynamicOut 是reflection adapter的逻辑/TargetABI lowering record：它指向带唯一active transaction的caller destination，并携带expected type、layout version、alignment、transaction identity和initialization state。canonical InkIR不物化可存储的DynamicOut SSA；`reflect.call` 的 P25 `ReflectCall` 或普通 reflection `call.invoke` 的 P14 `CallPayload` 直接编码 optional `DestinationPayload` 和准确 function type，lowering据此建立DynamicOut。fresh owner在调用前执行obj.init.begin；转发已有ResultDestination时复用其transaction。

规范操作：

~~~text
%scalar = reflect.call %function_snapshot receiver(%receiver) args(%arguments...) {expected_function_type = !tFn, receiver_present = true, argument_count = N, adapter_nothrow = true}

reflect.call %function_snapshot receiver(none) args(%arguments...) {expected_function_type = !tFn, receiver_present = false, argument_count = N, adapter_nothrow = true} to %destination {destination_role = result}

call.invoke callee_kind = reflection %function_snapshot receiver(%receiver)(%arguments...) {function_type = !tFn, entry_identity = version_local, calling_convention = ink, target_abi_tag = DIGEST, explicit_argument_count = N} to %destination {destination_role = result}
    normal ^normal
    unwind ^unwind(exception)
~~~

adapter 必须在调用前验证：

- function kind 是 sync；
- receiver 动态类型和可变性；
- 参数数量和顺序；
- 每个参数的精确类型和 parameter_passing_mode；reference/raw-pointer 的可变性与 pointee 由类型本身精确匹配；
- 精确结果类型；
- calling convention；
- snapshot、对象 layout 和 module version 兼容性；
- 每个receiver/argument DynamicRef捕获的ObjectLifetimeGeneration仍与当前Alive complete object匹配，且non-owning view没有被伪造成owner-authority place。

`adapter_nothrow` 是独立于目标函数nothrow的已验证snapshot contract。只有expected signature、receiver/argument动态类型、layout generation、calling convention、adapter资源取得和结果适配均已由lookup或支配证明保证不会产生结构化反射异常，且目标sync函数也nothrow时，它才为true。`reflect.call`只接受`target_nothrow && adapter_nothrow`；任一证明缺失或为false都必须使用`call.invoke callee_kind = reflection`。不能因为目标body nothrow就删除adapter验证的unwind edge。

具有address-only结果的adapter只有在准确结果完整构造后才执行唯一obj.init.commit。验证、资源取得或目标调用导致adapter outward unwind时不得commit；adapter先清理部分结果，outward-unwind schema postcondition再把DynamicOut transaction回滚到AllocatedUninitialized。reflect.call/call.invoke本身不合成rollback；中间adapter若只转发destination，则不得重复begin、commit或rollback。normal successor只建立Alive proof，caller用place.as_alive取得结果capability。

动态反射不执行默认实参、不执行数值强制转换、不执行用户定义隐式构造。合法反射输入不匹配时抛出结构化反射异常，不产生错误 ABI 调用或 UB。

### 11.4 异步 reflection adapter

异步动态调用在TargetABI lowering中使用独立DynamicTaskOut：

~~~text
DynamicTaskOut {
    address of !runtime-object<task,R> storage with active init transaction
    expected logical result R
    task ABI tag
    transaction identity
    initialization state
}
~~~

规范操作：

~~~text
async.invoke callee_kind = reflection %function_snapshot receiver(%receiver)(%arguments...) {function_type = !tFn, entry_identity = version_local, calling_convention = ink, target_abi_tag = DIGEST, explicit_argument_count = N} to %task_destination {destination_role = result}
    normal ^created
    unwind ^creation_failed(exception)
~~~

canonical async.invoke的DestinationPayload在lowering时形成DynamicTaskOut。adapter必须验证function kind是async、logical result精确等于R且transaction有效，然后在最终Task存储中直接构造一个created task，并由async.invoke schema原子执行Task-specific commit postcondition。它不能建立Task临时量、复制Task、形成Task::<Task::<R>>，也不能保存调用者栈上的private DynamicRef数组；frame只能保存按准确passing mode完成handoff的typed capture。creation_failed不得执行Task-specific commit；adapter完成部分状态cleanup后，由其outward-unwind schema postcondition回滚该transaction。created successor只建立Alive proof，caller以place.as_alive取得Task capability。

任务创建成功后，异步 body 失败通过 Task failed(ExceptionBox) 通道观察，不重新包装成 ReflectionInvocationError。reflection 创建期校验和资源失败仍从 creation_failed 同步展开。

## 12. async Task 调用约定

### 12.1 Task 逻辑状态

Task::<T> 至少具有以下不可逆状态：

~~~text
created
    → pending
        → succeeded(ResultStorage::<T>)
        → failed(ExceptionBox)
~~~

没有 cancelled 终态，也没有 observed bit。取消只是独立单向请求状态。

T 为 void 或 unit 时 succeeded 不需要物理 result storage；两者的语义区别仍由 Task logical type 和 async.await 的 normal edge 保留。

Task 是具有完整 size/alignment 的 semantic runtime object、address-only 且 noncopyable；其 canonical type 是 !runtime-object<task,T>，不是 runtime.opaque。T 必须满足 v0 Task result admissibility：void/never，或 closed runtime-representable、Copyable 且 CopyConstructionEnabled，且不能含 no-escape value。物理 frame pointer、控制块、waiter list、result placement、cancel bit 和 pin 集合仍属于私有 TargetABI，不能由普通 projection 或 memory opcode观察。

每个成功构造的Task generation还携带不可变、可验证的effect side facts；它们不是Task类型身份、用户可读字段或可序列化的自由payload：

~~~text
TaskEffectFacts {
    TaskBodyEffect
    TaskDestroySummary
}
~~~

TaskBodyEffect是从created/pending经drive/await可能执行的完整version-pinned async body上界，包含用户调用、memory/runtime/allocate/deallocate、TargetDependent、PdbBoundary、trap和diverge。TaskDestroySummary覆盖created capture/frame、succeeded result/frame及failed ExceptionBox/frame/pin最后释放的状态并集；path-sensitive verifier可以用已证明状态选择更窄分量，否则使用完整并集。construction从所选pinned version的准确body/envelope和capture/result cleanup推导这两个facts；stable selection从compatibility record取得上界。facts随准确Task generation及owner/borrow capability传播，CFG phi逐字段取并集；跨函数或其他无法保持准确provenance的位置退化为registry定义的最大保守上界，覆盖ReadWriteAny、全部runtime/allocate/deallocate kind、TargetDependent、PdbBoundary、MayTrap和MayDiverge。任何Task consumer的静态effect必须并入对应side fact或该保守上界，不能只按`Task<T>`推导。

async body/generated resume 的 entry envelope 始终由 Task runtime建立。`task_self` 是当前 Alive Task 的 borrowed `!place<rw,!runtime-object<task,T>>`，为 drive、cancel query 与最终 publish 提供规范 operand 来源；它不可被 body destroy、reinitialize、转成 raw address或逃逸。若 logical `T` 为 address-only，runtime 还提供独立 `task_result_storage : !place<init,T>`，具有结果对象的 owner lifecycle authority但没有 frame deallocation authority。该 storage 初始为 `AllocatedUninitialized` 且没有 active transaction；scalar/unit/void `T` 不具有这一 hidden argument。

### 12.2 async 调用与构造边

所有 async 调用都使用 Task caller destination：

async.call 和 async.invoke 的 callee_kind 至少支持 direct、indirect、virtual、interface 和 reflection。普通 async function value 不携带 construction nounwind 证明，因此 callee_kind = indirect 默认必须使用 async.invoke。

下列 fresh-storage 示例先建立唯一 transaction：

~~~text
obj.init.begin %task_destination : (!place<init,!runtime-object<task,T>>) -> ()
async.invoke callee_kind = direct @async_function(%arguments...) to %task_destination {destination_role = result}
    normal ^created
    unwind ^creation_failed(exception)
~~~

已证明同步构造不展开时可以使用：

~~~text
obj.init.begin %task_destination : (!place<init,!runtime-object<task,T>>) -> ()
async.call callee_kind = direct @async_function(%arguments...) to %task_destination {destination_role = result}
~~~

如果 task_destination 是当前 activation 转发的 ResultDestination，则省略上述 obj.init.begin，继续使用传入的 active transaction。中间转发层不得commit；最终async operation的normal schema postcondition原子验证created sealed state、消费transaction、建立fresh Task ObjectLifetimeGeneration并产生Alive proof。该Task-specific commit不是、也不得lower回canonical generic `obj.init.commit`。

同步任务构造阶段必须：

1. 求值并捕获完整实参；
2. 选择最终 static、virtual、interface 或 reflection implementation；
3. 受保护地取得对应 module version pin；
4. 取得并初始化具体 frame；
5. 保存 resume、destroy 和 task exception boundary；
6. 在最终 Task 存储中建立 created 状态；
7. 把 pin 转移给已构造 Task；
8. 由async schema原子执行唯一Task-specific commit postcondition并进入normal edge。

created/async.call normal completion只建立Task destination的Alive proof；caller必须以place.as_alive取得可await、drive或destroy的Task capability，原init generation不得继续使用。

任一步骤在 Task 完整构造前失败时：

- 同步 unwind；
- 清理已经建立的参数和 frame 部分状态；
- 释放已经取得但尚未转移的 pins；
- 不执行Task-specific commit，并由task-construction callee的outward-unwind schema postcondition把task_destination的active transaction回滚到AllocatedUninitialized；
- 不建立 ExceptionBox。

### 12.3 async virtual slot

async virtual slot 的规范物理意图是：

~~~text
async.invoke callee_kind = virtual %receiver, #slot(%arguments...) to %task_destination {destination_role = result}
    normal ^created
    unwind ^creation_failed(exception)

construct_task(
    RuntimeObject<task,T>* result_storage,
    DeclaringClass* dispatch_this,
    arguments...
)
~~~

slot 在任务创建时选择最终 override。所选 thunk 负责从根 slot 的 declaring-class receiver 调整到最终实现 this，并构造该实现的具体 frame。

只有slot的effective BehaviorContracts证明construction_nothrow时，调用点才可改用callee_kind = virtual的async.call。

第一次 await 不重新读取 vptr，不重新选择 override，也不因 hot update 改写已有 frame。

### 12.4 async interface slot

async interface slot 的规范物理意图是：

~~~text
async.invoke callee_kind = interface %receiver, #slot(%arguments...) to %task_destination {destination_role = result}
    normal ^created
    unwind ^creation_failed(exception)

construct_task(
    RuntimeObject<task,T>* result_storage,
    void* canonical_object,
    InterfaceTable* dispatch_table,
    arguments...
)
~~~

选择 class 实现时，frame 保存调整后的 raw this。选择 default method 时，frame 保存规范化到 default method declaring interface 的胖 receiver。

只有slot的effective BehaviorContracts证明construction_nothrow时，调用点才可改用callee_kind = interface的async.call。

interface 分派在任务创建时只执行一次。class method 同时是 virtual function 时，interface slot 必须直接选择最终 class task-construction thunk，不得在第一次 resume 再读取主 vtable。

### 12.5 await

规范操作：

~~~text
async.await %task
    normal ^success(result(0))
    unwind ^failure(exception)

async.await_copy %task to %destination {destination_role = result}
    normal ^success
    unwind ^failure(exception)
~~~

async.await 和 async.await_copy 在 canonical CFG 中都恰有 normal 与 unwind 两个 successor，不存在第三个 suspend successor，也不存在 Suspend edge argument tag。遇到 pending Task 时，operation 暂停当前 activation、登记与该 await operation 及其 frame state 绑定的 continuation，并且暂时不选择任何 CFG successor；Task 完成后恢复该 operation，随后才根据最终状态进入 normal 或 unwind。binary FunctionRecord 因此只编码这两个 successor，暂停所需 state 由 operation schema 和 coroutine frame lowering 表达。

两种 await 只允许出现在 verifier 标记为可暂停的 async/coroutine activation。pending 时，所有跨暂停 live SSA、place、borrow、ResultDestination transaction identity、部分初始化/cleanup state 和所需 version pin 都必须具有合法 frame representation，并在暂停前 spill 到生命周期足够长的 frame。no-escape safe slice、指向当前 native stack/alloca 的 borrow 或 raw pointer、active exception token、continuation capability，以及其他不能证明 suspend-safe 的临时能力不得跨暂停存活。ResultDestination 只有在最终 storage 地址稳定、frame 持有其 lifetime/pin 且 transaction 与 cleanup state 一起 spill 时才能跨暂停。

async.await 语义：

- created：原子进入 pending 并只驱动一次；
- pending：登记与当前 await operation 绑定的 continuation 并暂停 activation；完成后恢复同一语义 operation，再选择 normal 或 unwind；
- succeeded：从只读结果存储取得结果；
- failed：为当前等待者从共享 ExceptionBox 建立独立 ActiveUnwindRecord，并进入 unwind edge。

T 为 void 时 normal edge 没有结果；T 为 unit 时 normal edge 接收唯一 unit semantic value且没有物理 payload；单标量 T 使用 normal block argument。address-only、可复制 T 使用 async.await_copy 直接 copy-initialize destination。Task::<T> 的按值结果要求 T 可复制；async.await 不引入隐藏移动。

async.await_copy 是 caller-destination blanket rule 的显式 schema 特例。它的opcode schema声明ReadMemory(task_state)、ReadMemory(task_result)、WriteMemory(task_state)、WriteMemory(typed)、BeginLifetime、Allocate(exception)、RuntimeEffect(async_suspend)、MayUnwind、MayDiverge和Control，并满足以下pre/postconditions：

- entry 要求 destination 已经携带唯一、未提交的 active transaction；operation 不执行 begin；
- pending 时 transaction identity、destination storage 和相关 cleanup state 一起保存在可跨暂停存活的 activation/frame 中，不执行 commit 或 rollback；
- Task succeeded 后，operation 直接从只读 result storage 对 destination 执行语义 Copy；完整 copy 成功是 normal postcondition，并由该 schema 原子地执行唯一 commit；
- Task failed 是唯一 failure postcondition；typed Copy 按类型和值规范不会 unwind。operation 在失败时不开始 Copy，直接由 schema 把 transaction 回滚到 AllocatedUninitialized，并从共享 ExceptionBox 为当前等待者建立 canonical exception edge value；
- normal/unwind postcondition 都恰好消费这一 transaction；不存在 obj.init.abort opcode，也不得在 successor 中再补一次 commit 或 rollback。

### 12.6 Task 完成与销毁

相关规范操作：

~~~text
async.task.publish_success
async.task.publish_failure
async.cancel.request
async.cancel.is_requested
async.task.drive_once
async.task.destroy
~~~

async.task.publish_success 的 void/unit variant 没有物理结果；value variant 把标量写入私有 result storage 后原子发布 succeeded；object variant 要求 entry `task_result_storage` 已在实际 return-result 构造点 fresh begin、完整直接构造、commit，并经 `place.as_alive` 物化为该 Task 内部只读 ResultStorage，再原子发布 succeeded。async.task.publish_failure 捕获或复用当前异常 box，然后原子发布 failed；若 address-only result transaction 曾开始，generated boundary 必须先逆序清理并 rollback，使 entry storage 回到 `AllocatedUninitialized`，若从未 begin 则保持该状态。

async.task.destroy 必须按状态执行：

~~~text
created   → destroy captured arguments and frame
succeeded → destroy result and frame
failed    → release ExceptionBox and frame
pending   → fatal
~~~

pending destroy 不抛可捕获异常、不隐式 wait、不隐式 detach。

drive_once与从created状态执行的await/await_copy必须并入TaskBodyEffect；destroy必须并入TaskDestroySummary，path-sensitive状态已知时才可使用其对应分量。不能因为runtime entry本身nothrow就删除capture/result destructor、ExceptionBox最后释放、pin release或fatal/diverge效果。

### 12.7 ExceptionBox 与 failed await

task exception boundary 位于最终 async decorator 链之外。异常逃出完整 task body 时：

- 如果是普通活动 ExceptionRecord，则把其 payload、cause 和诊断状态重新归属到 immutable ExceptionBox；
- 如果异常已经来自另一个 failed Task，则保留已有 box，不复制 payload；
- 发布 Task failed 状态；
- 不触发普通线程 fail-fast。

每次 failed await 单独建立 ActiveUnwindRecord。多个等待者可以并发传播同一个 immutable payload，但各自具有独立平台展开状态和 await site。

failed Task持有ExceptionBox的一个强引用。每次failed await为新ActiveUnwindRecord取得独立强引用；Task destroy、eh.end_catch和其他终止/转移路径只释放各自的一份引用。payload、cause、诊断状态与pins保持immutable，并且只在最后一个强引用释放时销毁一次。waiter之间不共享ActiveUnwindRecord、exception token或平台unwind header，因而一个waiter完成catch不会使其他waiter的payload悬空。

### 12.8 取消和组合等待

v0 canonical InkIR 只保留两个取消原语：

~~~text
async.cancel.request
async.cancel.is_requested
~~~

async.cancel.request 是 nothrow、幂等、线程安全的请求发布，不直接运行用户代码、不结束任务、不产生取消异常。

普通 async.await 不传播当前任务的取消请求。需要“等待期间把当前请求传播给 child”的源码/库操作必须 elaboration 为 generated state machine 和已注册 runtime callable，并无丢失地处理“请求先发生、登记并发发生、完成先发生”三种竞态；v0 不注册 async.await_cancel_on_request opcode。

all 与 all-cancel-on-error 同样是库/state-machine 语义，不是 v0 canonical opcode。其实现仍必须在任何失败离开组合边界前等待全部输入到达最终状态，多个失败按源码参数顺序选择；取消版本只向未完成 siblings 请求取消，仍等待它们真正结束。

这些高层操作必须在生成 canonical Closed InkIR 前展开为基本 async.await、cancel 原语、普通 CFG 和具有 typed schema 的 runtime callable；不得残留 async.all、async.all_cancel_on_error 或 async.await_cancel_on_request 拼写，也不得在展开中改变稳定临时 Task 存储、确定性失败选择或 cleanup 语义。

## 13. decorator continuation 调用约定

decorator continuation 是静态、不可逃逸的局部调用边界，不是普通 function pointer、stable entry 或 Task。

相关规范操作：

~~~text
decorator.region
decorator.continuation_invoke
decorator.continuation_yield
async.continuation_invoke
~~~

同步 decorator 中每次 decorator.continuation_invoke：

- 使用被装饰函数的精确参数、结果、receiver、calling convention 和 ABI；
- 建立下一层 region 的新动态 activation；
- 允许零次、一次或多次顺序进入；
- 把下一层 return 改写为 decorator.continuation_yield；
- 不让原 body return 越过 decorator 后置代码直接退出最终函数。

async decorator 中每次 async.continuation_invoke：

- 可以零次、一次或多次顺序进入；
- 进入同一个 Task 和同一个具体 coroutine frame 中的下一层 region；
- 不创建第二个 Task、第二个 frame 或 Task::<Task::<T>>；
- 不是普通 await child task；
- 不允许并发进入、保存、返回、闭包捕获或交给调度器。

decorator.region 在每次进入时必须重新建立其局部初始化状态。跨 suspend 仍存活的 decorator locals 与原 body locals 共同参与同一个 frame layout。

decorator continuation 必须在 ABI lowering 前消除为普通 CFG、内部 version-local helper 或融合后的 coroutine state machine。Closed InkIR 不得把它物化成可逃逸运行时 callable。

## 14. code-only hot reload

### 14.1 v0 范围

Ink v0 只支持同 ABI、同布局的 code-only hot reload。

允许更新：

- 普通函数实现；
- virtual 或 interface slot 的实现代码；
- reflection adapter 的实现代码；
- 完整展开后的同步或异步 decorator 实现；
- 不改变签名和布局的 module version 内部代码。

不允许原位兼容更新：

- class 或 enum layout；
- base class chain；
- vptr 位置；
- virtual slot 集、顺序、签名或 sync/async kind；
- interface parent set、有效 slot 集、顺序、签名或 ancestor conversion set；
- 函数参数、parameter mode、receiver、calling convention 或逻辑结果；
- Task logical result；
- reflection function kind 或动态调用签名；
- C ABI 导出签名。

这些变化必须拒绝本次热更新，或者作为未来独立的迁移协议建立新类型和新 ABI。v0 不提供普通对象布局迁移。

### 14.2 stable entry

热更新模式中的逻辑函数入口使用 stable entry：

逻辑入口与物理实现是两条不同identity。logical stable symbol具有bodyless `FunctionRecordKind=declaration, EntryIdentity=stable` Function和同symbol的`StableEntry` RuntimeDeclaration；后者把`LogicalSymbolId`映射到当前artifact中唯一的`VersionLocalSymbolId`。物理symbol固定为generated `version_local_body` definition，其parent为`versioned_owner(LogicalStableSymbol, ModuleVersionContextKey)`；`ModuleVersionContextKey`必须由当前Manifest及冻结semantic输入重算。语义context不同的module version即使源码逻辑symbol与ABI不变，也具有不同version-local SymbolKey；仅CompilerBuildId、schema/container或pass pipeline等K-only输入不同的artifact variant复用该语义SymbolKey，但其`ArtifactVariantKey`与`LoadedVersionKey`不同。runtime version owner、物理mangling与pin以LoadedVersionKey区分，因此两类旧/new variant都可并存；active dependency只能导入bodyless logical projection，不能复制provider的mapping或物理body。

~~~text
stable entry
    → atomically select current compatible version
    → acquire version pin
    → enter version-local implementation
    → release pin after normal or unwind completion
~~~

每个 stable entry 必须携带独立于当前实现体的 StableEffectEnvelope：

~~~text
EffectUpperBound {
    MemoryAccess = None | ReadAny | ReadWriteAny
    RuntimeEffects = FiniteSet<handler> | Any
    AllocateKinds = FiniteSet<storage-kind> | Any
    DeallocateKinds = FiniteSet<storage-kind> | Any
    MayTrap
    MayDiverge
    TargetDependent
    PdbBoundary
}

StableEffectEnvelope {
    SyncEffects: EffectUpperBound
    AsyncConstructionEffects: EffectUpperBound
    AsyncBodyEffects: EffectUpperBound
    SyncMayUnwind
    ConstructionMayUnwind
    BodyMayFail
}
~~~

EffectUpperBound 是对可替换版本的逐字段上界，不是当前 body summary 的缓存。其规范偏序逐字段定义为：None < ReadAny < ReadWriteAny；FiniteSet 集合按子集排序且任意 FiniteSet < Any；所有布尔字段均为 false < true。精确 body summary 规范化时，无memory effect映射为None，仅有read映射为ReadAny，出现任意write映射为ReadWriteAny；runtime handler、allocate kind和deallocate kind分别形成去重并按中央tag升序排列的FiniteSet。两个upper bound的关系是所有字段同时不大于；三个phase summary只有分别满足该关系才是envelope的子集。

v0 没有源码可公开承诺的 pure/read-only effect contract，因此每个相关 phase 的默认 envelope 是 MemoryAccess = ReadWriteAny、RuntimeEffects = Any、AllocateKinds = Any、DeallocateKinds = Any、MayTrap = true、MayDiverge = true、TargetDependent = true、PdbBoundary = true。MayDiverge 表示调用可能没有 normal、unwind 或 trap completion，必须加入中央 effect registry；不能用“当前实现看起来会终止”消去。对sync entry，AsyncConstructionEffects和AsyncBodyEffects必须规范编码为全bottom，ConstructionMayUnwind和BodyMayFail为false；对async entry，SyncEffects必须为全bottom且SyncMayUnwind为false。全bottom是MemoryAccess = None、三个FiniteSet为空、四个effect布尔均为false；无关phase不得填top或复制另一phase以制造多个等价hash。

异常通道不使用任意顶值。首次建立兼容基线时，sync entry由当前正向nothrow bit初始化`SyncMayUnwind = !nothrow`；async entry分别初始化`ConstructionMayUnwind = !construction_nothrow`与`BodyMayFail = !body_no_fail`。这只是一次性建立固定upper-bound baseline，不产生、保存或digest任何反相的current contract字段。此后code-only update固定保留三个baseline bits及envelope hash；当前BehaviorContracts按14.4节只可设置更多正向保证而不反向改写baseline。每个版本的精确sync、construction和body异常summary都必须不超过相应baseline，调用点实际允许的异常通道分别是`SyncMayUnwind && !nothrow`、`ConstructionMayUnwind && !construction_nothrow`和`BodyMayFail && !body_no_fail`。async construction call只应用AsyncConstructionEffects，创建后的Task/runtime body才应用AsyncBodyEffects；不得把lazy body effect提前归到任务构造时，也不得因当前body较弱而缩小stable call effect。

每次进入 stable entry 都执行一个原子“选择当前 compatible version + acquire pin”语义动作，并固有 RuntimeEffect(version_select)。该 effect 即使 StableEffectEnvelope 未来允许 pure 也不可消去、不可 CSE、不可推测、不可复制，且不得越过 publish、pin lifetime、异常边界或 Task ownership transfer hoist/sink。sync stable call 的有效效果是SyncEffects、在`SyncMayUnwind && !nothrow`时加入的MayUnwind以及RuntimeEffect(version_select)的并集；async stable construction同理使用AsyncConstructionEffects、在`ConstructionMayUnwind && !construction_nothrow`时加入的MayUnwind以及RuntimeEffect(version_select)。Task body是否可能failed由`BodyMayFail && !body_no_fail`决定。version-local call只有在verifier已证明持有匹配pin时才使用所选body的精确effect summary，并且不再产生version_select。

只有全程序/装载契约证明该 stable entry 禁用热更新，或证明同一版本在选择前到调用完整 dynamic extent 结束期间被冻结且 pin 覆盖所有 version-local 地址时，optimizer 才能消除 version_select、绑定 version-local target 并依据精确 body summary 内联。单次读取当前 target、profile 单态、当前只有一个版本或 envelope 看似 pure 都不是证明。

取得函数地址时取得 stable entry，不取得裸 version-local body 地址。源码中的普通递归调用同样经过 stable entry。decorator continuation 直接进入下一层 continuation_local region，不能经 stable entry 递归回当前 decorator。

写入 live object vptr 或普通 interface reference 的 table 地址也不得在兼容更新发布后立刻变成悬空 version-local 地址。运行时必须使用常驻稳定 table、稳定 table cell 或语义等价的受保护间接层；code-only publish 只原子替换兼容 slot target。已经由 Task 或 pinned snapshot 明确保存旧 version-local table 的情况除外，这些 owner 必须持有旧版本 pin。

async stable entry 选择并固定 task-construction thunk。构造成功时 pin 转移给 Task；构造失败时立即释放。Task 持有其 frame、resume、destroy、exception metadata 和必要 table pins，直到 Task 完全销毁。

### 14.3 version-local entry

version-local entry 只允许：

- 已经持有兼容 version pin 的内部调用；
- compiler-generated thunk、cleanup、destroy、resume 和 continuation helper；
- pinned reflection snapshot 对对应版本 adapter 的调用；
- TargetABI 能证明不改变“新逻辑调用进入当前版本”语义的内部优化。

每个stable mapping都必须逐项核对logical declaration、version-local definition和`StableEntry` record的FunctionType、TargetAbiTag、FunctionAbiDigest与BehaviorContractDigest；只有logical declaration携带StableEffectEnvelope和公开default arguments，version-local definition只携带当前body。version-local symbol的`ModuleVersionContextKey`不允许由裸content hash、当前target地址或loader ordinal替代。

version-local entry 不得：

- 保存进普通函数值并逃出版本；
- 写入 module registration record 作为逻辑 function.entry；
- 作为跨模块公开地址；
- 在没有 pin 的路径上执行；
- 被优化器当作 stable entry 的永久替代。

### 14.4 ABI 和行为兼容检查

每个热更新入口必须具有 ABI compatibility data，至少覆盖：

- target triple 和 TargetABI version；
- function kind；
- receiver kind；
- ordered parameter types 和 passing modes；
- logical result type与result_passing_mode；
- destination presence、ConstructedTypeId和DestinationRoleTag；constructor仍为void/void且role为initializing_receiver；
- calling convention；
- class、enum、vtable 和 interface layout hashes；
- reflection adapter kind；
- async Task ABI tag，以及由TargetContext和RuntimeAbiRevision确定的RuntimeStorageAbiHash；
- registration encoding revision 与 `ModuleRegistrationInterfaceDigest`；该 digest 只覆盖从当前 records 提取并去重排序的 `(RegistrationTypeSemanticIdentity, ProtocolSchemaDigest, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)` schema tuple集合，不覆盖record identity、数量或值；
- 当前正向BehaviorContracts bitset（nothrow、construction_nothrow、body_no_fail、hot_reload_stable、extern_no_unwind）及BehaviorContractDigest；
- 完整 StableEffectEnvelope 的 canonical encoding 与 StableEffectEnvelopeHash。

StableEffectEnvelopeHash 使用规范字段顺序和中央 effect/handler/storage-kind tag 计算，不包含当前 body summary、当前可单调加强的行为契约、地址、表索引或 hash-table 顺序。code-only update必须保持既有envelope与hash不变；新sync body、async construction thunk和async body的精确summary必须分别是SyncEffects、AsyncConstructionEffects和AsyncBodyEffects的逐字段子集，且不得超过三个异常baseline bits或当前更强行为契约。想缩小或扩大公开effect envelope都需要重新编译依赖者或未来显式协议，不能伪装成同一compatibility record；单调加强nothrow/no-fail只更新独立BehaviorContracts并由后续版本继续保持，不改变envelope hash。

BehaviorContracts使用正向bit的单调契约检查：

~~~text
old sync nothrow == true               -> new sync nothrow必须为true
old sync nothrow == false              -> new可以设置nothrow
old construction_nothrow == true       -> new construction_nothrow必须为true
old construction_nothrow == false      -> new可以设置construction_nothrow
old body_no_fail == true               -> new body_no_fail必须为true
old body_no_fail == false              -> new可以设置body_no_fail
hot_reload_stable                      -> 当且仅当EntryIdentityTag=stable
extern_no_unwind                       -> 只允许EntryIdentityTag=extern；若设置则sync nothrow也必须设置
~~~

BehaviorContracts以中央BehaviorContractBit位置编码。sync callable只使用nothrow，async callable只使用construction_nothrow与body_no_fail；hot_reload_stable当且仅当EntryIdentityTag=stable；extern_no_unwind只允许EntryIdentityTag=extern且设置时同时满足sync nothrow；所有不适用bit必须为零。`BehaviorContractDigest = H("ink.behavior-contract.v0", CanonicalLineageIdentity, BehaviorContracts)`，不编码反相current bool。每个发布候选都携带stable entry compatibility lineage identity、固定StableEffectEnvelopeHash、当前BehaviorContracts和digest。loader在原子publish前必须以该lineage当前已发布版本为predecessor验证上述适用矩阵及单调关系并重算digest，不能只信任候选自报的previous hash；成功发布后新的current contract成为以后更新不得减弱的基线。

BehaviorContractDigest和完整当前bitset必须进入导入依赖记录、增量编译/cache key、AOT call-site assumption以及module publish record。依赖加强后nothrow/no-fail证明的新调用者不能命中只记录旧宽松契约的cache artifact；loader也不得接受未来重新减弱契约但仍复用固定effect-envelope hash的版本。旧依赖者继续按其较宽契约有效。

清除既有nothrow、construction_nothrow、body_no_fail、hot_reload_stable或合法extern_no_unwind保证都不是兼容code-only更新。特别地，已设置construction_nothrow的async construction不得重新同步展开，已设置body_no_fail的Task body不得开始发布failed。

v0 code-only update要求 `ModuleRegistrationInterfaceDigest` 精确相等，因此不能在同一lineage内悄悄改变某种registration frozen encoding、type layout、protocol schema或Runtime ABI。`ModuleRegistrationSetDigest`覆盖每条canonical ordered record的完整S projection，包括identity、producer/application/order path、callsite/dynamic path/ordinal、value和protocol字段，并允许变化；在当前 records 所代表的 unique schema set 不变时增加、删除或修改具体record由原子publish事务表达，不属于ABI/layout变化。增加某 schema 的第一条 record 或删除其最后一条会改变 interface digest，v0 不接受为 code-only update。

### 14.5 module 事务与全局状态

新版本必须先完整准备：

1. 初始化 V2 globals；
2. 按中央 `StaticRegistrationEncodable` schema构造readonly frozen typed registration records和受控relocation；
3. 重算并验证ABI、所有runtime descriptors、registration interface/set digest、固定StableEffectEnvelopeHash以及当前lineage的BehaviorContractDigest单调性；
4. 原子发布 stable entries、vtable/interface entries、reflection snapshots 和 registration records。

V2 global initialization 或验证失败时，loader 必须逆序清理已经建立的 V2 状态并放弃整次事务；V1 仍是唯一已发布版本，任何调用、查询或 table load 都不得观察到部分 V2。

registration descriptor安装、放弃和撤销都不调用用户constructor、destructor、rollback或任意module hook。frozen String等中央encoding由module-owned readonly backing覆盖；准备失败时只丢弃未发布descriptor/backing。每个已发布record由不可伪造的准确module-version owner管理。

新调用进入 V2。已经进入 V1 的调用、V1 Tasks、V1 ExceptionRecords/Boxes 和 V1 reflection snapshots 继续使用 V1。V1 globals的initializer/finalizer、AlivePrefix与逆序cleanup同样由V1 module-version owner锁定；publish current mapping只影响新的logical stable入口选择，不得重定向已激活lifecycle helper。

旧版本排空后：

1. 以 V1 version owner + RegistrationIdentity 撤销 V1 records，绝不只按业务逻辑键删除；
2. 逆序销毁 V1 globals；
3. 释放 V1 code、data 和 descriptors。

v0 不迁移或复用 V1 global object state。需要业务状态迁移时必须由正常应用协议显式完成。

### 14.6 raw borrow 与卸载前置条件

普通class reference和两字interface reference是携带ObjectLifetimeGeneration的checked non-owning borrow；裸指针是完全不携带generation的raw non-owning address。两类都不自行取得module pin，checked object generation也不能充当code-version pin。运行时不能为任意复制的borrow隐藏引用计数。

动态卸载前，loader 或 owner 必须保证这些 raw borrows 不再可能被使用。跨卸载使用悬空 raw borrow 违反其生命周期前置条件。

ABI/layout-compatible 的普通对象不因为由 V1 constructor 建立就固定 V1 代码。其 live vptr/interface table 必须指向常驻 stable compatible table/cell；后续 virtual/interface call 和 obj.destroy_dynamic 通过 stable entry 选择当时的 current compatible version。对象内不得保存 V1 version-local destructor、slot 或 table 裸地址。因而“对象仍存活”本身不是 V1 code pin，也不能被 loader 当作扫描所有对象的隐式 pin 协议。

以下对象必须持有显式内部 pin：

- 正在执行的 stable-entry call；
- created、pending 或尚未销毁的 Task；
- 活动 ExceptionRecord 和 ExceptionBox；
- pinned reflection snapshot；
- 保存旧 table 或version-local entry的async/default-method/runtime frame。

只有上述 Task/frame、active call/EH、snapshot 或显式持有 version-local entry/table 的内部 owner 才固定对应版本。任何 version-local raw address 一旦脱离这些 owner/pin 的动态范围都不安全，不能存入普通对象、普通函数值或长期缓存。V1 module globals 仍是 V1 的版本化状态：它们在 V1 owner/pin 排空前保持，随后按 14.5 节逆序销毁；普通对象不 pin V1 不会把 V1 globals 改成跨版本共享状态。

pin 的物理算法属于私有 runtime ABI。

## 15. C ABI 边界

### 15.1 显式边界

只有显式 extern C 或等价声明使用 C ABI。普通 Ink function、virtual slot、interface slot、Task constructor、reflection adapter 和 stable entry 都不是 C ABI。

EntryIdentityTag `extern`只表示链接期解析的Ink logical ABI declaration，仍由call.*调用。C linker import不创建Function/EntryIdentity，而由`RuntimeRecordKind=AbiImport`唯一表示；它和compiler-generated AbiBridge的唯一调用表示是abi.call/abi.invoke，C导出由AbiExport表示。不能因目标符号恰由同一linker解析就把两类extern合并。

extern C 的逻辑签名必须在语义分析时完全闭合，并只使用当前目标明确支持的 C ABI-safe 类型。v0 至少禁止直接跨 C ABI 传递：

- 默认布局 class；
- 带 vptr class；
- 默认布局带载荷 enum；
- interface 引用；
- Task；
- ExceptionRecord、ExceptionBox 和 ExceptionView；
- DynamicRef、DynamicOut 和 reflection snapshot；
- 非稳定布局 tuple、slice 和其他 Ink 聚合；
- 需要 Ink destructor 或 module pin 的值。

具有明确 C 表示的 repr 类型只有在其独立规范和 TargetABI mapping 已确认时才允许。类型定义首先满足与调用位置无关的封闭谓词`CRepresentationSafe(T, TargetAbiTag, TargetKey)`：T具有当前TargetABI唯一注册的C representation；aggregate必须显式C representation、无Ink destructor/vptr/NoEscape/部分初始化语义，全部可达field递归representation-safe，且size/alignment/offset/tag与LayoutDigest逐字段匹配。repr(C)定义本身没有parameter/result root，不能为通过类型验证而伪造一个`CAbiPositionPayload`。

实际C-facing位置再验证四参谓词`CAbiSafe(T, TargetAbiTag, TargetKey, CAbiPositionPayload)`。它先要求`CRepresentationSafe`，再要求TargetABI为准确root与递归field path注册唯一classification；`CAbiPositionPayload`完整编码`Root, RootOrdinal, AggregateFieldPath`，parameter、status-out、callback与result-out parameter携带zero-based ordinal，result/status-result的ordinal固定0，递归aggregate field按source ordinal追加path：

- `void`只允许作为result；`never`、unit、reference、safe slice、interface reference、Ink function value、runtime object/handle、exception、meta/dependent及任何no-escape/linear capability均不允许；
- 固定位宽整数、浮点、C bool、enum和raw pointer只有在TargetAbiTag为其指定唯一C类型、位宽、alignment、该准确position classification及pointer address space时允许；C function pointer的pointee signature必须以各自准确position递归验证且calling convention为c；
- C aggregate先满足`CRepresentationSafe`，再对当前root及每个`AggregateFieldPath`满足唯一parameter/result/status/callback classification；opaque C handle只表示为明确address-space的raw pointer，不能伪装成Ink owner；
- 所有通过类型、bridge declaration或alias到达的表示必须归一到当前TargetABI注册的唯一representation/classification与layout digest；缺少mapping、mapping歧义、varargs或无法由当前TargetKey验证时结果为false。

`CFunctionType`不是任意Ink function signature加上`calling_convention=c`，而只有一个canonical逻辑矩阵：ordinary callable（`CallableKind=function`）、`FunctionKind=sync`、`ReceiverKind=none`、ReceiverType absent、`CallingConvention=c`；每个parameter都固定`ParameterPassingMode=value`并以其zero-based parameter position验证四参`CAbiSafe`，因此通过验证的repr(C) aggregate可以作为逻辑value参数。逻辑result为`void`时`ResultPassingMode=void`；其他获准C类型一律`ResultPassingMode=value`并在result position验证`CAbiSafe`，即使TargetABI最终用物理sret/indirect return也不得把逻辑模式改成`result_destination`。`never` result、`result_destination`、parameter mode `object|const_reference|mutable_reference|raw_pointer`及varargs都拒绝；raw pointer类型本身仍可作为mode=`value`的C参数或结果，不能把类型与passing-mode枚举混同。status、callback与result-out等附加C channel也只能是该矩阵中的普通value parameter/result，再由唯一`CAbiPositionPayload`指出，不能产生隐藏参数或第二种signature形态。

该注册mapping由TargetAbiTag及AbiImport/AbiBridge/AbiExport的ABI digest提交；record、call site与backend必须使用同一mapping，backend不得重新猜测宿主编译器布局。

### 15.2 结果与参数桥接

C ABI 的小聚合寄存器返回、物理sret、bool 表示、枚举宽度和结构体对齐由目标平台 ABI 决定，不使用本文的默认 Ink 物理调用约定猜测；varargs在v0进入TargetABI分类前已经拒绝。

canonical InkIR只保存`AbiImport`、`AbiBridge`、`AbiExport`三类declarative RuntimeDeclaration及双signature、`BoundarySignatureMapPayload`和policy，不生成第二个Core Function thunk。map逐项给出每个Ink value parameter对应的C parameter ordinal及唯一`identity_c_abi` conversion，并显式给出optional logical-result channel；status/result out parameter分别必须是`pointer<rw,StatusType>`与`pointer<rw,ResultType>`。删除这些special channels后，C parameter序列必须与无receiver的sync ordinary Ink value parameters一一同序、类型相等且无剩余位置。`FunctionRecordKind=reserved_extern_bridge`在所有artifact中拒绝，任何Function record也不得使用`calling_convention=c`；backend才从已验证record直接合成不可被Core调用、取址、反射或序列化为Function entity的物理wrapper/thunk：

- Ink 侧继续遵守 scalar SSA 和 caller destination 语义；
- backend-synthesized bridge 把 Ink 参数转换为准确 C 物理签名；
- C 结果先按 C ABI 接收，再在不引入非法复制或析构的前提下初始化 Ink 结果；
- 不兼容或 noncopyable 类型不得通过自动 bridge 强行传递。

在 C varargs 的精确 promotion 和类型约束另行确认前，Ink v0 不生成可变参数 C 调用。

### 15.3 异常边界

Ink 异常不得展开穿过不理解 Ink EH ABI 的 C frame。

导出给 C 的 wrapper 必须：

- 使用`ExportExceptionPolicyPayload=nothrow`且Ink target具有positive nothrow；或
- 在 wrapper 内 catch-all，按`ExportExceptionPolicyPayload=catch_and_status|catch_and_callback`转换为唯一status channel或已注册callback protocol。

`catch_and_callback`的`CallbackFunctionType`必须是满足上述唯一CFunctionType矩阵的准确`(pointer<ro,ExceptionInfoType>) -> void`，`CallbackSymbol`只能解析到`CFunctionTypeId`逐项相同且`ForeignFailurePolicyPayload=nothrow`的AbiImport。被export的Ink target与C outer signature都必须logical result=void且BoundarySignatureMap.LogicalResultChannel absent。普通Ink Function、logical stable symbol、StableEntry及`EntryIdentity=extern`的Ink declaration都不能直接填入该C callback字段；若Ink callable需要参与callback协议，必须另经`ExternalTarget`绑定该AbiImport且双signature、BoundarySignatureMap、TargetAbiTag、foreign policy均逐项匹配的已验证InkToExternal bridge调用它，不能把Ink地址当成C callback地址。

如果异常仍越过声明为 C ABI 或 nothrow 的边界，运行时立即 fatal，不继续向外展开。

C 或其他外部异常也不会自动转换成 Ink exception。Ink到外部调用只按独立`ForeignFailurePolicyPayload`处理：`nothrow`违反时fatal，`status_to_exception`读取注册status channel并由nothrow exception factory构造准确Ink异常；raw C++/SEH exception永远不能直接成为Ink token。`TaggedBoundaryPolicyPayload`只存在于AbiBridge，并由direction选择foreign-failure或export-exception payload，两个policy family不得混用。

外部线程回调 Ink 时，bridge 必须建立普通 Ink thread fatal boundary，或者在返回 C 前捕获并转换全部 Ink exceptions。

### 15.4 hot reload 导出

需要 hot reload 的 C 导出符号必须保持稳定地址，并由稳定 C bridge 进入 Ink stable entry。C 调用者不能保存 version-local Ink body 地址。

C 签名或 C 表示变化不是 code-only compatible update。

## 16. v0 runtime opcode schema

本节是 06 所有 operation 的 v0 canonical registry。call.*、obj.*、eh.*、cf.* 和普通 memory/value operation 的基础 schema 由核心指令规范注册，本节只增加 runtime/dispatch 边界及对 call schema 的必要约束。表中每个 opcode 都有稳定 OpcodeTag，shape、operand/result type、attribute enum、effect、stage、successor kind 和 postcondition 都属于 schema；不能用自由 attribute 增删 operand、结果、effect 或 successor。S 表示只存在于 StagedModule 且 ClosedModule 前必须消失，SC 表示 Staged/Closed 都允许。MayDiverge 是“动态执行可能永不产生 normal、unwind 或 trap completion”的 effect，必须与本节同时加入中央 effect registry。

所有address-only destination shape都编码5.1节DestinationPayload。表内写destination_role = result的操作不得改成initializing_receiver。其normal completion只建立Alive state proof，不产生destination edge value；normal-dominated use必须以place.as_alive物化可访问capability。rollback后原init capability在旧transaction消费且cleanup完成后可fresh begin；place.as_uninitialized只用于destroy后的ro/rw capability，并要求严格证明无active object/transaction/child lifetime。两种CapabilityRebind消费PlaceCapabilityGeneration、产生next generation并使旧root/descendant views失效；它们无runtime memory effect但不可复制、CSE或推测。普通call.*仍由核心registry编码；其额外约束是calling_convention必须为ink、constructor destination_role必须为initializing_receiver、stable target使用StableEffectEnvelope + RuntimeEffect(version_select)，pinned version_local target才使用精确body summary。

### 16.1 cast schema

| Opcode | 精确 shape | Effects | 阶段 | terminator 与关键前置/后置条件 |
| --- | --- | --- | --- | --- |
| cast.class_up | %base = cast.class_up %derived {base = @Derived::<base-selector>} : ref<A,Derived> -> ref<A,Base> | Pure, TargetDependent | SC | 非 terminator；payload是Derived定义中唯一`SymbolKind=base` selector而非Base type symbol；single concrete base path唯一且已声明；source generation匹配Alive complete object；结果保持access、non-owning语义和同一ObjectLifetimeGeneration，不做dynamic check |
| cast.interface_make | cast.interface_make %object {interface = @I} to %dst {destination_role = result} : ref<A,C> -> InterfaceRef<A,I> | ReadMemory(runtime_metadata), WriteMemory(typed), BeginLifetime, TargetDependent, RuntimeEffect(interface_lookup) | SC | 非 terminator；C实现I且source generation有效；dst是匹配active owner transaction；直接建立object/table pair，descriptor保留同一ObjectLifetimeGeneration与non-owning referent语义，并由schema normal postcondition唯一commit |
| cast.interface_up | cast.interface_up %source {ancestor = @Parent} to %dst {destination_role = result} : !place<ro/rw,InterfaceRef<A,Child>> -> InterfaceRef<A,Parent> | ReadMemory(typed), ReadMemory(runtime_metadata), WriteMemory(typed), BeginLifetime, TargetDependent | SC | 非 terminator；ancestor conversion已注册且source descriptor generation仍有效；只替换table，不复制实现对象；结果保留同一generation/non-owning语义并唯一commit |
| cast.try_class | scalar variant: %raw = cast.try_class %source {target = @D, result_form = raw_pointer} : SourceView<A> -> ptr<A,D,S>; object variant: cast.try_class %source {target = @D, result_form = optional_reference} to %dst {destination_role = result} : SourceView<A> -> OptionalRef<A,D> | ReadMemory(runtime_metadata), RuntimeEffect(dynamic_cast), TargetDependent；object variant 再加 WriteMemory(typed), BeginLifetime | SC | 非 terminator且nothrow；source generation必须匹配Alive complete object；raw-pointer失败返回null且成功显式丢弃checked generation；Optional<ref>始终在dst构造Some/None并唯一commit，Some保留源generation/non-owning语义，禁止因niche物理表示改成SSA |
| cast.try_interface | cast.try_interface %source {target = @I} to %dst {destination_role = result} : SourceView<A> -> OptionalInterfaceRef<A,I> | ReadMemory(runtime_metadata), WriteMemory(typed), BeginLifetime, RuntimeEffect(dynamic_cast), TargetDependent | SC | 非 terminator且nothrow；source generation必须匹配Alive complete object；结果始终是address-only Optional/interface object，Some保留源generation/non-owning语义，None不携带active borrow generation；构造后唯一commit，禁止两字descriptor或niche SSA |

InterfaceRef<A,I>、OptionalRef<A,D>、OptionalInterfaceRef<A,I>和SourceView<A>是本表的schema metavariable，不是canonical text type spelling；实际text引用对应!tN，binary编码准确TypeId。前三者必须分别指向普通可复制address-only interface-reference/Optional类型，不得指向runtime.object。SourceView<A>的operand type必须精确为ref<A,C>或!place<ro/rw,InterfaceRef<A,I>>，schema按该TypeId选择已注册class/interface关系；raw ptr、整数和无lifetime证明的地址都不匹配SourceView。

### 16.2 reflection schema

lookup 的 found edge 原子取得所选版本 pin并产生一个线性 owned snapshot；missing edge不产生 snapshot。该 select+pin 固有 RuntimeEffect(version_select)，与 stable call 一样不可消去、CSE、推测或跨 publish 移动。result(0) 和 exception 继续使用统一 schema-produced edge encoding。

| Opcode | 精确 shape | Effects | 阶段 | terminator 与关键前置/后置条件 |
| --- | --- | --- | --- | --- |
| reflect.lookup_type | reflect.lookup_type %qualified_name {module = "canonical.module"} found ^found(result(0)) missing ^missing : !place<ro,String> -> !runtime-handle<type_snapshot> | ReadMemory(runtime_registry), RuntimeEffect(reflection_lookup), RuntimeEffect(version_select), Control | SC | terminator；P22 module是`CanonicalModuleIdentity : Str`而非SymbolId；按(module, LookupName, type)唯一查找；零项missing，一项found并产生owned linear snapshot，多项是artifact错误；模块与名称按canonical UTF-8/NFC identity比较 |
| reflect.lookup_interface | reflect.lookup_interface %qualified_name {module = "canonical.module"} found ^found(result(0)) missing ^missing : !place<ro,String> -> !runtime-handle<interface_snapshot> | ReadMemory(runtime_registry), RuntimeEffect(reflection_lookup), RuntimeEffect(version_select), Control | SC | terminator；P22 canonical module string规则同lookup_type，descriptor kind必须为interface且同样要求唯一候选 |
| reflect.lookup_function | reflect.lookup_function %qualified_name {module = "canonical.module", expected_function_type = !tN} found ^found(result(0)) missing ^missing : !place<ro,String> -> !runtime-handle<function_snapshot> | ReadMemory(runtime_registry), RuntimeEffect(reflection_lookup), RuntimeEffect(version_select), Control | SC | terminator；P23首先编码`CanonicalModuleIdentity : Str`再编码准确FunctionTypeId；按(module, LookupName, function, ExpectedFunctionTypeSemanticIdentity)唯一查找LookupOwner absent descriptor；found snapshot固定精确function kind/signature/adapter/version，不做默认实参或返回类型重载选择 |
| reflect.lookup_member | reflect.lookup_member %owner, %name {member_kind = K} found ^found(result(0)) missing ^missing : (!runtime-handle<type_or_interface_snapshot>, !place<ro,String>) -> !runtime-handle<member_view> | ReadMemory(reflection_snapshot), RuntimeEffect(reflection_lookup), Control | SC | terminator；按(owner snapshot SubjectSymbol, LookupName, K)唯一查找LookupOwner匹配的descriptor；零项missing，一项found，多项使artifact非法；v1同名同kind重载拒绝；result是不可存储的borrowed view，不增加pin，owner必须支配view的全部uses和eventual control merge |
| reflect.snapshot.clone | %clone = reflect.snapshot.clone %owner : !runtime-handle<K_snapshot> -> !runtime-handle<K_snapshot> | ReadMemory(runtime_registry), WriteMemory(runtime_registry), RuntimeEffect(version_pin) | SC | 非 terminator；显式产生第二个 linear owner；不能由 Copy/memcpy 替代 |
| reflect.snapshot.release | reflect.snapshot.release %owner : !runtime-handle<K_snapshot> -> () | ReadMemory(runtime_registry), WriteMemory(runtime_registry), RuntimeEffect(version_pin) | SC | 非 terminator且 nothrow；消费一个 owner；所有 borrowed views 已结束 |
| reflect.call | void/scalar: `[ %r = ] reflect.call %function_snapshot receiver(none\|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true}`，方括号内argument list整体可空；address: 同形后接 `to %dst {destination_role = result}` | pinned version-local adapter exact summary, RuntimeEffect(reflection_dispatch)；address variant 含 WriteMemory(typed), BeginLifetime | SC | 非 terminator；receiver/arguments是expected function type规定的typed SSA/handle/place，不存在Core DynamicRef value；ReceiverPresent与receiver spelling一致，ArgumentCount与args数一致，result/destination形态与result mode匹配；必须同时证明target_nothrow与adapter_nothrow；每个private descriptor沿用operand的有效generation/non-owning fact且只能形成borrow authority；snapshot/arguments仍由caller拥有；address result由最终adapter commit。任一nothrow证明缺失时唯一形式是核心call.invoke callee_kind = reflection及normal/unwind successors |

v0 不注册 reflect.snapshot.acquire；lookup found transition已经完成 acquire。snapshot 是线性 runtime handle，不是 address-only Copy 对象，因而也不注册 snapshot load/store/memcpy opcode。

### 16.3 async、Task 与 cancellation schema

async callee payload 由 callee_kind 确定：direct/indirect 使用 symbol或function value，virtual 使用 receiver+slot，interface 使用 interface place+slot，reflection 使用 function snapshot；binary 使用完整 P26 `AsyncCall`/`CallPayload`，其中 variant payload、`FunctionTypeId`、`ExplicitArgumentCount` 和 required `DestinationPayload` 唯一确定各 operand segment，不另编码可漂移的 `operand_segment_sizes`。

async.call/async.invoke由normal schema postcondition执行Task-specific commit并为!runtime-object<task,T>建立fresh ObjectLifetimeGeneration；不得对Task发出generic obj.init.commit。construction同时为该Task generation建立verifier-only `TaskBodyEffect`和`TaskDestroySummary`，consumer从operand provenance推导而不编码自由summary payload；无法证明来源时使用最大保守上界。下列所有Task place operand都要求其generation仍匹配当前Alive Task；async.task.destroy额外要求owner LifetimeAuthority并终结generation，await/drive/cancel所需rw access不能把borrow authority升级为lifecycle owner。

| Opcode | 精确 shape | Effects | 阶段 | terminator 与关键前置/后置条件 |
| --- | --- | --- | --- | --- |
| async.call | async.call callee_kind = K Callee(%args...) to %task_dst {destination_role = result} : Signature -> !runtime-object<task,T> | AsyncConstructionSummary, Allocate(task_frame), WriteMemory(typed), BeginLifetime, RuntimeEffect(task_construct)；stable target 再并入 RuntimeEffect(version_select) 与 StableEffectEnvelope.AsyncConstructionEffects | SC | 非 terminator；仅effective construction_nothrow；dst active且constructed type为!runtime-object<task,T>；成功直接构造created Task、建立TaskBodyEffect/TaskDestroySummary side facts、由schema原子执行Task-specific commit并转移pin；禁止generic obj.init.commit |
| async.invoke | async.invoke callee_kind = K Callee(%args...) to %task_dst {destination_role = result} normal ^created unwind ^failed(exception) : Signature -> !runtime-object<task,T> | async.call effects, MayUnwind, Control | SC | terminator；恰有normal/unwind；只表示同步construction failure，body failure不走此edge；unwind前callee cleanup并满足destination rollback postcondition |
| async.await | value/unit: async.await %task normal ^success(result(0)) unwind ^failure(exception) : !place<rw,!runtime-object<task,T>> -> T; void/never: async.await %task normal ^success unwind ^failure(exception) : !place<rw,!runtime-object<task,T>> -> () | TaskBodyEffect, ReadMemory(task_state), WriteMemory(task_state), Allocate(exception), RuntimeEffect(async_suspend), MayUnwind, MayDiverge, Control | SC | terminator；恰有normal/unwind；never normal block只能不可达终止；created可驱动body，pending不选successor而登记continuation；只允许suspendable activation且全部跨暂停live state必须frame-spill safe |
| async.await_copy | async.await_copy %task to %dst {destination_role = result} normal ^success unwind ^failure(exception) : (!place<rw,!runtime-object<task,T>>) -> T | TaskBodyEffect, ReadMemory(task_state), ReadMemory(task_result), WriteMemory(task_state), WriteMemory(typed), BeginLifetime, Allocate(exception), RuntimeEffect(async_suspend), MayUnwind, MayDiverge, Control | SC | terminator；T address-only、Copyable且CopyConstructionEnabled；dst只由DestinationPayload编码，不进入OperandValueIds/ResultCount，箭头T是constructed channel type；dst active并可安全跨暂停；Task succeeded时按obj.init.copy的完整descendant-generation规则typed Copy后schema原子唯一commit；Task failed是唯一unwind/failure，Copy不展开且failure postcondition rollback |
| async.task.drive_once | async.task.drive_once %task : !place<rw,!runtime-object<task,T>> -> () | TaskBodyEffect, ReadMemory(task_state), WriteMemory(task_state), RuntimeEffect(task_drive), MayTrap, MayDiverge | SC | 非 terminator；仅compiler/runtime generated；entry state必须created，原子转pending并执行到suspend/final；body异常捕获为failed，不向caller unwind |
| async.task.publish_success | void: async.task.publish_success %task {result_mode = void} : !place<rw,!runtime-object<task,void>> -> (); value: async.task.publish_success %task, %value {result_mode = value} : (!place<rw,!runtime-object<task,T>>, T) -> (); object: async.task.publish_success %task, %result {result_mode = result_destination} : (!place<rw,!runtime-object<task,T>>, !place<ro,T>) -> () | all: ReadMemory(task_state), WriteMemory(task_state), RuntimeEffect(task_publish), Control；非unit value另有WriteMemory(task_result) | SC | terminator且无CFG successor；`%task`必须是本body的borrowed `task_self` 或 schema-preserving phi；T = unit使用携带唯一semantic unit value但没有物理写入的value variant；object `%result`必须由本body唯一 `task_result_storage` 经 begin/commit/as_alive 得到，已Alive且不是待复制源对象；原子pending->succeeded并唤醒waiters |
| async.task.publish_failure | async.task.publish_failure %task, %active : (!place<rw,!runtime-object<task,T>>, !exception) -> () | ReadMemory(task_state), Allocate(exception_box), WriteMemory(task_state), RuntimeEffect(task_publish), Control | SC | terminator且无CFG successor；`%task`必须来自本body `task_self`；消费active token，address-only result storage必须无active transaction且为AllocatedUninitialized，复用或建立immutable ExceptionBox，原子pending->failed并唤醒waiters；自身不向外unwind |
| async.task.destroy | async.task.destroy %task : !place<rw,!runtime-object<task,T>> -> () | TaskDestroySummary, ReadMemory(task_state), WriteMemory(task_state), WriteMemory(typed), EndLifetime, Deallocate(task_frame), RuntimeEffect(task_destroy), RuntimeEffect(fatal), MayDiverge | SC | 非 terminator；要求owner authority；created/succeeded/failed按TaskDestroySummary清理、终结Task ObjectLifetimeGeneration并使storage uninitialized；pending进入pending_task_destroy fatal且不产生normal/unwind/trap completion，不wait/detach |
| async.cancel.request | async.cancel.request %task : !place<rw,!runtime-object<task,T>> -> () | WriteMemory(task_state), RuntimeEffect(cancel_request) | SC | 非 terminator、nothrow、幂等、线程安全；只发布单向request，不运行用户代码或改变Task final state |
| async.cancel.is_requested | %requested = async.cancel.is_requested %task : !place<ro/rw,!runtime-object<task,T>> -> bool | ReadMemory(task_state), RuntimeEffect(cancel_query) | SC | 非 terminator且nothrow；结果只观察request bit，不驱动Task |

### 16.4 decorator schema

decorator operation 是规范 Staged IR，但不是 Closed runtime ABI。它们必须在 ClosedModule 前展开；表中 continuation capability 由包围的 decorator.region schema产生，不能成为 SSA、存储或逃逸。

| Opcode | 精确 shape | Effects | 阶段 | terminator 与关键前置/后置条件 |
| --- | --- | --- | --- | --- |
| decorator.region | decorator.region {decorator_kind = sync/async, layer = N, function_type = !tN} (%entry_args...) -> Results {single-entry region} | nested region effect upper bound, Control | S | 非 CFG terminator但拥有一个 region；entry signature精确等于被装饰 callable；region只能以 continuation_yield、unwind transfer或不可达结束 |
| decorator.continuation_invoke | value: decorator.continuation_invoke {next_layer = N, function_type = !tN}(%args...) normal ^after(result(0)) unwind ^cleanup(exception); void: 同形但normal ^after; address: 同形并to %dst及DestinationPayload、normal ^after | CallSummary, Control；按 effective next-layer BehaviorContract 含 MayUnwind | S | terminator；每次建立fresh dynamic activation；destination原样转发；continuation不可存储/并发进入；normal/unwind edge使用统一sentinel |
| decorator.continuation_yield | decorator.continuation_yield [ %value ] {result_mode = value/result_destination/void} | Control | S | terminator且无普通 CFG successor；把当前layer完成返回最近的continuation_invoke；address result只传播destination已commit证明 |
| async.continuation_invoke | value: async.continuation_invoke {next_layer = N, function_type = !tN}(%args...) normal ^after(result(0)) unwind ^cleanup(exception); void/address: 同形但normal ^after，address复用包围Task result destination | TaskBodyEffect, RuntimeEffect(async_suspend), MayDiverge, Control；按 effective next-layer BehaviorContract 含 MayUnwind | S | terminator；共享同一Task/frame，不构造child Task；允许暂停但所有live state满足frame规则；不可并发/逃逸 |

### 16.5 hot-reload schema

stable-entry selection不是可拆出的普通 opcode；它是stable target call schema固有的 RuntimeEffect(version_select) 原子语义。以下 operation只管理已经确定版本的显式 pin。

| Opcode | 精确 shape | Effects | 阶段 | terminator 与关键前置/后置条件 |
| --- | --- | --- | --- | --- |
| rt.version.current_owner | %owner = rt.version.current_owner : () -> !runtime-handle<version_owner> | Pure | SC | 非terminator；只允许entry_identity = version_local，或继承已pin activation的compiler-generated helper entry block；物化当前activation已经固定版本的borrowed owner capability，不选择current target、不新增pin；owner不可存储、返回或逃出该activation/pin lifetime |
| rt.version.pin | %pin = rt.version.pin %owner : !runtime-handle<version_owner> -> !runtime-handle<version_pin> | ReadMemory(runtime_registry), WriteMemory(runtime_registry), RuntimeEffect(version_pin) | SC | 非 terminator且 nothrow；owner明确某一已发布版本；产生线性pin，不执行current-version selection |
| rt.version.unpin | rt.version.unpin %pin : !runtime-handle<version_pin> -> () | ReadMemory(runtime_registry), WriteMemory(runtime_registry), RuntimeEffect(version_pin) | SC | 非 terminator且 nothrow；消费pin；该pin覆盖的version-local uses与owner transfer均已结束 |
| rt.version.transfer_pin | rt.version.transfer_pin %pin, %owner {owner_kind = task/exception/snapshot/frame} | ReadMemory(runtime_registry), WriteMemory(runtime_registry), RuntimeEffect(version_pin) | SC | 非 terminator且 nothrow；消费独立pin并把唯一释放责任转给Alive runtime owner；owner kind/type必须匹配且不得重复transfer |

### 16.6 external ABI schema 与 record

abi.call/abi.invoke只接受declarative`AbiImport`或direction=`InkToExternal`的`AbiBridge`；call.*只接受Ink Function。`AbiCalleeKindTag=direct`没有C callee operand且`DirectSymbolId`必须解析到唯一AbiImport；`BridgeSymbolId`要么就是该AbiImport.Symbol的identity bridge，要么解析到`ExternalTarget`恰为该AbiImport的target-bound InkToExternal AbiBridge。`indirect`的operand 0是准确C function pointer、没有DirectSymbol，并且BridgeSymbol必须解析到`ExternalTarget=absent`的InkToExternal AbiBridge；AbiImport、target-bound bridge、普通Function或`EntryIdentity=extern`都不能替代该indirect bridge。两种form的C/Ink type、TargetAbiTag、policy owner和payload必须逐项一致。

`TrustedExternalEffects : Opt<EffectUpperBoundPayload>` present时必须来自compiler/platform registry并进入RuntimeDeclaration AbiDigest、dependency identity与verifier assumption，不能来自用户或call-site自由attribute。`ExternalEffectSummary`是从direct AbiImport及其匹配target-bound bridge的相等字段推导的effect set，不是第二种record或自由flag。v0 indirect C function pointer没有不可伪造的registered-target provenance，其`ExternalTarget`与`TrustedExternalEffects`都必须absent并无条件使用最大保守ExternalEffectSummary；只有direct form能使用匹配受信record中present的较窄上界。

EffectUpperBoundPayload闭合覆盖MemoryAccess、runtime/allocate/deallocate集合、MayTrap、MayDiverge、TargetDependent和PdbBoundary；外部代码可能执行的callback效果必须递归并入同一上界。TrustedExternalEffects absent时使用ReadWriteAny、全部runtime/allocate/deallocate kind、MayTrap、MayDiverge、TargetDependent和PdbBoundary的最大保守值。

三个policy类型严格分离：AbiImport保存`ForeignFailurePolicyPayload`，其闭集只有`nothrow`与`status_to_exception`；AbiExport保存`ExportExceptionPolicyPayload`，其闭集只有`nothrow`、`catch_and_status`与`catch_and_callback`；AbiBridge保存outer-tagged `TaggedBoundaryPolicyPayload`，InkToExternal恰包foreign-failure payload，ExternalToInk恰包export-exception payload。`status_to_exception`保存准确StatusType/StatusChannel/SuccessStatus、registered foreign error type、sync nothrow factory/type与protocol digest，且只允许abi.invoke；success后才发布result，failure先撤销未发布result再由factory建立ExceptionBox。Export status/callback policy保存各自mapping与digest并在wrapper内catch-all；raw foreign exception永远不能直接成为Ink token。

| Opcode/record | 精确 shape | Effects/阶段 | terminator 与关键前置/后置条件 |
| --- | --- | --- | --- |
| abi.call | void/scalar: `[ %r = ] abi.call CTarget(%args...) {c_function_type = !tC, bridge = @Bridge, expected_ink_function_type = !tInk, target_abi_tag = DIGEST, explicit_argument_count = N}`；address: 同形后接 `to %dst {destination_role = result}` | ExternalEffectSummary, RuntimeEffect(ffi)；SC | 非terminator；AbiCalleeKind direct/indirect及callee operand由P32唯一编码；ForeignFailurePolicy必须为nothrow，违反约定唯一进入fatal；result form与bridge signature精确，address result唯一commit |
| abi.invoke | value: `abi.invoke CTarget(%args...) {c_function_type = !tC, bridge = @Bridge, expected_ink_function_type = !tInk, target_abi_tag = DIGEST, explicit_argument_count = N} normal ^normal(result(0)) unwind ^failure(exception)`；void: 同形但normal无result；address: 同形并在successor前接destination | abi.call effects, MayUnwind, Control；SC | terminator且恰有normal/unwind；ForeignFailurePolicy可为nothrow或status_to_exception，只有后者从注册status/factory产生Ink exception；raw C++/SEH exception不得直接成为edge token |
| AbiImport RuntimeDeclaration | `RuntimeRecordKind = AbiImport`，KindPayload依次为`CSymbol, CFunctionTypeId, ExpectedInkFunctionTypeId, BoundarySignatureMapPayload, TargetAbiTag, Opt<TrustedExternalEffects>, ForeignFailurePolicyPayload` | record，SC，无执行effect | 不是Function/CFG opcode；direct abi target的唯一linker identity；双signature、逐参数map、logical result channel、TargetAbiTag/effects/foreign policy进入AbiDigest |
| AbiBridge RuntimeDeclaration | `RuntimeRecordKind = AbiBridge`，KindPayload依次为`CFunctionTypeId, InkFunctionTypeId, BoundarySignatureMapPayload, c, BridgeDirectionTag, Opt<AbiImportSymbolId> ExternalTarget, TargetAbiTag, Opt<TrustedExternalEffects>, TaggedBoundaryPolicyPayload` | record，SC，无执行effect | InkToExternal present ExternalTarget只供匹配direct import，absent只供indirect且effects absent；ExternalToInk的target/effects都absent并只供AbiExport；direction、signature map与outer policy tag精确匹配 |
| AbiExport RuntimeDeclaration | `RuntimeRecordKind = AbiExport`，KindPayload依次为`CSymbol, InkTargetSymbolId, CFunctionTypeId, BoundarySignatureMapPayload, BridgeSymbolId, TargetAbiTag, StableAddress, ExportExceptionPolicyPayload` | record，SC，无执行effect | 不是CFG opcode；ink_target固定sync ordinary/no receiver，ExternalToInk bridge的双signature/map/policy闭合；backend直接合成nothrow或catch-all C wrapper，不生成Function extern_bridge；hot reload export必须stable_address并进入Ink stable entry |

### 16.7 v0 非 canonical 拼写

以下名称不得出现在v0 canonical Staged/Closed dump或binary OpcodeTag中：

- reflect.snapshot.acquire：与lookup found transition的select+pin重复；
- async.await_cancel_on_request、async.all、async.all_cancel_on_error：降为库/runtime callable或generated state machine；
- 任意reflect.lookup_* 的“返回nullable裸descriptor pointer”变体：会绕过snapshot owner；
- 任意把interface reference、Optional<ref>、Optional<InterfaceRef>或其他address-only结果按TargetABI niche改成SSA的cast变体；
- 通用rt.call、runtime.invoke、obj.init.abort或可用自由attribute伪造上述effect/successor的操作。

实现可以在private lowering中展开canonical operation，但canonical printer/encoder必须保留本节schema身份直到相应合法阶段边界；不能把语义提前压成无法重建的任意runtime call。

## 17. Verifier 规则

### 17.1 通用调用

Verifier 必须检查：

1. Closed InkIR中每个callable和调用点都具有闭合logical signature、function kind、result_passing_mode、calling convention和TargetABI tag；
2. 实参数量、顺序、类型、passing mode和callee_kind精确匹配；reference、interface reference及borrowed typed place参数携带的ObjectLifetimeGeneration必须仍匹配当前Alive complete object，copy/edge transfer只保留generation，raw pointer不携带generation；reflection private DynamicRef不得作为Core实参类型出现；
3. sync callable不能由async.call/async.invoke调用，async callable不能由call.*伪装成返回Task的同步函数；
4. 标量结果只能由标量形式产生，address-only结果只能使用caller destination；
5. address-only destination的大小、对齐、地址空间和transaction identity正确，并在调用开始时具有唯一未提交active transaction；fresh owner路径具有唯一obj.init.begin，forwarded entry不要求本activation再次begin；normal证明普通结果最终完成者唯一generic commit或Task的唯一schema-specific commit，unwind证明callee outward-unwind postcondition已rollback；
6. 每个destination调用都编码DestinationValueId、ConstructedTypeId和DestinationRoleTag；constructor FunctionSignature为void/void且destination_role为initializing_receiver，普通结果/Task为result；constructor文本的constructed channel T不得被当作logical result；
7. address-only normal successor不得接收result(N)或place edge value；所有访问由normal-dominated place.as_alive开始。rollback路径保留原init capability并在旧transaction消费后才能fresh begin；destroy后复用storage必须经place.as_uninitialized；每次CapabilityRebind消费当前PlaceCapabilityGeneration并使旧root及全部descendant capability失效，且不得复制/CSE/推测；
8. call.direct、call.indirect、call.virtual、call.interface和reflect.call只能调用nothrow或持有独立不展开证明的目标；
9. may-unwind直接、间接、virtual、interface和reflection同步调用必须使用准确callee_kind的call.invoke；may-unwind async construction必须使用async.invoke；
10. 普通间接函数类型不得隐式携带nothrow；
11. call.invoke、async.invoke、abi.invoke及其他schema-declared unwind terminator的normal result不得在unwind edge使用；
12. unwind edge必须以canonical exception sentinel产生并最终线性消费exception token；
13. never callable没有可达normal completion；其invoke仍保留无result normal successor，目标block只能cf.unreachable或由verifier证明不可达；
14. call.*只接受calling_convention = ink（含EntryIdentityTag extern）；CallingConvention=c的CFunctionType只被declarative boundary records引用，调用只经abi.call/abi.invoke，导出只由abi.export record表示；
15. 不得通过tail-call lowering跳过cleanup、pin release、catch completion、version_select或stable entry。

对每个passing mode为`object`的参数，verifier还必须从call operand追溯到准确`!place<rw,T>` prepared owner及其显式copy/direct-construction，证明所有较早prepared参数在较后实参、dispatch、adapter验证或activation建立失败时由caller逆序清理，并证明activation建立点原子把全部owner转移给callee。callee entry的对应`parameter` argument必须是同一Alive owner place；normal与unwind出口都恰好discharge一次callee cleanup obligation。async construction还必须区分转移前caller cleanup、转移后construction cleanup及成功后Task frame capture owner，任何路径都不得双重destroy或遗失obligation。

### 17.2 初始化与生命周期

Verifier 必须检查：

1. fresh final storage owner在callee与全部实参求值完成后、首次构造前对AllocatedUninitialized destination恰好执行一次obj.init.begin；begin与首次构造之间不得读取、逃逸或另行begin；
2. forwarded ResultDestination在entry时已经携带唯一active transaction；中间转发activation必须原样保留transaction identity，不得重复obj.init.begin/commit；
3. 每个带destination的call、reflection、async construction、cast address-result、abi bridge或async.await_copy开始时都消费同一个尚未提交active ResultDestination；
4. 最终完成普通非sealed结果构造的leaf/callee在每条normal completion path恰好执行一次generic obj.init.commit；中间转发callee自身零次commit，只有内层normal已证明committed后才可normal return；
5. 最终失败leaf/callee的unwind/creation-failure不得commit；它先cleanup已完成子对象，再由outward-unwind postcondition使storage回到AllocatedUninitialized；中间forwarder只传播rolled-back proof；
6. call.*、reflect.call和abi call operation不合成begin、commit或rollback；async.call/async.invoke不合成begin或generic commit，但其normal schema postcondition必须原子执行唯一Task-specific commit，unwind postcondition执行唯一rollback；successor只传播对应状态证明；
7. async.await_copy是明确schema特例：entry消费已有active transaction，pending不转换状态，normal在不会展开的typed Copy完整完成后原子唯一commit；Task failed是唯一failure，Copy尚未开始且failure postcondition直接rollback；successor不得补第二次commit/rollback；
8. constructor entry 额外使用 initializing_instance receiver；
9. constructor normal edge 只建立一次完整对象生命周期；
10. constructor unwind edge 不运行完整 destructor；
11. obj.destroy只接受当前Alive、非SealedRuntimeStorage且具有owner LifetimeAuthority的place；rw borrow仍不得销毁或重建，Task只由async.task.destroy结束生命周期；
12. obj.destroy后旧PlaceCapabilityGeneration及全部descendant不可继续使用；只有place.as_uninitialized产生next generation并重新完成合法初始化后才能访问；
13. obj.destroy_dynamic只用于具有兼容vtable、完整对象地址、匹配ObjectLifetimeGeneration和owner LifetimeAuthority的virtual object；完成时终结generation；
14. obj.destroy_dynamic 后的 storage release 必须由独立 owner operation 完成；
15. destructor、defer 和 cleanup 不得向外产生普通 unwind edge；
16. address-only copy 必须显式满足 Copy，不得用 memcpy 代替可能具有 padding、vptr 或子对象规则的语言复制。

Task destination虽然允许owner执行obj.init.begin，却因SealedRuntimeStorage禁止generic obj.init.commit；只有匹配async schema的Task-specific commit postcondition可建立Alive generation。destructor_body必须满足4节的闭合签名矩阵，且只能从obj.destroy或obj.destroy_dynamic的compiler-generated destroy plan可达。obj.destroy_dynamic还必须从dynamic-destroy stable envelope重建CallSummary、必要的version_select和MayDiverge，不能把nothrow误当成pure或willreturn。

### 17.3 异常

Verifier 必须检查：

1. handler 顺序与源码语义顺序一致；
2. catch-all 最多一个且必须最后；
3. 没有 exception filter 或 multi-catch 伪操作；
4. catch binding 永久 readonly；
5. ExceptionView 不可复制、不可存储、不可逃出 handler 生命周期；
6. eh.rethrow 只使用当前词法 handler 的 active token；
7. eh.throw_from 的 cause 只能是当前 handler token，并且只在新 payload 初始化成功后转移；
8. eh.end_catch、eh.rethrow、eh.throw_from、eh.resume和`rt.fatal %exception`对同一token恰好选择一条所有权路径；operandless rt.fatal不消费token；
9. nothrow sync function 没有可到达的 outward eh.resume；
10. cleanup 中再次抛出只能进入 fatal；
11. exception token producer严格限于已注册schema的unwind edge、eh.throw/eh.throw_copy/eh.throw_from和failed await；eh.rethrow/eh.resume只转移现有token，其他operation或自由attribute不得生产。

从failed Task建立的每个ActiveUnwindRecord必须持有独立ExceptionBox强引用；Task、各waiter record与cause link分别只释放自己的引用。verifier/runtime schema必须保证eh.end_catch不会无条件销毁共享payload，且payload/cause/pins仅在最后一个强引用释放时销毁一次。

### 17.4 virtual 与 interface

Verifier 必须检查：

1. override 精确匹配根 slot 的参数、receiver、function kind、calling convention、logical result type 和 result_passing_mode；
2. 同步和异步 virtual/interface 都不允许结果 covariance；
3. override 复用原 slot，新 slot 顺序确定；
4. virtual call 的 receiver 与 declaring class 兼容；
5. interface ref同时具有有效canonical object pointer、兼容table和匹配该Alive complete object的ObjectLifetimeGeneration；descriptor copy、interface upcast和thunk adjustment保留同一generation/non-owning语义，从descriptor形成的place只有borrow LifetimeAuthority，rw不得升级为lifecycle owner；
6. interface upcast 使用已声明 ancestor conversion；
7. default method receiver 已规范化到 declaring interface；
8. 可实例化动态类型不存在可到达 abstract slot；
9. dynamic cast同时检查名称身份、layout version、关系数据和source ObjectLifetimeGeneration，不仅比较缓存指针；checked成功结果保留generation，raw-pointer结果显式不携带；
10. cast.try_class只有raw_pointer result_form可产生nullable scalar pointer；Optional<ref>必须使用active destination并在Some/None构造后commit；
11. cast.try_interface、interface reference和Optional<InterfaceRef>始终address-only并使用destination，TargetABI niche或两字寄存器拆分不得改变canonical result shape。

### 17.5 reflection

Verifier 必须检查：

1. lookup found edge原子select+pin并产生一个linear owned snapshot，missing不产生；不存在第二次reflect.snapshot.acquire；
2. 每个owned snapshot在所有路径恰由reflect.snapshot.release消费一次，或显式clone出独立owner；snapshot/member view不可存储、Copy或逃逸；
3. canonical receiver/arguments必须是expected signature规定的typed SSA、handle或place；address-only place携带的ObjectLifetimeGeneration仍匹配Alive complete object且保持non-owning，object mode携带3.3节prepared owner。private DynamicRef只可由adapter lowering临时形成，不得作为TypeId、SSA、block argument、capture或serialized payload出现；borrowed member view的全部uses受owner支配且早于release；
4. sync descriptor只由reflect.call或callee_kind = reflection的call.invoke使用，async descriptor只由callee_kind = reflection的async.call/async.invoke使用；
5. canonical optional DestinationPayload 与准确 function type能唯一lower成DynamicOut/DynamicTaskOut，expected type、alignment、ABI tag、transaction identity和state正确；
6. reflection adapter只有在完整结果构造成功后唯一commit；outward unwind先cleanup再由postcondition rollback，中间adapter不重复begin/commit/rollback；
7. adapter不把private DynamicRef array、ExceptionView、snapshot handle或borrowed member view保存进Task；async construction只保存完成passing-mode handoff的typed capture并转移derived version pin；
8. version update后旧snapshot仍调用旧pinned version-local adapter并使用其精确summary，不经过current stable entry重绑定。

`reflect.call`必须同时具有snapshot记录的target_nothrow和adapter_nothrow证明；后者覆盖signature/type/layout/generation/resource/result adapter的所有失败。缺少任一证明时必须使用`call.invoke callee_kind = reflection`，且其unwind edge不能被目标body的nothrow单独删除。

### 17.6 async Task

Verifier 必须检查：

1. async call 只在最终 Task destination 中构造一次，并遵守同一 fresh/forwarded ResultDestination transaction：fresh owner 唯一 begin，forwarding activation 不重复 begin 或 commit；
2. !runtime-object<task,T>必须是由TargetContext和RuntimeAbiRevision给出Size、Alignment与RuntimeStorageAbiHash、并固定具有SealedRuntimeStorage的SizedObjectType，且不作为普通SSA值、普通Copy、隐式move或普通address-only memcpy的对象；禁止place.addr/deref与projection、typed/raw load/store、constant、generic copy/assign/destroy及重叠byte-count operation观察或改变其私有表示；
3. construction failure edge 不 commit；最终 task-construction callee 清理部分 Task、释放临时 pin，并由其 outward-unwind schema postcondition 使 destination 回到 AllocatedUninitialized；中间 forwarding activation 不重复 rollback；
4. 每个async declaration分别使用正向BehaviorContract bits construction_nothrow与body_no_fail；nothrow bit对async不适用且必须为零，源码`[nothrow] async`只设置body_no_fail；
5. async declaration logical T 与所有形成的 !runtime-object<task,T> 都满足Task result admissibility：T为void/never，或闭合runtime-representable且Copyable，并且不含ContainsNoEscapeValue；noncopyable address-only T在形成Task之前拒绝；
6. effective contract不能证明construction_nothrow时必须使用async.invoke；只有construction_nothrow证明在当前版本边界稳定时才可使用async.call；
7. 创建成功由async.call/async.invoke normal schema postcondition恰好执行一次Task-specific commit并把pin转移给Task，同时为fresh Task generation建立准确TaskBodyEffect与TaskDestroySummary side facts；任何路径都不得对Task执行generic obj.init.commit，中间转发层不得再次转换状态；
8. created 只转换到 pending 一次；
9. pending 只转换到 succeeded 或 failed；
10. final state 不可改写；
11. async body entry具有唯一borrowed `task_self`；address-only T另有唯一owner `task_result_storage`，其entry状态为AllocatedUninitialized且无transaction；二者都只能来自runtime entry envelope、不能由普通projection伪造或逃逸，role/type/order与Function record精确一致；
12. async.task.publish_success的`%task`来自当前body `task_self`；object variant的`%result`来自当前body `task_result_storage`的fresh begin/commit/as_alive链且完整Alive，value variant由schema写入标量后原子publish，void/unit无物理payload；
13. async.task.publish_failure消费有效active exception token；address-only result若曾begin则partial cleanup与rollback已完成，若未begin则保持未初始化，publish前无active transaction且storage为AllocatedUninitialized；postcondition持有有效immutable ExceptionBox；
14. async.await/await_copy的result type与Task logical T精确一致，canonical CFG恰有normal/unwind；pending暂停activation并登记当前operation continuation，不进入第三个successor；
15. await只出现在suspendable async/coroutine activation；所有跨暂停liveSSA/place/borrow/transaction/cleanup/pin可合法spill并由frame覆盖lifetime；checked reference/interface与typed place必须连同ObjectLifetimeGeneration/non-owning side fact spill，borrowed place还要spill LifetimeAuthority，且其owner lifetime覆盖暂停区间，resume后重新匹配；private DynamicRef根本不能跨暂停；active exception、continuation capability、no-escape safe slice和native-stack borrow不得跨暂停，raw pointer不能凭地址相同获得checked lifetime证明；
16. async.await_copy的destination地址稳定、transaction与cleanup state可spill；T满足Copy，typed Copy不会unwind；normal唯一commit，Task failed时Copy未开始且failure postcondition直接rollback；
17. failed await为当前传播建立独立active unwind record，不复用其他等待者的平台展开对象；
18. async.task.destroy要求匹配Alive Task generation与owner LifetimeAuthority并在成功时终结generation；statically known pending Task不得进入该操作，无法静态证明时保留runtime fatal check；
19. body_no_fail=true的async函数不存在可到达async.task.publish_failure，所有语言异常在体内处理或转换为fatal；
20. 普通async.await不隐式传播cancellation request；
21. generated combinator state machine/runtime callable在任何outward failure前已使全部输入final，并按源码参数顺序选择失败；ClosedModule不得含async.await_cancel_on_request、async.all或async.all_cancel_on_error OpcodeTag。

每个Task construction必须从callee/slot/envelope重建准确TaskBodyEffect与TaskDestroySummary；二者是verifier-only generation side facts，不是可序列化payload。Task place在phi处逐字段取并集，跨函数或其他provenance不能保持的位置退化为最大保守上界。async.await、async.await_copy和drive_once的derived effect必须并入TaskBodyEffect；async.task.destroy必须并入TaskDestroySummary。failed Task与每个failed-await ActiveUnwindRecord分别持有ExceptionBox强引用，destroy/end_catch只释放自身引用，最后owner才销毁payload。

### 17.7 decorator 与 hot reload

Verifier 必须检查：

1. continuation signature 与目标函数精确一致；
2. continuation 不可存储、返回、闭包捕获或跨 Task/线程逃逸；
3. 每次 continuation 进入建立 fresh activation；
4. async.continuation_invoke 与目标共享 Task 和 frame，不创建 child Task；
5. continuation_local entry 不经 stable entry 递归调用当前 decorator；
6. 每个stable entry call执行不可消去/CSE/推测/复制/跨publish移动的原子select+pin及RuntimeEffect(version_select)，不能用当前target读取或当前body summary替代；
7. stable call只使用对应StableEffectEnvelope phase；version-local call只有持有匹配pin时使用精确body summary，且不再次version_select；
8. v0无公开pure contract时envelope默认覆盖ReadWriteAny、任意runtime/allocate/deallocate、MayTrap、MayDiverge、TargetDependent和PdbBoundary；三个异常baseline bits在首次兼容记录后保持不变，实际异常通道还必须与当前不可逆加强的nothrow/construction/no-fail BehaviorContracts取交集；
9. rt.version.current_owner只出现在version_local entry或继承已pin activation的compiler-generated helper，结果严格借用当前activation版本且不选择current target、不新增pin、不可存储/返回/逃逸；rt.version.pin只接受仍有效的这种owner并产生需唯一释放或转移的owned pin；
10. normal/unwind completion释放sync pin，async construction success转移pin、failure释放；rt.version.transfer_pin保持线性唯一释放责任；
11. code-only update保持StableEffectEnvelope canonical encoding/hash以及ABI/layout/dispatch/Task hash兼容，新版本三个phase summary逐字段不超出旧envelope且不削弱当前单调异常契约；行为契约加强不改写baseline envelope bits；
12. 每个候选publish record的BehaviorContractDigest由当前contract值重算，loader相对同lineage当前已发布contract验证单调性后才可原子publish；当前digest和值进入import dependency、incremental/cache key和AOT call-site assumption，不能让依赖加强证明命中旧宽松artifact；
13. module registration interface/set digest均从中央schema与canonical records重算；兼容更新保持interface digest，允许set digest变化但必须与stable entries/reflection/globals原子publish；每条旧record只由匹配旧version owner和RegistrationIdentity撤销，frozen record安装/移除不运行用户代码；
14. 只有证明禁用热更新或版本在整个dynamic extent冻结且pin覆盖时才能消除version_select并内联version-local body；
15. version-local entry的所有可达调用路径持有兼容pin，raw version-local entry/table地址不逃出owner/pin dynamic range；
16. 普通ABI/layout-compatible对象即使由V1构造也不pin V1代码；其vptr/interface table常驻stable，virtual/interface/dynamic destroy选择current compatible version；
17. Task/frame、active call/EH、snapshot及显式version-local owner持有必要pin；raw class/reference/interface borrow不伪装成owner；
17. V1 module globals保持版本化并只在V1 owners/pins排空后逆序销毁。

### 17.8 C ABI

Verifier 必须检查：

1. extern C签名只包含目标已确认的C ABI-safe类型；
2. CallingConvention=c的CFunctionType/AbiImport只由abi.call/abi.invoke调用，call.*目标只为ink EntryIdentityTag（包括extern）；
3. Ink address-only值不未经bridge直接套用C聚合ABI，每个address result编码destination_role = result；
4. Ink或foreign exception不存在越过未适配C boundary的路径；abi.invoke token只由registered conversion bridge产生；
5. export只由abi.export record表示，wrapper nothrow或具有catch-all到显式外部错误协议；
6. C function pointer和Ink function value不在没有bridge时互相重解释；
7. hot reload C export只公开stable bridge地址并进入Ink stable entry。

## 18. 私有 TargetABI 默认

本节给出初始实现默认，不属于长期语言承诺。TargetABI 可以在保持前述规范性语义和 ABI version 隔离的前提下替换它们。

### 18.1 默认物理参数顺序

初始 Ink TargetABI 建议使用：

1. address-only result storage；
2. instance receiver；
3. 显式参数；
4. 必要的 runtime context 或 pin transfer token。

标量结果使用目标自然返回寄存器。address-only 结果使用隐藏 sret 风格指针。constructor destination 复用 result-storage 位置。

unit 参数和结果不占物理寄存器或栈槽，但其 SSA 定义与使用仍保留到语义验证完成。address-only 按值参数先具有独立参数对象语义，再由 TargetABI 选择传递其地址或在证明等价时拆分到寄存器。

LLVM lowering 可以给隐藏 destination 添加适用的 sret、noalias、nonnull、dereferenceable、align 或 writeonly 属性，但只能在 InkIR alias、初始化和异常规则确实证明时添加。

上述是普通 sync/constructor 的初始物理顺序，不覆盖 async body 的 runtime-private resume ABI。后者至少传入 frame/task context，由 lowering 在固定 frame layout 中物化 semantic `task_self`；address-only logical T 还物化 `task_result_storage` 的准确地址与 side state。二者在 Core entry envelope 的相对顺序固定，但可以在 LLVM 中来自寄存器、frame slot 或已验证的 rematerialization，不能通过公开 Task struct GEP 或普通用户参数伪造。

### 18.2 默认 vtable

初始私有 vtable 建议逻辑排列为：

~~~text
private header
    TargetABI version
    class layout hash
    minimal type descriptor pointer
    dynamic destroy stable entry
virtual slots
~~~

vptr 可以指向表对象起始位置，也可以按目标选择指向 slots 并用固定负偏移访问 header。源码和 portable InkIR 不得观察这一选择。

### 18.3 默认 interface table

初始私有 interface table 建议逻辑排列为：

~~~text
private header
    TargetABI version
    concrete type descriptor
    interface identity and layout hash
method slots
ancestor conversion entries
~~~

slot 和 conversion entry 的精确顺序由编译器生成的确定性布局描述决定，不由用户手写索引。

### 18.4 默认 stable thunk

初始 runtime 建议让公开 slot 指向 stable thunk，而不是把可卸载的裸实现地址直接暴露给调用者：

~~~text
stable thunk
    → read current compatible implementation
    → acquire pin in the same protected protocol
    → call or construct task
    → release or transfer pin
~~~

原子实现可以使用 epoch、hazard pointer、RCU、引用计数或等价同步。本文不固定算法和内存序细节，只要求没有“先读取可卸载裸地址、后补 pin”的窗口。

### 18.5 默认异常 lowering

Itanium 目标可以把平台 unwind header 与 Ink private record 放在同一分配块。Windows 目标可以让 funclet/SEH 状态引用同一个逻辑 ExceptionRecord。两者不要求物理结构相同。

异常记录分配失败不得再次抛出普通异常。runtime 必须具有紧急终止路径或受限 emergency record。

### 18.6 默认 Task lowering

!runtime-object<task,T> 的size、alignment和RuntimeStorageAbiHash由TargetContext与RuntimeAbiRevision查询，因此它在semantic IR中是可形成place和caller destination、但固定SealedRuntimeStorage的SizedObjectType。其初始私有表示可以保存opaque control/frame pointer、状态和flags、resume/destroy entries、result或box位置以及pin set；具体字段是否内联、frame是否单独分配和waiter如何登记由runtime version决定。TaskBodyEffect/TaskDestroySummary是与Task generation关联的verifier/lowering side facts，可以在AOT中降为已验证side table或静态proof，不要求增加用户可观察机器字段。lowering只能经已注册runtime schema访问这些字段，不能先把它改写成runtime.opaque，也不能向普通place/memory opcode公开地址、物理field path或bytes；Task-specific commit必须在async runtime intrinsic的normal语义内原子完成，不能发出generic obj.init.commit；销毁必须使用async.task.destroy，不能降成generic destroy。

任何表示都必须保持：

- Task 最终存储地址稳定；
- created 到 pending 只驱动一次；
- final publish 与等待者同步；
- pending destroy fatal；
- result 和 ExceptionBox 发布后只读，每个failed-await record取得独立box强引用且最后owner才销毁payload；
- drive/await/destroy保留TaskBodyEffect/TaskDestroySummary给出的效果上界；
- 用户 Copy 不隐藏 runtime retain。

## 19. 解释器与 AOT 一致性要求

RuntimeWorld 和每个 AOT TargetABI 必须对以下事件序列进行 differential 验证：

- 标量和 address-only 参数、结果，包括object parameter逐个prepare、转移前caller逆序cleanup、activation原子handoff及转移后callee/Task唯一cleanup；普通fresh destination的唯一begin/commit、forwarded destination的零次重复begin/commit、Task的唯一schema-specific commit和所有failure path rollback；
- direct、indirect、virtual、interface 和 reflection 调用；
- constructor normal completion 与部分构造 unwind；
- static 和 dynamic destroy 顺序；
- catch handler 选择、cleanup、rethrow 和 cause；
- async creation failure 与 task body failure 的双通道区别；
- virtual/interface async dispatch 只发生在任务创建时；
- failed Task重复await的独立ActiveUnwindRecord/ExceptionBox强引用，以及最后owner唯一payload销毁；
- pending Task destroy fatal；
- decorator continuation 零次、一次和多次进入；
- code-only hot reload 中新调用进入 V2、旧 calls/tasks/snapshots 保持 V1；
- stable call在当前body较纯时仍保留version_select和公开StableEffectEnvelope，新版本三个phase summary逐字段不越界；
- !runtime-object<task,T>在解释器与各TargetABI中具有相同RuntimeStorageAbiHash、size/alignment查询结果、TaskBodyEffect/TaskDestroySummary上界和私有访问边界；
- place.as_alive/place.as_uninitialized消费旧PlaceCapabilityGeneration并使所有旧root/descendant views失效；
- reference/interface/typed place复制与CFG合流保留准确ObjectLifetimeGeneration；reflection private DynamicRef不跨activation或CFG存在；destroy并在同址重建后旧checked view继续失效，raw pointer不被错误赋予generation；
- C bridge 的参数、结果和异常隔离。

解释器不得为了实现方便把 address-only 结果先构造成宿主临时对象，也不得使用宿主 exception、RTTI、vtable 或 coroutine ABI 代替 Ink 语义。

## 20. 相关已确认议题

本文落实以下既有语言议题：

- 议题 16—18：decorator continuation、module registration、module lifecycle；
- 议题 19—31：reflection、class、vtable、interface、dynamic destroy 和 try_cast；
- 议题 34—42：unchecked exception、nothrow、ExceptionRecord、cause、traceback 和 fail-fast；
- 议题 43—60：Task、lazy async、ExceptionBox、cancellation、async dispatch、async reflection 和 async decorator。

值初始化、place、drop flag 和普通 cleanup plan 的基础定义由本目录的值与生命周期规范给出；本文只规定它们穿过调用、unwind、Task 和 runtime boundary 时必须保持的契约。
