# IR 序列化

`src/include/ink/ir/serialization/serialization.h` 提供文本和二进制读写，以及独立的 `IRVerifier`。实现位于 `src/lib/ir/serialization`，测试位于 `src/testcase/ir/serialization_*_test.cpp`。

## API 与所有权

| 操作 | 便捷函数 | 类入口 |
|---|---|---|
| 文本写入 | `writeText(Module, Options)` | `IRTextWriter::write` |
| 文本读取 | `readText(Context, Text, Options)` | `IRTextReader::read` |
| 二进制写入 | `writeBinary(Module, Options)` | `IRBinaryWriter::write` |
| 二进制读取 | `readBinary(Context, Bytes, Options)` | `IRBinaryReader::read` |
| 完整 IR 检查 | — | `IRVerifier::verify` |

写入返回 `std::string` 或 `std::vector<std::byte>`，读取返回 `std::unique_ptr<Module>`，都包装在 `std::expected<T, IRSerializationError>` 中。所有读写均校验 IR。文件读写、日志和诊断展示由调用方负责；编解码器不产生进程输出，也不通过异常报告失败。

```cpp
ink::ir::IRSerializationResult<std::unique_ptr<ink::ir::Module>> copyModule(ink::ir::IRContext &Destination, const ink::ir::Module &Source)
{
  auto Bytes = ink::ir::writeBinary(Source);
  if (!Bytes)
  {
    return std::unexpected(std::move(Bytes.error()));
  }
  return ink::ir::readBinary(Destination, *Bytes);
}
```

Context 由外部持有，必须比返回模块及其所有使用者存活更久。返回模块可以直接交给使用同一个 Context 的 `ModuleManager::adoptModule()`；读取过程不自动注册模块、查找导入或加载依赖文件。

加载期间需要独占 Context；现有名称池也遵循外部同步约定。内部 `IRLoadTransaction` 记录两个池的存储边界。失败时清除本次新增常量、类型对应的全部缓存项，再释放对象和临时模块；成功后提交事务。事务不会回滚全局 NamePool 中已经 intern 的名字。成功加载后的模块仍遵循现有“模块卸载及缓存失效不受支持”的生命周期约定。

## 格式与覆盖范围

文本语法详见 [IR-Serialization-Text.md](IR-Serialization-Text.md)。文本版本统一使用 `src/include/ink/core/config.in` 中的 `INK_IR_VERSION`，当前为 `"0.0.1"`；生成的 `ink/core/config.h` 供读写器使用。文件头写为 `ink_ir 0.0.1`，读取器要求版本字符串与宏完全一致。文本采用 LLVM 风格的函数体、基本块标签和指令，类型保留 Ink 的 `u32`、`*const u8` 等语义。文本版本和二进制版本独立；二进制格式为版本 1。

两个格式共用内部记录和重建模型，覆盖当前 Module 能表达的内建及结构类型、名义类型及其限定视图、六类常量、全局变量、函数及参数、基本块、全部现有指令、字段、方法分派属性、导入导出和模块初始化/销毁入口。保留名字、声明顺序、字段布局、freeze 状态及字符串存储缓存身份；浮点与宽整数按位保存。未支持的 kind 明确失败。文本使用模块符号和函数内的局部符号，符号与内部记录编号独立。

内部协议定义集中在 `src/lib/ir/serialization/Record.def` 和 `schema.h`：前者声明记录编号、分类、文本名称和布局，后者定义各记录的命名字段、固定或可变字段数量、可空引用以及枚举的原生值、协议值和文本拼写。模型和文本读写器通过这些定义访问字段并构造记录，二进制读取器也使用相同的记录分类定义。增加记录或枚举时修改公共定义，并补充对应的语义构造和验证逻辑；不依赖记录编号连续、枚举数组顺序或各读写器独立维护的字段下标。

二进制序列化以速度为优先：`binary.cpp` 直接编码、解码内部记录，保持固定宽度字段、连续分区和数值引用。文本符号分配、类型拼写、常量格式化和语法解析均隔离在文本编解码器中；二进制路径不调用这些逻辑，也不增加用于文本显示的字段或字符串。文本往返后的二进制输出仍须与原模块的二进制输出逐字节一致。

名义类型先创建身份，然后补充结构类型、字段和方法，支持递归引用。函数与基本块先建立，再用显式工作栈解析指令依赖，最后按记录顺序挂入基本块。当前格式支持前向引用，全量重建模块；没有延迟函数体加载或压缩。

Verifier 检查声明与引用的所有权、类型、方法及字段元数据、指令操作数、终结指令、分支目标和 SSA 支配关系。分析按 CFG 建立支配树，不要求基本块的存储顺序等于执行顺序。泛型实例化、导入绑定和运行时安全不属于序列化校验。

## 二进制版本 1

所有整数固定使用 little-endian，按字段编码，不直接写入 C++ 对象、指针、结构体 padding、`Name::index()` 或 `ValueKind` 的枚举序号。目标机器端序作为独立属性保存。头和分区中的偏移、长度、数量使用无符号数。

文件头固定为 32 字节：

| 字节偏移 | 宽度 | 内容 |
|---|---|---|
| 0 | 8 | ASCII `INKIRB` 后跟 CR、LF |
| 8 | 4 | 版本号，当前为 1 |
| 12 | 4 | 特性标志，当前必须为 0 |
| 16 | 8 | 整个文件的字节长度 |
| 24 | 4 | 分区数，当前固定为 4 |
| 28 | 4 | 保留，必须为 0 |

