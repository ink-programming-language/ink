# IR Module 归档

公共接口位于 `ink/ir/module/module_serialization.h`，由 `ink::ir` 提供：

```cpp
ModuleSerializeResult serializeModuleText(const Module &ModuleValue, ModuleArchiveLimits Limits = {}, std::span<const parser::ParseResult *const> Sources = {});
ModuleSerializeResult serializeModuleBinary(const Module &ModuleValue, ModuleArchiveLimits Limits = {}, std::span<const parser::ParseResult *const> Sources = {});
ModuleDeserializeResult deserializeModuleText(IRContext &Context, std::string_view Text, ModuleArchiveLimits Limits = {});
ModuleDeserializeResult deserializeModuleBinary(IRContext &Context, std::string_view Bytes, ModuleArchiveLimits Limits = {});
```

四个接口都通过 `succeeded()`、`Status` 和 `Message` 报告结果，不执行文件或进程输出，不依赖异常。成功序列化的数据位于 `Bytes`。反序列化成功后，`ModuleValue` 指向目标 `IRContext` 新增的根模块。失败不会返回部分归档或发布部分模块；此前已驻留的名称、类型、常量及源码缓冲区可以保留。

## 内容与生命周期

v5 增加完整 class 定义、名义身份、字段名称/类型/可见性，以及 `constant.class`、`class.value`、`field.pointer`、`field.extract`（稳定记录编号 60–63）。v6 进一步保留模块登记的 class、方法归属及字段初始化器。类型恢复先建立全部名义身份和结构类型，再补全 class 字段，最后恢复常量与指令；允许经指针的递归，拒绝无限大小的按值递归和不完整的存储布局。class 字段指令保存声明顺序索引，物理偏移由共享目标布局服务计算。

例如 `type !point = class "Point" identity "geometry::Point" { public "X": i32, public "Y": i32 }`；`class.value !point {1, 2}` 构造值，`field.extract i32, 0, !point %value` 提取字段，`field.pointer ptr<rw, i32>, 0, ptr<rw, !point> %storage` 投影字段地址。文本里的 class 常量写作 `!point {1, 2}`。未完成的名义类型仍可写作 `type !forward = class "Forward"`，只能用于无需完整布局的位置。

归档包含模块及嵌套模块、顺序基本块、函数、参数、全部当前指令种类、引用到的类型与常量，以及完整的声明树。整数按低位在前的 64 位字保存；浮点保存 IEEE 原始位模式，包括负零、NaN payload 和无穷。字符串和名称保存完整字节，允许包含 NUL。共享对象、递归调用、名义类型身份、参数类别和名字、调用约定、语言链接、源码可见性和原生符号方向均保留。

泛型声明的 AST 在二进制中使用 AST archive v5，在文本中使用具名字段语法保存，包含整个所属 `ParsedUnit` 的源码、token、语法树、恢复记录和解析状态。声明引用通过 AST 快照编号与后序遍历节点编号恢复；不会重新解析源码。多个声明和模块可以共享同一 AST 快照。语义分析器的外部绑定、实例化缓存与目标机器状态不属于 Module 归档。

原始 IR 声明只保存借用的 AST 节点，无法由节点查回所属 `ParseResult`。首次归档含声明树的模块时，调用方必须提供这些输入：

```cpp
const parser::ParseResult *Sources[] = {&Parsed};
auto Saved = ir::serializeModuleBinary(*ModuleValue, {}, Sources);
if (!Saved.succeeded())
{
  return Saved.Status;
}
auto Loaded = ir::deserializeModuleBinary(DestinationContext, Saved.Bytes);
```

缺少任一声明的 AST 输入会返回 `InvalidInput`，不会丢弃声明。没有声明树的模块不需要 `Sources`。恢复后的模块通过 `archivedASTs()` 共享持有所需 `ParseResult`，它们先于声明创建、晚于声明销毁。嵌套模块被拆出或原根模块被销毁时，其 AST 仍然有效。重新归档恢复后的模块无需再次提供 `Sources`。

非池对象的操作数必须位于待归档模块的拥有树中。引用其他根模块或游离函数会返回 `InvalidInput`；需要在模块内提供相应函数声明。归档不会将外部对象悄悄复制成新的身份。

