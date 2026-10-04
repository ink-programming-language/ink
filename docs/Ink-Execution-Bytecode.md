# Ink 执行字节码

解释执行使用两层表示：`ink::ir` 保留语义已经确定的函数、类型、操作数和控制流；`ink::execution` 在具体函数首次执行时将它降低为 `ExecutableFunction`。后续执行复用指令和槽位布局，每次调用仍使用独立实参、局部存储和中间结果。普通解释执行与编译期函数调用通过同一个 `ExecutionEngine::execute()` 入口进入这条路径。

执行镜像拥有已编码的常量、字符串字节、函数描述和共享的存储布局，不保存 `ir::Type*`、`ir::Constant*` 或 `ir::Function*`。IR 到运行时身份的映射只保留在编译侧的 `SemanticValueBridge`；`ExecutionLinker` 的按需编译模式通过该桥接层访问源函数，也可以直接接收准备完整的 `ExecutionImage`，在不持有桥接层、源 IR 已销毁后继续执行。`artifact/` 层为镜像增加模块符号、独立对象链接与磁盘归档：不同 Context 生成的对象可分别保存，再读入、重映射 ID 并链接为完整可执行镜像。v6 文件要求与运行端相同的 target，不保存宿主地址或执行快照；格式和边界见 [Ink 字节码文件格式](Ink-Bytecode-Format.md)。

## 组成与职责

| 类型或文件 | 职责 |
| --- | --- |
| [`BytecodeInstruction`](../src/include/ink/execution/bytecode/instruction.h) | 一条操作码及固定长度操作数数组 `Operands` |
| [`instruction.def`](../src/include/ink/execution/bytecode/instruction.def) | 统一登记操作码名称和每项操作数的种类，生成 `BytecodeOpcode` 与元数据 |
| `BytecodeInstructionMetadata`、`bytecodeInstructionMetadata()` | 查询名称及操作数模式，供验证和工具使用 |
| [`ExecutableFunction`](../src/include/ink/execution/bytecode/executable_function.h) | 拥有连续指令、运行时槽位类型 ID、初始槽、调用点、自有常量字节区与局部帧单元数量 |
| [`RuntimeValue`](../src/include/ink/execution/runtime/runtime_value.h) | 归档常量与语义/原生边界的拥有型值；进入 VM 时转换为目标布局字节，执行中不作为槽位载荷 |
| [`RuntimeSlot`](../src/include/ink/execution/runtime/runtime_bytes.h) | 调用帧字节区的视图，保存地址、共享布局引用与初始化状态；值字节中不含这些元数据 |
| `ClassDesc` | 每个类型域中每个类共享一份不可变描述，包含名义身份、字段名、字段类型、偏移和子布局 |
| [`TypeDesc` / `RuntimeTypeTable`](../src/include/ink/execution/reflection/type_table.h) | 预先确定表示种类、位宽、符号、大小、对齐、指向类型、数组元素类型与数量、权限和函数签名 |
| `ExecutionCallSite` | 直接 `FunctionId` 或间接被调用者槽位、签名 ID，以及已经排序的实参槽位 |
| [`ExecutionCompiler`](../src/include/ink/execution/bytecode/execution_compiler.h) | 按函数分配槽位、选择操作码、定位跳转并验证产物 |
| [`ExecutionImage`](../src/include/ink/execution/bytecode/execution_image.h) / [`ExecutionLinker`](../src/include/ink/execution/engine/execution_linker.h) | 持有执行镜像和函数描述，准备运行时调用目标；支持按需编译与完整镜像两种入口 |
| [`BytecodeArtifact`](../src/include/ink/execution/artifact/bytecode_artifact.h) | 在镜像外记录 target、模块、符号定义/导入、可见性与可选入口 |
| [`buildBytecodeObject()`](../src/include/ink/execution/artifact/bytecode_builder.h) / [`linkBytecodeArtifacts()`](../src/include/ink/execution/artifact/bytecode_linker.h) | 编译显式登记的全部定义，按稳定符号身份链接独立对象并重映射类型和函数 ID |
| [`bytecode_archive.h`](../src/include/ink/execution/artifact/bytecode_archive.h) | 确定性写出、受预算约束的读取和文件 I/O；对象与可执行镜像共用版本化格式 |
| [`SemanticValueBridge`](../src/include/ink/execution/bridge/semantic_value_bridge.h) | 在语义类型、常量和函数与运行时 ID、载荷之间转换 |
| [`ExecutionEngine`](../src/include/ink/execution/engine/execution_engine.h) | 对外执行入口、共享存储、预算、取消和显式状态 |
| [`ExecutionMachine`](../src/include/ink/execution/engine/execution_machine.h) | 使用函数 ID、运行时值和显式调用栈分派指令，不遍历 IR |
| [`ExecutionMemoryManager`](../src/include/ink/execution/memory/execution_memory_manager.h) | 根据存储布局分配 Cell/Buffer、维护身份、权限、生命周期和预算，不依赖 IRContext |
| [`NativeCallCache`](../src/include/ink/execution/ffi/native_call_cache.h) | 按函数缓存原生地址与已准备的 ABI 签名，实参缓冲仍属于每次调用 |
| [`IRContext::revision()`](../src/include/ink/ir/context.h) | 在语义执行入口 O(1) 判断源程序是否改变，以失效代码与链接缓存 |

