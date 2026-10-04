# 数组与下标访问

Ink 支持定长数组的构造、读取、写入、取元素地址、按值传参和返回；源码解释执行与字节码执行使用相同语义。通用切片、区间切片、指针算术和数组解构尚未接入。

## 声明和构造

```ink
func main(): i32
{
  var Values = [10, 20, 30];
  var Bytes: [u8; 16] = [0; 16];
  var Matrix: [[i32; 2]; 2] = [[1, 2], [3, 4]];
  var Empty: [u8; 0] = [];
  Values[1] = 42;
  Bytes[0] = 65;
  return Values[1];
}
```

类型写为 `[T; N]`，普通字面量写为 `[A, B, C]`，重复字面量写为 `[Value; N]`。`N` 必须是非负的编译期整数，类型包含元素类型和固定长度；不同长度、位宽或符号属性的数组属于不同类型。数组元素不能是 `void` 或 `type`。

CLI 和 `analyzeModules()` 会先预声明全部函数签名，再执行模块初始化。函数签名中的数组长度必须在预声明时即可求值，例如 `16` 或 `2 + 2`；当前不能引用模块级 `comptime` 变量或常量，也不能依赖尚未完成的函数体。函数体内的数组声明可以使用此前已初始化的编译期绑定。源码顺序入口 `analyze()` 的环境不同，不应据此假定 CLI 已支持模块绑定参与签名长度计算。

无其他类型约束的整数元素推导为 `i32`；显式声明、唯一被调用函数的形参和函数返回类型可提供元素类型。已经具有类型的元素保持其类型，不隐式改变位宽或符号。`[]` 需要 `[T; 0]` 上下文；`[0; 0]` 可从值推导为 `[i32; 0]`。零长度数组可以存储和传递，但没有合法元素下标。

普通字面量按从左到右的顺序求值，每个元素求值一次。重复字面量只求值一次 `Value`，再复制该值；即使 `N` 为零，也求值一次 `Value`。数组长度求值发生在语义分析阶段，不随函数每次运行重新求值。构造长度还受语义物化上限限制：按照运行时元素表示的大小和嵌套数组长度估算所需空间，不得超过 `INK_EXECUTION_MAX_STORAGE_BYTES`，超限报告 `INK-S0055`；实际执行仍受对象数、存储字节等预算限制。

## 访问、复制和地址

```ink
func Make(Value: i32): [i32; 2]
{
  return [0, Value];
}

func Read(Values: [i32; 2]): i32
{
  return Values[1];
}

func main(): i32
{
  var Original = Make(42);
  var Copy = Original;
  var Element = &Original[1];
  *Element = 10;
  return Read(Copy);
}
```

该程序返回 `42`。数组赋值、参数传递和返回均采用值语义；修改 `Original` 不会改变 `Copy`，嵌套数组也如此。若元素本身是指针，复制的是指针值，两个指针仍引用同一目标。整个数组重新赋值保留其存储身份，因此已有的元素地址能观察后续写入。

`Values[Index]` 的下标可以是任意受支持位宽的整数。可变数组的下标表达式在元素读取之前求值；赋值时目标地址和下标在右侧之前各求值一次。`Matrix[Row][Column]` 支持逐层访问，`Make(42)[1]` 支持读取临时数组。`(*PointerToArray)[Index]` 支持通过数组指针访问元素。

`const` 数组可读，不能写入元素或取得允许写入的元素地址。临时数组和按值形参不能直接取得可写地址。数组必须整体初始化后才能访问元素；`var A: [i32; 2]; A[0] = 1;` 不作为部分初始化支持，可先写 `A = [0; 2]`。元素指针保存原始机器地址，不延长数组生命周期；函数返回后解引用其局部数组元素地址属于未定义行为，不保证产生失效诊断。

合法下标范围是 `[0, N)`。编译期可知的负数和越界下标报告 `INK-S0054`；运行期下标执行边界检查，越界报告 `INK-E0025`，读、写和取地址使用相同检查。非整数下标报告 `INK-S0053`，非法数组长度报告 `INK-S0051`，无上下文的空数组报告 `INK-S0052`。

## 编译期数组

```ink
comptime var Values = [[1, 2], [3, 4]];
comptime
{
  Values[1][0] = 40;
}

comptime func Make(): [i32; 2]
{
  var Values = [0, 0];
  Values[1] = 42;
  return Values;
}

func main(): i32
{
  var Frozen = comptime Make();
  return Frozen[1];
}
```