文件头后是四个 32 字节目录项，每项包含 `Kind:u32, Reserved:u32, Offset:u64, Size:u64, Count:u64`。分区按下表顺序连续排列，不能重叠、留洞或附加未索引尾部数据；目录偏移相对文件开头。

| Kind | 分区 | Count |
|---|---|---|
| 1 | Target | 1 |
| 2 | Strings | 去重字符串数量 |
| 3 | Records | IR 记录数量 |
| 4 | Operands | 本分区字节数 |

Target 固定 56 字节：`PointerBits:u32, ByteOrder:u32, NativeAbi:u32, Reserved:u32`，再跟五个 `u64` 对齐值，依次为 i8、i32、i64、i128、f64。ByteOrder 的 0/1 表示 little/big；NativeAbi 为 0/1；Reserved 为 0。读取时全部与外部 Context 核对，不能仅凭相同指针位宽就视为 ABI 相同。

Strings 先放 `Count` 个 `Offset:u64, Length:u64` 条目，再顺序放原始字节。这里 Offset 相对 Strings 分区开头，Length 不含任何隐式终止符。字符串按首次遇到的顺序去重，空字符串和内嵌 NUL 均有效。

Records 中每条记录固定 32 字节：

```text
Kind:u32
NumberCount:u32
ReferenceCount:u32
StringCount:u32
PayloadOffset:u64
PayloadLength:u64
```

记录 ID 是 Records 中的零基下标。Kind 使用 `src/lib/ir/serialization/Record.def` 声明的 `RecordKind` 数值，与运行时枚举排列无关。PayloadOffset 相对 Operands 分区开头。每条 payload 顺序存放 `NumberCount` 个 u64、`ReferenceCount` 个 u32 记录 ID、`StringCount` 个 u32 字符串 ID。可选的空记录引用用 `UINT32_MAX` 表示；字符串引用没有空哨兵。

以下是记录 payload 的字段次序。N、R、S 分别表示数字、记录引用和字符串；省略的组为空。

| 记录 | N | R | S |
|---|---|---|---|
| Module | — | init, deinit | 模块路径组件 |
| BoolType / VoidType / NullPointerType | mutable | — | — |
| IntegerType | mutable, width, signed | — | — |
| FloatType | mutable, width | — | — |
| MetaType / VariadicType | — | — | — |
| PointerType / ReferenceType | mutable | element | — |
| ArrayType | mutable, count | element | — |
| FunctionType | mutable, 每个参数的 variadic | return, 参数类型 | — |
| ClassType / InterfaceType / StructType | size, alignment, frozen | — | name |
| QualifiedType | mutable | declaration | — |
| Field | offset | owner, type, initializer | name |
| Function | linkage, language, visibility, dispatch, override, final, slot | signature, owner | name |
| Parameter | index | function | name |
| Block | — | function | name |
| Global | string_storage | address_type, initializer | name |
| Import | — | type | 模块路径组件, symbol |
| Export | — | declaration | symbol |
| BoolConstant | value | type | — |
| IntegerConstant | 从最低有效字开始的所有 u64 字 | type | — |
| FloatConstant | bits | type | — |
| NullPointerConstant | — | type | — |
| StringConstant | — | type | bytes |
| ArrayConstant | — | type, elements | — |
| Alloca | initialized | type, block | name |
| Load | — | type, block, pointer | — |
| Store | — | type, block, value, pointer | — |
| Neg | — | type, block, operand | — |
| ICmp | predicate | type, block, left, right | — |
| Branch | — | type, block, target | — |
| CondBranch | — | type, block, condition, true, false | — |
| Return | — | type, block, value | — |
| Call | — | type, block, signature, callee, arguments | argument_names |
| GetElementPtr | inbounds | type, block, source_type, pointer, indices | — |

布尔字段使用 0/1。Function 的 `linkage` 0/1/2 分别为 Local/Import/Export；`language` 0/1 为 Ink/C；`visibility` 0/1 为 Public/Private；`dispatch` 0/1/2 为 Direct/Virtual/Abstract。缺省 `slot` 使用 `UINT64_MAX`，因此完整的 uint32 槽位范围都可以保存。Global 的 `string_storage` 标识模块的池化字符串存储缓存条目。

ICmp 的 `predicate` 0 至 9 依次为 Equal、NotEqual、UnsignedGreaterThan、UnsignedGreaterEqual、UnsignedLessThan、UnsignedLessEqual、SignedGreaterThan、SignedGreaterEqual、SignedLessThan、SignedLessEqual。

二进制读取拒绝未知版本、标志、分区和记录种类。改变字段含义或布局需要升级格式版本。

## 资源限制与性能

`IRReadOptions` 限制输入字节、记录数、数字/引用/字符串项总数、字符串字节总数和类型/常量依赖深度。二进制还检查字符串去重展开后的总字节，防止小字符串表被重复引用而产生无界复制。所有文件范围都先检查再读取或分配，错误通过返回值报告。

二进制省去文本 token、十进制转换、字段名识别和字符串转义处理，使用记录下标解析引用、预留容器容量。它仍需构造现有 IR 对象并完成相同的语义校验；性能评估应包括这些开销，同时区分内存缓冲区读取和文件 I/O。输出大小由 `IRTextWriteOptions::MaxBytes`、`IRBinaryWriteOptions::MaxBytes` 限制。

现有 CMake 递归收集源文件和 IR 测试，无需新增 target。可运行 `ink_tests --gtest_filter=*Serialization*` 检查格式往返、损坏输入、资源限制、深层依赖、长 SSA 链及事务和验证行为。