语义分析负责名称绑定、重载选择、类型检查、编译期常量和运行时控制流。执行编译器只处理已确定的 IR，不重新查找源码名字，不推导重载，也不重新解释函数 AST。

## 指令模式

`BytecodeInstruction` 使用 `std::array<std::uint32_t, 4> Operands` 统一保存操作数。每项的意义由 `instruction.def` 的对应行决定，宏模式为 `INK_INSTRUCTION(Name, Operand0Kind, Operand1Kind, Operand2Kind, Operand3Kind)`；下标不预设目标、输入或跳转等用途。`SlotId` 同样为 32 位无符号整数，`InvalidSlot` 是保留的无效值。操作数种类包括读槽、写槽、指令目标、调用点、运行时布局 ID、局部帧索引、常量字节偏移与长度、比较谓词和失败状态。32 位的是索引或编码，64 位整数值与宿主指针不压缩进这些操作数字段。

例如：

```cpp
INK_INSTRUCTION(AddI32, WriteSlot, ReadSlot, ReadSlot, None)
INK_INSTRUCTION(CompareSigned32, WriteSlot, ReadSlot, ReadSlot, Predicate)
INK_INSTRUCTION(CallDirect, WriteSlot, CallSite, None, None)
INK_INSTRUCTION(JumpIf, None, ReadSlot, Target, Target)
INK_INSTRUCTION(AllocaLocal, WriteSlot, Layout, Local, None)
INK_INSTRUCTION(CString, WriteSlot, DataOffset, DataLength, None)
INK_INSTRUCTION(Array, WriteSlot, DataOffset, DataLength, None)
INK_INSTRUCTION(ArrayRepeat, WriteSlot, ReadSlot, None, None)
INK_INSTRUCTION(ArrayElementPointer, WriteSlot, ReadSlot, ReadSlot, None)
INK_INSTRUCTION(ArrayExtract, WriteSlot, ReadSlot, ReadSlot, None)
```

`AddI32` 将 `Operands[1]`、`Operands[2]` 指定的槽值相加后写入 `Operands[0]` 指定的槽；`CompareSigned32` 额外用 `Operands[3]` 指定谓词。`CallDirect` 的 `Operands[1]` 是调用点表索引，不是函数体宿主地址。`JumpIf` 的 `Operands[1]` 是 bool 条件槽，`Operands[2]` 和 `Operands[3]` 分别为真、假分支在本函数指令数组内的 PC。未使用项以 `None` 标记并置零。

`AllocaLocal` 的 `Operands[1]` 是被分配值的运行时类型 ID，`Operands[2]` 是该函数独占的局部帧单元索引。`CString` 的 `Operands[1]`、`Operands[2]` 是自有 `ConstantData` 中的字节偏移与长度；`Function` 从 `InitialSlots[Operands[0]]` 取出预先编码的函数 ID。比较谓词使用独立的 `ExecutionPredicate`，编译器显式转换 IR 谓词，VM 不依赖 IR 枚举。