## 文本格式 v6

文本采用可编辑的 IR 汇编语法。模块、函数和基本块直接体现嵌套结构，操作数使用符号引用，常量内联。下面是 `serializeModuleText` 的实际输出，单元测试逐字节验证此例：

```text
ink-ir 6
module @Example {
  define i32 @addOne(i32 %x) {
  entry:
    %v0 = add i32 %x, 1
    ret i32 %v0
  }
}
```

这是 Ink IR 的汇编语法，保留 Ink 自身的类型和指令语义，不能直接交给 LLVM 汇编器。没有函数体的函数使用 `declare`。嵌套模块继续使用 `module { ... }`，独立子块使用 `block ^label { ... }`。

### 符号与类型

- `@name` 引用函数或模块，`%name` 引用参数或指令，`^name` 引用基本块；函数体中的标签写作 `entry:`。
- 符号在整个归档内唯一，允许前向引用，包括递归调用和前向指令依赖。生成器给重复名字追加后缀，并用 `name "原名"` 保留模型中的原名。未命名参数同样用 `name ""` 保留。
- 名字可以加引号，例如 `@"包含空格的函数"`。字符串支持 `\\\"`、`\\\\`、`\\n`、`\\r`、`\\t`、`\\xHH`，UTF-8 文本直接保留，NUL 等控制字节转义。
- 分号 `;` 引入行注释；空格、制表符、LF 和 CRLF 都可使用。
- 参数类别使用 `named` 或 `variadic`；默认是 positional。函数可按顺序追加 `cc fast` / `cc cold`、`linkage c`、`visibility private` 和 `binding import` / `binding export`；默认调用约定为 C，语言链接为 Ink，可见性为 public，绑定为 local。Reader 也接受显式 `visibility public` 和 `binding local`，Writer 省略默认值。函数定义与声明都保留可见性和绑定。
- 原生 import 必须是无函数体的 C 声明，原生 export 必须是有函数体的 C 定义；两者要求 C 调用约定。`linkage c` 本身不表示原生导入或导出，本地 C ABI 回调使用默认的 `binding local`。`visibility private binding export` 合法，表示 Ink 名字访问受限但原生符号对外导出。
- 模块入口块通常隐含；如果其他指令引用它，模块头会附加 `entry ^label`。

基本类型使用 `type`、`void`、`bool`、`label`、`module`、`i32`、`u8`、`f64` 等拼写。整数位宽不限于常见机器字长，浮点位宽为 16、32 或 64。复合类型可内联，也可用别名；规范输出用别名避免深层类型展开：

```text
type !t0 = class "Box"
type !t1 = ptr<rw, !t0>
type !t2 = slice<ro, u8>
type !t3 = [16 x i32]
type !t4 = fn(i32, !t2) -> i32
```

同名的 `class`、`enum`、`interface` 定义保留各自的名义身份。复合类型中的目标别名允许前向引用；直接将一个别名定义为另一个别名时，目标需要已定义。类型和操作数依赖的循环会被拒绝。

整数常量使用有符号或无符号十进制，读取时检查位宽。浮点使用 `bits(0x7fc12345)` 明确保留负零、无穷与 NaN payload。布尔使用 `true/false`，字符串使用引号，类型作为值时写成 `type i32` 或 `type !t0`。

数组常量按元素顺序内联，例如 `[2 x i32] [3, 4]`、`[2 x [2 x i32]] [[3, 4], [3, 4]]` 和 `[0 x i32] []`。每项必须是数组元素类型的常量，元素数必须与类型长度一致；恢复时通过 `ConstantPool::getArrayConstant()` 驻留，嵌套数组和重复出现的相同常量保持共享身份。

### 指令

