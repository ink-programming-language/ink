# Ink IR 文本格式，版本 0.0.1

`ink/ir/serialization/serialization.h` 提供 `writeText(Module, Options)` 和 `readText(Context, Text, Options)`。文本版本由 `src/include/ink/core/config.in` 中的 `INK_IR_VERSION` 定义，经过 CMake 配置生成到 `ink/core/config.h`，当前值为 `"0.0.1"`。写入器使用该宏生成文件头，读取器只接受与宏完全相同的版本字符串，其余版本返回 `UnsupportedVersion`，不做版本范围匹配或迁移。Context 必须比返回模块及借用其中类型、常量的对象存活更久。

文本采用 LLVM 风格的函数、基本块、指令与引用，保留 Ink 的有符号类型、逐层 `const`、名义类型和模块语义，不能直接交给 LLVM 汇编器。二进制格式保持版本 1，编码和解码直接处理原有内部记录；文本符号分配、格式化及解析不进入二进制路径。

## 文档与作用域

```text
ink_ir 0.0.1
target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])
module ["demo"]
{
  define i32 @negate(i32 %value)
  {
  entry:
    %0 = neg i32 %value
    ret i32 %0
  }
}
```

`target` 保存指针位宽、端序、是否使用原生 ABI，以及 i8、i32、i64、i128、f64 的字节对齐。读取时全部与调用方的 Context 核对。模块路径是字符串组件列表，空列表表示没有模块标识。可选的 `module [...] [init = @init, deinit = @deinit]` 保存初始化和销毁入口，省略表示不存在。

- `@name` 引用模块级函数、全局变量或导入符号；`%Type` 引用名义类型。这两类符号各有独立的模块级作用域。
- 参数、指令结果和基本块共用各自函数的局部符号作用域。基本块定义写为 `entry:`，分支引用写为 `label %entry`。
- 允许前向引用，包括后续函数、名义类型和基本块。局部值不能跨函数引用；引用仍须符合类型、所有权和 SSA 支配约束。
- 匿名实体使用数字符号，例如 `@0`、`%0`、`0:`，与内部记录 ID 独立。函数之间的局部编号独立。
- 有名字的实体优先使用原名。重名时分配确定性的 `.1`、`.2` 等后缀，通过 `[name = "原名"]` 保留原始名字；同名同布局的名义类型不会因此合并。
- 任意名字可以使用引号，例如 `@"a b"`、`%"0"`；带引号的数字是实际名字，无引号数字是匿名编号。指令临时符号只用于文本引用，现有 IR 中实际保存的名字仍限于支持命名的实体。

规范输出保留各类声明、字段、方法、基本块、指令、导入和导出的原有顺序。读取后重新输出得到规范文本；手写名字、属性顺序等可能被规范化。

## 类型

| 语法 | 含义 |
|---|---|
| `bool`、`void`、`nullptr` | 布尔、空返回值、空指针字面量类型 |
| `i32`、`u32` | 有符号、无符号整数；位宽按 IR 的支持范围指定 |
| `f32`、`f64` | IEEE 浮点 |
| `*T`、`&T` | 指针、引用 |
| `[14 x u8]` | 定长数组 |
| `fn(i32, u8...) -> void` | 函数类型；`...` 是对应参数的变参标记 |
| `%Node` | 名义类型声明的身份 |
| `meta`、`variadic` | 编译期类型标记 |
| `const T` | 只限定紧随其后的类型层 |

`*const u8` 是指向 const 字节的指针；`const *u8` 是指针自身不可变。`const [14 x u8]` 与 `[14 x const u8]` 也不同。普通结构类型直接内联，不单独分配类型记录号；递归的类、接口和结构体通过名义类型符号引用。

## 常量与操作数

操作数通常写为 `类型 值`，例如 `u32 14`、`*void %handle`、`nullptr null`。引用旁的类型必须与被引用值一致；能够作为函数实参或初始化值使用的转换关系仍由 IR 校验器检查。

| 常量 | 规范形式 |
|---|---|
| 布尔 | `bool true`、`bool false` |
| 不超过 64 位的整数 | `i32 -11`、`u32 4294967285` |
| 更宽的整数 | `u128 0xFEDCBA9876543210FFFFFFFFFFFFFFFF`，按完整位模式保存 |
| 浮点 | `f64 bits(0x7FF8000000000123)`，保留负零和 NaN payload |
| 空指针 | `nullptr null` |
| 字符串 | `[14 x u8] c"hello, world\0D\0A"`，不附加隐式 NUL |
| 数组 | `[2 x u32] [u32 1, u32 2]` |

`none` 只表示缺省的可选初始化值，不能代替空指针常量或必需操作数。

十进制输入须符合类型的有符号性及数值范围，绝对值最多为 `18446744073709551615`；更大的整数使用十六进制位模式，避免十进制逐位转换的开销。十六进制不接受负号，例如 `i8 0xFF` 表示位模式全一，规范输出为 `i8 -1`。

## 声明