`Array` 从 `ConstantData` 指定的范围读取小端 `u32` 源槽号，按顺序构造结果数组；字节长度必须为元素数乘 4。`ArrayRepeat` 读取一个元素槽，按结果数组布局的元素数复制，不会重新执行产生该元素的指令，零长度重复数组仍求值并检查该元素。`ArrayElementPointer` 的两个输入依次是数组地址和整数索引，结果为同权限的元素地址；`ArrayExtract` 的两个输入依次是数组值和整数索引，结果为元素值快照。普通构造与重复构造由 `ArrayInstruction::repeated()` 区分，其余两种操作分别降低对应的 IR 指令。

指令构造使用嵌套聚合明确操作数数组，例如 `BytecodeInstruction{BytecodeOpcode::AddI32, {3, 1, 2, 0}}` 表示把槽 1 与槽 2 的和写入槽 3。

操作码按执行需要分组：

| 分组 | 操作码 | 含义 |
| --- | --- | --- |
| 地址与存储 | `Alloca`、`Load`、`Store` | 有语言可见地址或别名的受控内存访问 |
| 定宽整数内存 | `LoadI8/I16/I32/I64`、`StoreI8/I16/I32/I64` | 预先选择整数访问位宽；原生 Cell 可直接读取或写入位模式 |
| 已证明局部的存储 | `AllocaLocal`、`LoadLocal`、`StoreLocal` | 地址仅供本函数直接读写时使用内部局部存储路径 |
| 字符串 | `CString` | 按执行次数创建独立可写、以 NUL 结尾的副本 |
| 数组 | `Array`、`ArrayRepeat`、`ArrayElementPointer`、`ArrayExtract` | 构造数组值，按重复模式复制元素，或经边界检查取址、提取元素 |
| 整数加法 | `AddI8/I16/I32/I64`、`AddWide` | 按精确位宽回绕；其他位宽走通用宽整数路径 |
| bool | `LogicalNot`、`LogicalAnd`、`LogicalOr` | 对已求值 bool 操作，源码短路由控制流实现 |
| 比较 | `CompareSigned8/16/32/64`、`CompareUnsigned8/16/32/64`、`CompareWide`、`CompareBool` | 编译时确定符号和位宽，保留比较谓词 |
| 调用 | `CallDirect`、`CallIndirect` | 直接函数身份或本次调用的函数值 |
| 控制流 | `Jump`、`JumpIf`、`Return`、`ReturnVoid` | 已定位的跳转与返回 |
| 其他 | `Function`、`Failure` | 函数值及显式执行失败 |

添加操作码需要同时补充 IR lowering、VM 分派和验证规则。`.def` 中没有独立实现的操作码不会自动获得执行能力；现有 IR 序列化格式也不因增加执行操作码而改变。

## 槽位、地址与控制流

参数占据每个函数槽位数组的前缀；指令结果、常量及其他操作数获得该函数内的固定槽号。槽号相对于当前调用帧，不是 IR 节点地址，也不是整个程序唯一的变量地址。同一函数的递归调用使用相同槽号布局，但有独立槽位内容。

编译时允许使用 `ir::Value * → SlotId` 和 `ir::BasicBlock * → PC` 映射。生成产物后，标量指令直接按槽号访问当前帧，分支直接设置 PC，不在每次读取时通过 IR 对象哈希查找结果。所有槽位都按目标大小和对齐排入每帧独立的连续字节区。整数、指针、数组、class 和字符串描述符使用与可寻址对象相同的表示；宽整数仅在执行算术时解码临时字数组。

初版保持每个 IR 值独立的结果槽，没有寄存器着色或跨值槽位复用。加载结果是读取发生时的快照，后续写入同一局部变量不会改变早先加载的值。别名、动态地址和初始化遵守共享执行语义；无效裸指针访问属于未定义行为，不承诺 VM 与 AOT 给出相同诊断。