| 指令 | 文本示例 |
| --- | --- |
| 加法 | `%sum = add i32 %x, 1` |
| 逻辑非 | `%negated = not bool %condition` |
| 逻辑与 | `%both = and bool %left, %right` |
| 逻辑或 | `%either = or bool %left, %right` |
| 比较 | `%less = cmp lt i32 %left, %right` 或 `%same = cmp eq bool %left, %right` |
| 栈分配 | `%slot = alloca i32` |
| 读取 | `%value = load i32, ptr<rw, i32> %slot` |
| 写入 | `store i32 %value, ptr<rw, i32> %slot` |
| C 字符串 | `%text = cstring slice<ro, u8> "hello"` |
| 数组构造 | `%array = array [2 x i32] [%x, 7]` |
| 重复数组 | `%repeated = array.repeat [8 x i32] [%x]` |
| 数组元素地址 | `%element = array.element ptr<rw, i32>, ptr<rw, [2 x i32]> %slot, i32 %index` |
| 数组值取元素 | `%value = array.extract i32, [2 x i32] %array, i32 %index` |
| 直接调用 | `%result = call i32 @f(i32 %x)` |
| 间接调用 | `%result = call i32 %callback(i32 %x)` |
| 返回 | `ret i32 %result` 或 `ret void` |
| 无条件跳转 | `br ^merge` |
| 条件跳转 | `br bool %condition, ^then, ^else` |

非 void 指令必须声明结果名；void 指令仅在其结果被其他对象引用时输出结果名。读取时校验所有显式操作数类型、调用签名、结果类型、指令位置及返回类型。

`array` 接受与类型长度相同数量的元素值；`array.repeat` 始终接受一个已经求值的元素，按结果类型的长度复制，长度为零时也保留该操作数的求值。两者对应 `ArrayInstruction` 的普通与重复模式。`array.extract` 返回元素值的快照；`array.element` 接受指向数组的指针，返回指向元素的指针并保留原访问权限。索引必须是整数，执行时检查负值和越界；空数组的任何索引都越界。嵌套数组可逐层提取或取址，元素类型必须精确匹配。

`not`、`and` 和 `or` 只接受 bool 操作数，结果为 bool。`and`、`or` 是对已经求值的两个操作数执行的 IR 运算，不会跳过此前的操作数求值；源码 `&&`、`||` 的短路由条件分支、临时 bool 存储和汇合块表达。`cmp` 的结果为 bool，谓词 `eq`、`ne`、`lt`、`le`、`gt`、`ge` 分别对应相等、不等、小于、小于等于、大于、大于等于。两个整数操作数必须类型完全相同，比较遵守其符号属性和 IR 位宽；bool 仅允许 `eq` 和 `ne`。不支持浮点、指针或其他类型比较。

跳转和返回都是基本块的终结指令，其后不得追加其他指令。条件跳转的条件必须为 `bool`；所有跳转目标必须是同一函数中的基本块。目标允许前向引用、回边、自环，以及条件跳转的两个目标相同；控制流中的环不会作为类型或操作数依赖循环被拒绝。恢复时先创建全部目标块，再挂载跳转指令。

### 泛型声明与 AST

文本中的 AST 使用独立的 `ast 4` 语法，直接显示源码、token 名称、AST 节点类别、命名字段、声明可见性、原生符号方向和恢复信息。以下仅展示区段结构，省略号不是有效语法：

```text
syntax !ast0 ast 4 {
  source_name = "<input>"
  source = "func Identity[T: type](Value: T): T { return Value; }"
  status = Completed
  syntax_errors = false
  lexical_success = true
  tokens [
    ...
  ]
  nodes [
    %1 = NameExpr { range = range(17, 21), name = name(6, "type", range(17, 21)), comptime = false }
    ...
  ]
  recovery [
    ...
  ]
}
declarations from !ast0 %23 {
  func "Identity" from !ast0 %21 {
  }
}
```

节点使用从 1 开始的后序编号，`%N` 表示节点引用。字段使用 `range(begin, end)` 表示字节范围，`name(token_id + 1, "拼写", range(...))` 表示名字 token（0 表示缺失 token），`some(...)/none` 表示可选字段，`[...]` 表示数组，枚举直接使用名称。每个普通 AST 节点都包含 `comptime` 布尔属性，不再使用独立的 comptime 包装节点；旧的 `ast 1` 快照会被拒绝。声明树引用 AST 快照及节点，多个声明可共享相同节点。

恢复过程使用完整字段构造 AST，不重新解析 `source`，因此也保留错误恢复节点、token payload、取消/中断状态和共享身份。文本 AST 读入后复用现有 AST 构造与校验逻辑；文本与二进制可互相转换。

