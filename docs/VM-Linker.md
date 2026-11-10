# VM 模块链接器

`src/include/ink/backend/vm/linker/module_linker.h` 提供 `ModuleLinker::link()`，输入多个只读 `MachineModule`，输出一个独立拥有全部节点的 `MachineModule`。第一版支持强定义、Local/External 链接属性、显式 Native 声明、严格符号解析和深克隆。

```cpp
std::array<const MachineModule *, 2> Inputs{&First, &Second};
auto Linked = ModuleLinker::link(Inputs, Diagnostics, OutputName);
```

返回值为 `std::unique_ptr<MachineModule>`，调用方必须传入 `const core::DiagnosticEngine &Diagnostics`。成功后可立即销毁所有输入；失败由链接器内部报告 `VMModuleLinkFailed`，返回 nullptr，不修改输入，也不返回部分模块。该诊断为普通错误，不使用异常或强制终止。输出名由调用方指定，默认匿名。空输入产生空模块。链接器保留输入顺序及函数、全局变量内部的创建顺序；只读与可写全局变量共用插入顺序，相同输入重复链接会产生相同的文本结果。

## 符号定义与声明

函数和全局变量共用一个 VM 链接名命名域，通过创建接口的 Linkage 参数指定 `MachineSymbolLinkage`：

- `Local` 是默认值，只能由原模块内的名称引用解析到。不同输入模块可以有同名 Local 定义；输出时为 Local 定义生成无冲突名称，同时重写其所属输入中的符号引用。匿名 Local 节点可保留，但无法通过名称引用。
- `External` 是跨模块可见的强定义，要求非空名称；全部输入中同名 External 定义最多一个，不按输入顺序选择胜者。
- `MachineModule::declareSymbol(Name, Kind, IsConstant)` 记录外部声明，Kind 为 `Function` 或 `GlobalVariable`。全局变量通过 IsConstant 属性区分只读和可写；全局声明与定义的常量属性必须精确一致，函数声明不能标记为常量。重复的兼容声明允许存在，但必须找到同名同种类、同属性的 External 定义。声明本身不占用函数体或数据存储，也不能用空函数或零大小数据代替。

模块内同名定义被拒绝，函数和全局之间也不能重名。Local 定义不能与本模块的普通声明或 Native 声明同名。普通声明可与同名同种类、同常量属性的 External 定义并存。

所有输入定义会先被收集，再处理声明和指令引用，因此解析支持前向引用和模块间循环引用。符号地址操作数不要求预先建立普通声明：若本模块没有同名 Local 定义或显式 Native 声明，可以直接找到全体输入中的 External 定义。普通声明用于额外约束目标种类。

第一版为严格最终链接：所有普通声明和指令中的 VM 符号必须解析成功。即使声明没有被指令使用，也不能留下未定义的普通声明。链接成功后已经解析的普通声明不再作为未解析依赖保留。

## Native 声明

`declareNativeFunction(LinkName, Import)` 和 `declareNativeData(LinkName, Import)` 分别声明宿主函数和宿主数据。LinkName 是 VM 名称，`Import.Name` 是宿主查找名，两者可以不同；未定义的普通符号不会自动变成 Native。

函数的宿主身份按 `Provider + Import.Name` 区分，数据按 `Import.Name` 区分。函数 Provider 使用 `Native/SystemApi`，调用类别使用 `C/InkNative`，数据没有 Provider。宿主函数和宿主数据可以同名，不同函数 Provider 也可以使用相同宿主名称；VM 链接名仍必须能无歧义解析。

各输入必须显式声明自己引用的 Native 符号。兼容的重复声明会合并；多个 VM 名称引用同一宿主身份时，输出选择统一的 VM 链接名并重写引用。同一宿主函数身份的 CallKind 不一致会报错；同一 VM 名称指向不同宿主身份、Native 函数和数据重名、Native 名称与 External 定义冲突也会报错。普通外部声明不能同时充当 Native 声明。

输出保留合并后的 Native 声明及逻辑符号引用。链接器不解析宿主地址，不生成函数导入索引或数据地址重定位，不为普通 VM 内存访问自动增加宿主访存能力。

## 深克隆与执行镜像边界

链接器创建新函数和全部新基本块，再通过旧基本块对象到新基本块对象的映射重建指令。前向、后向和自跳转均保留目标身份；匿名或重名的基本块不依靠名称匹配。输出的 parent 指针和跳转目标只指向输出对象。

全局变量保留声明大小、初始化字节、对齐、常量属性和插入顺序，未初始化尾部不展开为宿主缓冲区。各函数保留栈槽顺序、大小、对齐以及已布局或未布局状态。符号内存地址和符号立即数都会改写名称，`i64` addend 完整保留，不因指令位宽截断。

局部名称改写的实现链为 `assignLocalName → resolve → cloneOperand → MachineInstructionBuilder`：先为输出分配无冲突名称，解析时选择该输出名称，克隆 AddressOperand 或符号 ImmediateOperand 时构造新操作数，最后插入新的输出指令。源节点及源操作数保持原值，没有通过原位修改输入实现链接。

链接器不决定最终代码或数据地址，不计算基本块跳转的 PC 相对位移，也不选择镜像入口。后续流水线为：

```text
MachineModule × N → ModuleLinker → MachineModule → ExecutableEmitter → ExecutableFile
```

当前尚未提供 VM CALL、机器级函数签名和完整 ABI 校验，也未提供全局初始化字节内部的符号地址记录。跨模块函数调用、全局指针和函数指针表需要后续补充；第一版链接不会推测某段原始字节是否表示地址。所有输入由调用方保证采用兼容的 VM 与 ABI 配置。weak、COMDAT、部分链接、归档提取和不可达代码剔除也不在第一版范围内。

## 文本格式

[VM Module 文本序列化](VM-Module.md#符号操作数与文本序列化) 保留链接属性和全部声明，仍兼容省略 Local 属性的原有文本。例如一个输入声明并引用另一个输入定义的数据：

```text
.module "consumer"
.declare global @counter
.section .text
.function update external
entry:
    add64mmi [@counter], [@counter], 1
    ret
.endfunction
```

```text
.module "provider"
.section .data
.global counter, 8, 8 external
.endglobal
```

Native 声明使用独立的 VM 名称和宿主名称：

```text
.native_function @vm_log, native, c, @host_log
.native_function @vm_api, system_api, ink_native, @host_api
.native_data @vm_buffer, @host_buffer
```

文本读写只保留结构和元数据，不代替链接器的冲突、依赖与引用检查。