`ExecutableFunction` 中几张表承担不同职责：

| 表 | 使用位置 | 保留原因 |
| --- | --- | --- |
| `Layouts` | 分配、验证、宽整数和通用内存操作 | 共享有主的 `RuntimeTypeTable`，表项只包含预先降低的执行布局与类型身份 |
| `ConstantData` | `CString` 字节范围和 `Array` 源槽号表 | 自有字节区；CString 执行时复制成独立缓冲，数组按小端 `u32` 读取源槽号 |
| `SlotTypes` | 产物验证与槽位初始化 | 每个槽的 `RuntimeTypeId`，不含 IR 指针 |
| `InitialSlots` | 调用帧初始化和 `Function` 指令 | 与槽位等长；常量已经编码为位模式或拥有的载荷，其他槽保留类型 ID 与未初始化状态 |
| `Calls` | 直接或间接调用 | 已确定的函数 ID、签名 ID 与实参槽位，不保存源函数指针 |

这些表都是执行前已经确定的数据；普通 `AddI8/I16/I32/I64`、本机位宽比较和 bool 运算直接由 opcode 决定操作，不在每次运算时查询类型表。运行时布局仍保留必要的类型身份：相同大小的 `bool` 和 `u8` 不互换，不同函数签名也不因表示大小相同而互换。布局描述、签名 ID 和动态权限检查不承担源码名称解析或类型推导。

数组布局使用 `RuntimeKind::Array`、`ElementType`、`ElementCount` 和拥有的 `ElementLayout`；大小为元素步长乘元素数，对齐继承元素类型。数组按值传参、赋值和返回，通过复制整块目标布局字节形成独立快照；写入数组存储或其元素不会修改先前取得的快照。元素指针只保存基址加字节偏移得到的裸地址，整个数组重新赋值后地址保持不变。嵌套数组内联存储；空数组没有合法元素索引。索引接受各整数位宽，负值、无法容纳的宽整数和超出元素数的索引返回 `IndexOutOfBounds`。

可寻址数组和 class 使用共享目标布局的连续存储，可将元素或字段地址直接交给原生函数。宽整数、指针变量和指针字段也有真实内存表示；外部原生地址可以按声明类型直接读写。指针不携带分配身份或代次，复制聚合中的指针只复制地址，不延长目标生命周期。调用方负责保证地址有效、类型/对齐/权限匹配以及原生读写长度合法。

旧的 `Source`、`Types`、`Strings` 和借用 IR 的初始化表已经移除。函数镜像的常量、布局、验证和已链接代码的执行可以独立于原 IR 的生命周期。`ExecutionEngine::execute(ir::Function, ...)` 仍是服务于存活语义 Context 的入口；直接构造 `ExecutionLinker(ExecutionImage)` 后，`ExecutionMachine` 可按 `FunctionId` 执行自有镜像，缺少的函数体显式失败，不尝试返回源 IR 编译。磁盘加载由 `readBytecodeFile()` 和 `deserializeBytecodeArtifact()` 负责：验证通过的完整镜像交给上述入口执行，VM 继续使用 Engine 提供的存储与执行预算。

局部存储优化仅在分析全部相关使用后启用：Alloca 地址只用作本函数直接 Load/Store 的地址操作数时，才可采用 `Local` 指令；把地址作为参数、返回值、被存储的值或交给其他可观察它的操作时，保留普通指针路径。内部局部存储身份不能传出该路径。验证器检查其生产者、使用者和类型，不把内部句柄当作 Ink 指针。

`LocalStorageCount` 确定每个调用帧的紧凑局部单元数组。`AllocaLocal` 重置其固定单元，`LoadLocal` 和 `StoreLocal` 按已知大小直接复制帧内字节，不再逐次创建 `ExecutionCell` 或语言指针；循环再次经过分配时重新变为未初始化。每次动态分配仍按既有 Cell 大小收取累计对象数和存储字节预算，优化不会使循环逃过预算。非局部的 Alloca 仍分配具有独立地址的存储，由调用帧负责清理。