## 二进制格式 v6

二进制以编解码吞吐量为目标，独立于文本语法。所有整数固定宽度、小端序、字节对齐；没有逐字符 VBR 编解码，也不需要转义字符串。写入前计算完整长度，输出缓冲区只分配一次；字符串和 AST payload 整块复制。固定宽度元数据会比变长编码占更多空间。

16 字节文件头：

| 偏移 | 宽度 | 内容 |
| --- | --- | --- |
| 0 | 4 字节 | 签名 `IIRB` |
| 4 | u32 | `ModuleBinaryVersion`，当前为 6 |
| 8 | u32 | 对象记录数 |
| 12 | u32 | 保留 flags，必须为 0 |

每个对象使用 24 字节头部，紧随字段和原始文本字节：

| 偏移 | 宽度 | 内容 |
| --- | --- | --- |
| 0 | u32 | `module_serialization_tags.def` 中稳定的 kind ID |
| 4 | u32 | 类型对象 ID |
| 8 | u32 | 父对象 ID |
| 12 | u32 | 附加字段数 |
| 16 | u64 | 字符串/AST payload 字节数 |
| 24 | u64 × 字段数 | 附加字段 |
| 后续 | 原始字节 | 字符串或 AST payload，无 NUL 终止符 |

对象 ID 由记录顺序隐含决定，从 1 开始；1 是根模块，0 表示无引用。父对象先于子对象；类型和操作数允许向前引用。类型和常量不具有结构父对象。整数常量保存低位字到高位字，浮点保存原始位模式；AST payload 直接使用现有 IAST v5 二进制快照，旧版本快照会被拒绝。

读取器在分配前验证记录数量、字段数量和剩余字节，拒绝未知 kind、flags、截断、越界引用和尾随字节。版本由 `ModuleTextVersion` 和 `ModuleBinaryVersion` 分别管理；当前 v6 的 Function 记录前四项依次保存调用约定、语言链接、可见性（`Public=0`、`Private=1`）和原生符号绑定（`Local=0`、`Import=1`、`Export=2`），可选的后两项保存反射关联。v6 不兼容此前的 v1–v5 格式，缺失或非法字段、导入函数有函数体、导出函数缺少函数体会明确失败。AST 文本和二进制版本也独立管理。相同 Module 的规范输出不依赖指针地址、无关池插入顺序或宿主大小端。

跳转使用追加的稳定 kind ID，不改变既有记录：`Branch = 50` 的唯一字段为目标基本块 ID；`ConditionalBranch = 51` 的三个字段依次为条件值、真分支基本块、假分支基本块 ID。两种指令的结果类型均为 `void`，父对象为所属函数的基本块。

逻辑和比较指令继续追加稳定 ID，结果类型均为 bool：

| kind | 字段 |
| --- | --- |
| `LogicalNot = 52` | 操作数 ID |
| `LogicalAnd = 53` | 左操作数 ID、右操作数 ID |
| `LogicalOr = 54` | 左操作数 ID、右操作数 ID |
| `Compare = 55` | 谓词编号、左操作数 ID、右操作数 ID |

比较谓词的稳定编号为 `eq = 0`、`ne = 1`、`lt = 2`、`le = 3`、`gt = 4`、`ge = 5`，通过显式映射与 C++ 枚举关联；谓词编号不作为对象引用参与依赖恢复。非法谓词、不匹配的操作数或结果类型、越界引用和循环操作数依赖均返回 `InvalidArchive`。

数组记录继续追加稳定 kind ID，文本和二进制版本仍为 4，既有记录不变；不了解新 kind 的旧读取器会拒绝包含这些记录的归档：

| kind | 字段 |
| --- | --- |
| `ArrayConstant = 56` | 按顺序排列的元素常量 ID，空数组无字段；类型为数组，父对象为 0 |
| `ArrayValue = 57` | 重复标志（0 或 1）、元素值 ID；普通模式元素数等于类型长度，重复模式恰好一个元素 |
| `ArrayElementPointer = 58` | 指向数组的地址值 ID、索引值 ID |
| `ArrayExtract = 59` | 数组值 ID、索引值 ID |