外部函数使用 `declare`，含基本块的函数使用 `define`；参数直接写在签名中。`import` / `export` 指定函数链接方式，缺省为 local；`"C"` 指定 C 语言链接，缺省为 Ink。

```text
declare import "C" *void @GetStdHandle(u32 %StandardHandle)

define i32 @main()
{
entry:
  %StdOutputHandle = alloca const u32 [initialized]
  store u32 4294967285, *const u32 %StdOutputHandle
  %0 = load u32, *const u32 %StdOutputHandle
  %1 = call *void @GetStdHandle(u32 %0)
  ret i32 0
}
```

函数末尾的可选属性包括 `private`、`owner = %Node`、`virtual` / `abstract`、`override`、`final`、`slot = 3`、`signature_const` 和 `name = "原名"`。缺省分别为 public、无所有者、direct、false、false、未分配槽位、可变函数签名及从符号推导的名字。槽位 `4294967295` 仍然有效；未分配时省略，不打印整数哨兵。参数和基本块也可使用 `name` 属性。

```text
%Node = type struct [size = 16, alignment = 8, frozen]
{
  field *const %Node "next" [offset = 0] = nullptr null
  field u64 "value" [offset = 8] = u64 42
}

@counter = global *u32 = u32 0
@0 = global *const [14 x u8] = [14 x u8] c"hello, world\0D\0A" [string_storage]

import @1 : fn() -> void from ["library", "entry"]
export @counter as "counter"
export type %Node as "Node"
```

名义类型种类为 `struct`、`class` 或 `interface`，显式保存 size、alignment、字段 offset；省略 `frozen` 表示尚未冻结。字段和全局初始化值可为 `none`。全局变量 `global` 后写地址类型，等号后写带类型的初始化值，因此地址本身和被指向对象的限定都能保留。`string_storage` 标识池化字符串存储，省略为 false。

方法用普通函数语法加 `owner` 属性表示。模块导入的别名仅用于文本引用，`from` 列表保存模块路径及最后的符号名。模块 `export` 映射独立于函数的链接方式与可见性，支持同名的多个导出；类型导出使用 `export type T as "名字"`。

## 指令

| 指令 | 语法 |
|---|---|
| 分配 | `%p = alloca T [initialized]` |
| 加载 | `%v = load T, *T %p` |
| 存储 | `store T %v, *T %p` |
| 取负 | `%v = neg T %operand` |
| 比较 | `%v = icmp eq T %left, T %right` |
| 分支 | `br label %target` |
| 条件分支 | `br_if bool %condition, label %yes, label %no` |
| 返回 | `ret void` 或 `ret T %value` |
| 调用 | `%v = call T @callee(T %argument)` |
| 元素地址 | `%p = getelementptr SourceType, PointerType %base, IndexType 0, IndexType 0 [inbounds]` |

`store`、分支和 `ret` 不绑定结果；普通 void 调用也省略结果名。Ink 允许将 void 调用结果作为 unit 实参，因此被后续指令引用的 void 调用使用 `%n = call void ...` 保留引用身份。`icmp` 谓词为 `eq`、`ne`、`ugt`、`uge`、`ult`、`ule`、`sgt`、`sge`、`slt`、`sle`。

`alloca` 的类型是被分配对象的类型；`initialized` 保存声明初始化记录，不表示自动清零，也不是控制流上的确定初始化。省略为 false。分配指令支持 `name` 属性保存原始变量名。

调用的命名实参写为 `T %value [name = "参数名"]`。直接调用且签名与目标函数完全一致时省略签名；间接调用或需要保留独立签名时显式附加 `[signature = fn(...) -> T]`。GEP 显式保留源类型及非缺省 `inbounds` 标记；结果指针类型由源类型和索引推导，并沿所选数组元素或字段传播 `const` 限定，不单独标注。此推导仅用于文本解析，二进制记录继续直接保存结果类型。

## 词法、校验与资源限制

空白不影响语法，`;` 开始行注释。字符串使用按字节的 `\HH` 十六进制转义，例如 `\00`、`\22`、`\5C`、`\0A`；不使用 C 风格的 `\n` 或 `\"`。规范输出直接保留可打印 ASCII，其他字节使用大写十六进制转义；输入中的原始 UTF-8 字节被保留。控制字节必须转义，字符串不做 Unicode 规范化。

未知或重复属性、未定义或重复符号、错误的类型注解和缺少的必需内容都会失败。读写返回 `IRSerializationResult<T>`，词法和语法错误尽可能携带字节偏移；不会依赖异常或输出日志。

`MaxBytes` 限制输入和输出大小；`MaxRecords`、`MaxOperands`、`MaxStringBytes` 限制解析时的内部记录、操作数和字符串；`MaxDepth` 限制嵌套类型和常量深度。函数中的普通线性指令数量不作为递归深度计算。

文本解析先建立符号与内部记录，再执行共用的重建和 IRVerifier 校验。错误输入不会交付半成品模块，重建失败会回滚本次加载增加的池化 IR 对象；成功模块遵循 Context 生命周期约定。