每个基本块先登记其首条指令位置，跳转在块布局完成后回填。本函数指令数组的存储顺序不改变 IR 控制流；无终结指令的块补充 `Failure(MissingBody)`，避免意外落入后续块。

## 调用、链接与缓存失效

执行编译器每次编译一个具体函数，不递归编译整个调用图。桥接层先登记 `FunctionId`，再降低函数体；直接调用点保留该 ID、签名 ID 与实参槽列表。链接器在目标第一次需要时准备代码，检查目标描述与调用签名一致，因此自递归和相互调用不要求递归展开所有被调用者。间接调用每次读取当前帧的函数 ID 并检查签名，不能把第一次观察到的目标永久当作该调用点唯一目标。

链接器有两种入口：`ExecutionLinker(SemanticValueBridge&)` 用于语义分析期间按需编译；`ExecutionLinker(ExecutionImage)` 用于已经准备好的自有镜像。后一模式要求提供被执行函数的代码、函数描述和共享布局表，能够在源 Context 销毁后完成函数调用、宽整数运算、局部存储、CString 分配以及按自有 ABI 描述发起外部调用。仍允许首次实际调用时准备原生地址和 libffi 计划；这不需要源 IR。

独立文件之间的符号解析由 `linkBytecodeArtifacts()` 完成，运行期 `ExecutionLinker` 接收其生成的完整镜像。对象构建器编译每个显式登记的定义，包括从未执行的函数；Ink 导入、原生导入及函数值引用也必须具备元数据。符号身份包含模块、声明名、规范化函数签名与有类型的泛型实参，`Private` 限于当前对象，`Module` 限于同模块对象，`Public` 对其他模块可见。源码顶层函数默认 `public`，显式 `private` 限于定义文件；语言映射只使用 `Public`/`Private`，协议的 `Module` 继续供底层 API 使用。链接先分配最终类型/函数 ID，再修正描述、调用点、函数值初始槽和布局操作数，支持前向引用与相互调用。完整镜像不保留未解析的 Ink 导入；原生符号仍在运行进程中解析。

VM 保存调用者 PC、返回结果槽和各调用帧的独立状态，Ink 函数调用通过压入/退出 VM 帧完成。被调用者返回后恢复调用者，再写入对应结果槽。源码中的深调用链不会为每个 Ink 调用再递归进入一个 C++ 解释函数；调用预算仍然有效。

原生边界由 `FunctionBinding` 决定，不能从 `LanguageLinkage::C` 推导：`import "C" func` 降低为 `External=true` 的原生导入；`export "C" func` 和 `[abi("C")] func` 均保留并执行本地字节码函数体。运行时描述独立保存 `CAbi` 和 `Exported`，模块导入引用保持 C ABI，但只由定义对象发布导出名。public/private 只控制 Ink 源码和对象链接访问；private 原生导出仍会参加最终镜像的原生名称冲突检查。字节码容器 v6 保存这些信息，实际 DLL/共享库生成与原生回调地址桥接仍未提供。

原生导入优先绑定同一链接集合中的同名原生导出，签名与 C ABI 必须一致；这个边界允许调用 private 导出的函数体，未被导出的普通 C ABI 函数不会参与匹配。对象链接器直接重映射函数 ID，只有未匹配的导入仍走宿主 FFI。解释模式由执行堆的 `SemanticValueBridge` 按 IRContext revision 为已加载模块建立导出索引，在降低原生引用时选择同一字节码目标；源模块依赖仍须通过 `import provider;` 等模块语句提供。普通编译侧桥接层默认关闭这项运行时解析，对象构建期间也显式关闭并在退出时恢复，避免提前吞掉应由对象链接器解析的导入。编译期调用准备对应导出函数体后进入同一路径。

已经解析并保存的函数值固定引用所选具体目标；之后撤销该目标的原生导出不会重定向这个值，重新降低原生导入和仍未解析的 import ID 才按当前导出集合解析。固定的是函数身份，函数体发生合法修改后仍通过 revision 更新执行代码。与普通本地函数值相同，按需编译模式下被引用的 IR `Function` 必须在函数值及其存储被使用期间保持有效；函数值不拥有或延长目标 IR 对象的生命周期，revision 不会将已保存值重绑定到替代对象。

