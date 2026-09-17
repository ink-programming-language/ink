# Ink printf 与 AST 示例

`printf.ink` 按当前 `docs/grammar.bnf` 编写，`printf.ast.txt` 是当前 `ink-parse` 对该文件的实际输出。此次验证完成了 tokenize -> parse -> AST；没有完成语义编译、泛型实例化、编译期执行、目标 ABI 检查、链接或运行。

## 调用

```ink
printf::[int32, float]("hello, {} world {}", 1, CallOther(1,2));
```

示例中 `CallOther(Left, Right)` 返回 `float(Left) / float(Right)`。按示例的接口约定，在 C locale 下预期输出 `hello, 1 world 0.5`，末尾不自动添加换行。这是预期行为，不是当前 Ink 编译器的实测运行结果。

## 实现

- `func printf[Ts: type...](Format: const ptr uint8, Args: Ts...) -> int32` 使用泛型类型包和固定形状的普通参数包。
- `comptime printfFloatKinds::[Ts...]()` 只检查类型，生成一个浮点类型位图；它不读取 `Args`，也不执行输出。整个辅助调用处于编译期，所以循环里的 `Ts[Index]` 使用本次编译期执行中已知的索引。
- 运行时扫描格式字符串，把 `{}` 转成 `%d` 或 `%g`。`{{`、`}}` 输出字面括号；普通 `%` 转成 `%%`，避免将用户文字当成 C 格式说明符。
- `Args...` 直接展开给外部 C 变参函数 `snprintf`。`float` 按规则 49.4 所要求的目标 C ABI 提升为 `double`；Ink 函数本身不读取 `va_list`。
- 第一次 `snprintf` 查询所需长度，第二次填充精确分配的缓冲区，然后循环 `_write(1, ...)` 写标准输出并处理短写入。两次转发的是已求值的参数，按规则 7 的调用顺序，调用点的 `CallOther(1,2)` 只求值一次。
- `defer free(...)` 负责释放两个缓冲区，包括普通错误返回路径。返回成功写入的字节数，失败返回 `-1`；I/O 失败前可能已经输出部分内容。

本例支持 `int32`、`float`、`double`，最多 64 个值参数。64 的上限来自示例选用的 `uint64` 位图，是库实现限制，不是 Ink 参数包限制。浮点使用 C `%g` 默认精度；不实现对齐、宽度、具名字段或自定义类型格式化。缺少参数、多余参数和不匹配的括号均返回错误。格式字符串长度最多为 `INT32_MAX` 字节，最终格式化结果也受 C 函数的 `int` 返回范围限制。

## 示例的边界约定

这些约定用于描述这个库示例，不是向语言规范追加已确认规则：

1. 目标为 Windows x64：C `int` 对应 `int32`，`unsigned int` 对应 `uint32`，`size_t` 对应 `uint64`。`malloc`、`free`、`_write` 的 `void*` 缓冲区及 `snprintf` 的 `char*` 缓冲区在此用 `ptr uint8` / `const ptr uint8` 描述其字节访问。
2. 格式参数要求在整个调用期间有效、以 NUL 结尾的只读字节存储。为了保留上述直接传字符串字面量的用法，本例假定字面量可以适配这个 C 字符串接口；当前规范尚未完整确定该映射，parser 也不验证它。字符串中的内嵌 NUL 会结束格式扫描。
3. `_write` 使用标准输出描述符 1，要求它是有效的可写字节/普通文本流，不能是 CRT UTF-16 翻译模式。文本模式可能把换行转换为 CRLF；非 ASCII 的控制台显示还取决于输出环境。
4. 真正运行还需实现并验证字符串/指针边界、类型值比较、泛型与 comptime、C 变参默认提升和链接接口。`extern "C"` 只声明链接契约，不会由 parser 加载 CRT 或验证符号是否可链接；stdio 的实际导出或包装入口也需由目标链接配置解决。

## AST 结果

生成命令（仓库根目录）：

```powershell
.\cmake-build-debug\src\tools\parser\ink-parse.exe .\docs\examples\printf.ink
```

本次进程退出码为 `0`，stderr 为空，输出包含 `501` 个节点；`error=true` 和 `missing=true` 均为零。完整输出见 `printf.ast.txt`。

关键位置：

- 节点 458：`printf` 的 `FunctionDeclaration`。
- 节点 108：`Ts` 的 `GenericParameter`，`Variadic=true`。
- 节点 115：`Args` 的 `FunctionParameter`，`Variadic=true`。
- 节点 127：`ComptimeExpression`，其中 `Ts...` 记录为泛型实参的 `Pack=true`。
- 节点 362、407：两次 `snprintf` 调用，`Args...` 记录为普通实参的 `Pack=true`。
- 节点 489：示例的 `printf` 调用；callee 是节点 482 的 `GenericInstantiationExpression`，第三个普通实参是节点 488 的 `CallOther` 调用。

AST 保留 `comptime` 和泛型的源码结构；此阶段不会把 `printf` 展开成 `int32, float` 的实例，也不会求出 `CallOther` 的结果。

## 外部 API 依据

- [Microsoft：snprintf](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/snprintf-snprintf-snprintf-l-snwprintf-snwprintf-l?view=msvc-170)：签名、长度查询及截断语义。
- [Microsoft：_write](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/write?view=msvc-170)：签名、返回值及输出模式。
- [Microsoft：malloc](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc?view=msvc-170)、[free](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/free?view=msvc-170)：分配与释放契约。