重复标志不参与对象依赖恢复。数组常量拒绝非常量元素，数组指令校验精确元素类型、结果类型和指针权限；无效引用、非法标志和循环依赖返回 `InvalidArchive`。

Class 继续使用类型记录 `Class = 27`。未完成声明没有附加字段，文本为类名；完整定义保存 `[类名长度, 名义身份长度, 字段类型 ID, 字段可见性, 字段名长度, ...]`，文本依次拼接类名、名义身份和各字段名。字段可见性为 `Public=0`、`Private=1`，字段顺序即声明顺序。新增值记录如下：

| kind | 字段 |
| --- | --- |
| `ClassConstant = 60` | 按声明顺序排列的字段常量 ID |
| `ClassValue = 61` | 按声明顺序排列的字段值 ID |
| `FieldPointer = 62` | 字段索引、对象地址值 ID |
| `FieldExtract = 63` | 字段索引、对象值 ID |

Class 的名义身份先创建，再补全字段关系，因此经指针的递归类型可以恢复。字段索引不是对象引用；缺失字段、字段类型错误、重复字段名和按值循环布局都会被拒绝。

## 验证与资源限制

`ModuleArchiveLimits` 限制归档总字节、对象数、单条记录字段数、单条字符串字节数、累计内存预算和拥有树深度；文本中的内联类型和数组常量嵌套也受深度限制，数组元素占用字段和分配预算。`AST` 字段控制内嵌 AST 的源码、token、节点、数组等上限，其解码内存计入模块总预算。类型和操作数依赖采用迭代拓扑恢复，循环依赖会被拒绝；函数递归调用不构成这种依赖循环。

这些默认值统一登记在 `ink/core/config.def`，构造 `ModuleArchiveLimits` 时通过 `ConfigManager::getSize` 读取当时的环境变量：

| 字段 | 环境变量 | 默认值 |
| --- | --- | --- |
| `MaxArchiveBytes` | `INK_MODULE_ARCHIVE_MAX_BYTES` | 67108864（64 MiB） |
| `MaxObjects` | `INK_MODULE_ARCHIVE_MAX_OBJECTS` | 1048576 |
| `MaxFields` | `INK_MODULE_ARCHIVE_MAX_FIELDS` | 1048576 |
| `MaxStringBytes` | `INK_MODULE_ARCHIVE_MAX_STRING_BYTES` | 16777216（16 MiB） |
| `MaxAllocationBytes` | `INK_MODULE_ARCHIVE_MAX_ALLOCATION_BYTES` | 268435456（256 MiB） |
| `MaxNestingDepth` | `INK_MODULE_ARCHIVE_MAX_NESTING_DEPTH` | 256 |

非法或溢出的配置值回退到登记的默认值，0 保留其含义。已构造的限制对象不随环境变量变化，调用方仍可直接修改字段。嵌套的 `AST` 继续使用已有的 `INK_AST_ARCHIVE_*` 配置，不与 Module 的配置键混用。

反序列化通过既有类型池、常量池与 IRBuilder 校验对象、参数、结果类型、返回值和指令位置。所有树先在临时拥有者中恢复，声明恢复成功后才将根模块发布给目标 context。`parser::trySerializeAST` 与 `parser::tryDeserializeAST` 为容器提供返回状态的 AST 读写；原 `serializeAST` / `deserializeAST` 的 ICE 行为保持不变。


## 反射关联（v6）

Module 的载荷保存需要保留的 class 类型引用，即使没有函数使用该类型也会被序列化。文本使用 `reflect type !t0` 登记类型。

Function 原有四个字段之后，可以追加所属 class 引用和成员标志：`0` 表示方法，`字段索引 + 1` 表示默认初始化器。文本分别使用 `reflect !t0 method` 和 `reflect !t0 initializer 0`。读取时验证方法首参数指向所属 class，初始化器无参数且返回字段类型；方法可见性沿用函数可见性。源 AST 销毁后这些关联仍有效。

v6 新增以上关联，旧版 IR 需重新生成。字节码容器也升级为 v6；字节码指令模式仍为 v3。