编译期求值传入 C ABI 函数的字符串仍按 FFI 参数规则转换为独立可写的临时字节缓冲，即使目标已经解析到本地字节码定义。执行引擎复用 `FfiArgument` 的 RAII 生命周期：失败与普通返回释放临时缓冲；返回指针引用某个临时缓冲时，只保留该缓冲至堆释放。原字符串和同一字符串的其他实参副本不受修改影响。已求值字符串保留嵌入 NUL 字节；运行时源码字面量的 `CString` 降低继续检查并拒绝嵌入 NUL。

分派循环将 `CallDirect` 和 `CallIndirect` 交给 `ExecutionMachine::executeCall()`，实现位于 [`execution_machine_call.inc`](../src/lib/execution/engine/dispatch/execution_machine_call.inc)。该函数统一处理目标与签名校验、实参检查、原生调用和 Ink 帧入栈，并返回显式状态；调用栈扩容前完成全部调用者帧访问，失败仍由分派循环统一清理活动帧。

所有指令处理函数集中放在 `src/lib/execution/engine/dispatch/` 目录下，按职责分为 `execution_machine_scalar.inc`、`execution_machine_wide.inc`、`execution_machine_control.inc`、`execution_machine_memory.inc`、`execution_machine_array.inc` 和 `execution_machine_call.inc`，由 `execution_machine_dispatch.cpp` 包含到同一编译单元，并使用仓库现有的 `FORCE_INLINE` 宏。分派循环只选择处理函数和传播状态；内联实现及其依赖在该编译单元可见，最终机器码是否完全展开仍由编译器决定。

递归调用使用显式调用栈，每层都有独立的槽位、局部存储和 PC。正常 `Return` 只对栈顶帧执行 `endCall()` 并弹出该帧，把结果写回调用者后继续执行。执行失败、取消或预算耗尽时，`run()` 才在循环结束后逆序清理所有尚未返回的帧；正常返回到入口时调用栈已经为空，不会重复清理。

代码缓存缓存的是指令和布局，不缓存函数结果。两次调用的参数、可观察存储、外部调用和副作用都按本次执行重新发生。外部 C 函数通过 `NativeCallCache` 按需准备并复用原生地址与 `NativeCallPlan`；计划预先确定 libffi 签名、参数和返回值布局，VM 的 FFI 路径直接封送 `RuntimeValue`，不再将每个实参转换回 `ExecutionValueRef` 或读取 IR 签名。每次调用仍有独立实参缓冲，未找到的符号允许后续重试。`clearNativeSymbolCache()` 清除符号和调用计划；IR revision 改变也清除相应调用计划。字节码调用点不代替 ABI 参数封送。源码依赖发现和 `from ... import ...`、模块成员访问由 CLI 与语义层处理；运行时使用已链接的函数 ID，不读取源码导入语句。

`IRContext::revision()` 在结构/所有权变更以及 `Value`、`Decl` 销毁时递增。外层执行入口比较保存的 revision，变化后丢弃缓存，再按需编译。这样可以处理首次缺少函数体、随后补齐函数体，替换 return，摘除/重挂函数，以及函数销毁后宿主分配器复用原地址等情况。

类型和常量的纯新增、池查询及读取 IR 不递增 revision，因此将编译期结果冻结到常量池不会无故使全部代码失效。revision 目前是整个 Context 共用的保守版本；以后可以增加函数级版本，但必须覆盖源函数的销毁与依赖变化。一次执行期间不允许并发或重入修改 IR。运行时类型表保持追加，已有 ID 与描述稳定，以支持语义存储中的值跨调用保存；桥接层的源对象映射仍要求所属 Context 在引擎使用期间有效。

## 编译期执行