编译期绑定支持数组构造、复制、下标读取及嵌套元素赋值。编译期函数可以操作局部数组，将返回数组冻结为常量后用于运行时初始化。直接取得 AST 编译期绑定的地址仍不允许；这与已有标量编译期绑定规则相同。

## 接入 `read`

初始化的可变 `u8` 数组提供连续原生缓冲区。使用 `&Buffer[0]` 可将首元素地址传入宿主 `read` / `_read`；也可以用 `&Buffer[Offset]` 从内部位置开始写入。外部函数写入后，后续 Ink 下标读取立即看到新字节，先前复制的数组保持原值。

以下 Windows x64 示例通过 CRT `_read` 从标准输入读取最多 16 字节，并输出第一个已读取字节。`_read` 的计数是 32 位无符号整数，返回值是 32 位有符号整数。

```ink
import "C" func _read(Descriptor: i32, Buffer: *u8, Count: u32): i32;
import "C" func _write(Descriptor: i32, Buffer: *u8, Count: u32): i32;

func main(): i32
{
  var Buffer: [u8; 16] = [0; 16];
  var Count = _read(0, &Buffer[0], 16);
  if (Count < 0)
  {
    return 1;
  }
  if (Count > 0)
  {
    _write(1, &Buffer[0], 1);
  }
  return 0;
}
```

Linux x86-64 使用下面的声明并将示例中的 `_read` / `_write` 替换为 `read` / `write`。这里 `size_t` 为 `u64`，`ssize_t` 为 `i64`；其他宿主 ABI 需要对应的类型声明。

```ink
import "C" func read(Descriptor: i32, Buffer: *u8, Count: u64): i64;
import "C" func write(Descriptor: i32, Buffer: *u8, Count: u64): i64;
```

将源码保存为文件并执行 `inkc --interpret -i read.ink`，标准输入即可供程序读取。不要在此模式下同时用 `-i -` 把同一标准输入作为源码。

读取返回值可能小于请求数量；`0` 表示 EOF，负值表示失败。调用者必须保证请求字节数不超过从传入元素到数组结尾的剩余容量；原生 C 函数不会自动获得 Ink 的数组长度。`read` 不追加字符串终止 NUL，应使用实际返回的字节数解释有效内容。Windows CRT 文本模式仍遵循宿主的换行转换规则。

## 测试

`SemanticArrayTest.*` 覆盖类型推导、嵌套数组、值复制、参数和返回值、求值次序、编译期求值、指针别名与生命周期、静态和动态边界。IR 与执行模块分别测试数组指令、常量、存储和归档。

`src/testcase/execution/programs` 提供可单独执行的数组回归程序，每个检查输出用例名和 `PASS`，失败输出 `FAIL` 并返回非零退出码：

| 程序 | 覆盖内容 |
| --- | --- |
| `arrays_basics.ink` | 类型推导、上下文类型、布尔元素、重复构造、嵌套和空数组 |
| `arrays_widths.ink` | 8/16/32/64/128 位有符号与无符号元素、下标和指针写入 |
| `arrays_values.ink` | 深层值复制、整体赋值、元素别名、参数、返回值和数组指针 |
| `arrays_effects.ink` | 构造顺序、重复值只求值一次、零长度副作用、索引与右值顺序、传参快照 |
| `arrays_comptime.ink` | 编译期嵌套写入、复制、复合赋值、长度、返回值冻结和副作用 |
| `arrays_read.windows.ink` / `arrays_read.linux.ink` | Windows `_read` / Linux 64 位 `read`、内部地址、短读、零字节请求、EOF、原生写入与值复制 |

`read` 程序需要将同目录 `arrays_read.input` 的 6 字节 `ABCDEF` 作为标准输入；该文件没有结尾换行。其他程序可直接用 `inkc --interpret -i <文件>` 运行。

`ExecutionSourceTest.Array.*` 自动核对完整 stdout、stderr 和退出码。成功程序及运行时错误用例还会生成对象文件、链接并重新加载字节码，再执行同样检查；每次执行单独打开输入文件。输入沿用 `src/testcase/execution/cli/inputs` 目录约定，验证错误码、动态边界及仅保存或丢弃逃逸地址；不对悬空地址进行解引用。

`ArrayReadProcessTest` 另验证真实标准输入、重复加载字节码与动态边界诊断。可运行：

```sh
ctest --test-dir build -R 'ExecutionSourceTest.Array|SemanticArrayTest|ArrayReadProcessTest' --output-on-failure
```

多配置构建还需指定 `-C Release` 或所用配置。