单模块 `Analyzer::analyze()` 按源码顺序进行分析与编译期求值；多模块 `analyzeModules()` 先预声明各模块的函数签名、解析导入，再分析函数体。跨模块编译期调用按需完成被调用函数及其函数引用依赖的分析，准备函数体前先处理其定义之前尚未执行的模块语句。已完成的具体函数可以编译执行，不必等待所有模块完成；编译期依赖正在分析的函数体时报告用户错误。函数内部编译期常量在分析函数体时确定。

显式编译期语句、静态 if/while 展开、模块和分析帧中的可变编译期绑定继续由语义驱动管理。`ExecutionFrame` 保存这些语义绑定与所属存储；`ExecutionEngine::resolveValue()` 将常量、函数身份或已有绑定解析为执行值，不访问 VM 私有槽位，也不执行 IR 指令。这些操作中的具体函数调用进入同一个字节码 VM：`SemanticValueBridge` 在入口将 `ExecutionValueRef` 转成运行时值，在出口转回语义值，可表示的编译期结果再冻结为 `ir::Constant`。已生成常量保留当时的值，后续模块编译期写入不会追溯修改它。

泛型的名称绑定、类型实参、实例缓存与实例化仍属于语义层。完成实例化后得到的具体函数使用同一条 lowering 与执行路径；运行时类型 ID 不代替泛型类型参数，也不要求提前实例化所有泛型。

失败、取消或预算耗尽不会把已发生的副作用自动回滚。取消及预算耗尽仍停止该引擎的后续执行，所有已建立的调用帧与受控存储通过显式清理释放。

## 执行预算与验证

步骤预算按 IR 语义操作收费，不按最终 opcode 的机器指令条数收费。函数准备阶段预计算指令的基础成本；VM 在指令生效前统一扣除。局部存储的已知成本一并计入；通用指针路径仍由其存储辅助操作补充动态成本，不能再次收取已经预扣的操作数读取成本。

| 指令 | 步骤成本 |
| --- | --- |
| 无操作数普通指令、无条件跳转、无值返回 | 1 |
| 普通读值操作 | 1 加读取操作数的数量 |
| `Alloca`、`AllocaLocal` | 2，包含分配操作 |
| `LoadLocal` | 3，包含地址读取与存储读取 |
| `StoreLocal` | 4，包含地址、值读取与存储写入 |
| `Load` / `LoadI*`、`Store` / `StoreI*` | 基础成本分别为 2、3；Cell 读写按现有内存路径再收取 1，Buffer 路径不额外收费 |
| `CallDirect`、`CallIndirect` | 2 加实参数量，包含被调用者与实参读取；进入被调用函数另行计费 |
| `Array`、`ArrayRepeat` | 基础成本分别为 1、2，再按结果元素数收取构造成本 |
| `ArrayElementPointer`、`ArrayExtract` | 3，包含数组或地址与索引读取 |

因此本地槽优化不会绕过步骤限制，某条 IR 操作选用本机位宽或宽整数 opcode 也不改变它的预算类别。调用深度、求值深度、累计对象数与累计存储字节继续通过 `ExecutionLimits` 约束；释放存储不退还累计分配预算。VM 栈消除了 Ink 调用对 C++ 调用栈的逐层依赖，但不取消这些逻辑限制。

编译器在读取 IR 时检查常量归属。`ExecutionCompiler::verify()` 对独立产物检查操作码、槽位及布局 ID、参数/结果类型、调用签名、初始位模式、分支目标、常量字节范围、紧凑局部单元唯一性与内部句柄使用规则，不再访问源 IR。链接器另外检查直接函数 ID 的目标描述和签名。验证器不是完整的语义验证器，也不以静态验证替代实际路径上的未初始化、动态指针生命周期和预算检查。

数组验证还检查元素数量、递归元素类型与初始化状态、源槽号表范围，以及索引类型和元素指针权限。重复构造在分配元素向量前检查 `MaxStorageBytes`，因此零大小元素也不能绕过元素数量预算；取址仍使用普通受控存储，不能把 `AllocaLocal` 的内部句柄传给数组元素地址操作。数组文件标签和条件布局字段见 [Ink 字节码文件格式](Ink-Bytecode-Format.md)，当前容器版本为 6、指令模式版本为 3。

数组专项测试位于 [`array_bytecode_test.cpp`](../src/testcase/execution/bytecode/array_bytecode_test.cpp)、[`array_storage_test.cpp`](../src/testcase/execution/memory/array_storage_test.cpp) 和 [`array_artifact_test.cpp`](../src/testcase/execution/artifact/array_artifact_test.cpp)，覆盖 lowering、非法槽号与类型、各位宽动态越界、独立快照、重复求值次数、局部指针逃逸、空与嵌套数组、存储预算及独立对象归档链接。

回归测试位于 [`execution_compiler_test.cpp`](../src/testcase/execution/bytecode/execution_compiler_test.cpp)、[`execution_image_test.cpp`](../src/testcase/execution/engine/execution_image_test.cpp)、[`bytecode_execution_test.cpp`](../src/testcase/execution/engine/bytecode_execution_test.cpp)、[`bytecode_storage_test.cpp`](../src/testcase/execution/engine/bytecode_storage_test.cpp) 和 [`context_revision_test.cpp`](../src/testcase/ir/context_revision_test.cpp)。除布局与签名身份、常量字节范围、局部索引、缓存更新、间接调用、整数边界及帧清理外，专门覆盖源 IR 全部析构后执行镜像内的函数调用、宽整数、局部存储，以及 CString 和重复原生调用；同时验证所有权编辑和归档恢复的 revision 边界。独立文件测试位于 [`execution/artifact`](../src/testcase/execution/artifact)：覆盖对象和完整镜像归档、三模块链接、ID 冲突、函数值调用、符号可见性、重载与泛型身份，以及截断、版本、target、宿主指针和资源预算错误。源码层的多模块主集成测试位于 [`execution/multifile`](../src/testcase/execution/multifile)，直接使用真实导入和 `public`/`private` 声明，覆盖各文件独立编译、正反顺序链接、跨模块编译期函数依赖及移走源码和对象文件后加载执行。普通 Ink 无体声明在语义阶段拒绝，不作为源码导入占位符。

## Class 值与字段地址

`Class` 按共享 `ClassDesc` 的字段偏移从源槽复制字节，`FieldExtract` 按偏移复制字段值，`FieldPointer` 将裸基址加上字段偏移。`ClassDesc` 保存稳定名义身份、字段名、字段类型和偏移；这些元数据由类型表统一持有，不嵌入对象，也不随实例复制。默认初始化表达式已经降低为构造调用中的代码，每次构造分别执行。对象在连续内存中保存字段，整体赋值写回原地址，先前取得的字段指针观察新值。嵌套数组/class 内联存储；指针字段复制地址，不延长目标生命周期。

构造验证字段数量、类型和初始化状态，并在分配前计算步骤及存储预算。字段提取与取址验证字段索引和静态类型，源码语义层检查完整初始化和可写权限。空 class 占用 1 字节，保证数组中的不同元素可分别寻址。冻结递归生成 `ClassConstant`，含无法持久化指针的字段仍失败。FFI 支持 `*Class` 和多级指针，按值 C 聚合调用 ABI 尚未开放。布局与未来虚函数/interface 约定见 [Class 与对象语义](Ink-Classes.md#对象内存-abi-与裸指针)。
VM 的常量在函数准备时转换一次；外部参数和结果在调用边界转换。内部调用、返回和 load/store 不递归构造 `vector<RuntimeValue>`。字符串槽位只保存 `{Data, Length}`，导入的不可变字符缓冲由执行内存管理器持有并计入字节预算，寿命覆盖调用帧和后续对象存储。初始化区间与运行时未知状态放在存储侧元数据；普通指针读写用有序地址索引查询已知分配并直接复制字节，不再扫描分配槽表或逐字段恢复值树。

类型描述与反射接口单独位于 `execution/reflection/`。`StorageLayout` 仅保存大小和对齐，`TypeDesc` 共享由继承实现的专用描述，不使用 `std::variant`。字段访问、方法调用、默认构造和 AOT 入口见 [运行时反射](Ink-Reflection.md)。
