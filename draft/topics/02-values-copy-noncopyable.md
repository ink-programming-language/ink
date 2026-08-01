# 议题 02：值、复制与不可复制类型

> 状态：已确认
> 确认日期：2026-08-01

## 1. 不提供通用移动语义

Ink 不提供以下机制：

- `move(value)` 表达式；
- 移动构造函数；
- 移动赋值函数；
- 普通函数调用隐式消费调用方变量；
- “已移动但尚未重新初始化”的变量状态；
- 根据最后一次使用自动移动变量。

普通函数调用之后，实参绑定不会仅仅因为按值或按引用调用而变成不可使用。

```ink
func inspect(file: const File&) {
    // 临时借用
}

var file = try File.open("data.txt");
inspect(file);
file.read(); // 合法，inspect 不会消费 file
```

## 2. 按值操作表示复制

对命名值执行按值赋值、按值传参或其他按值使用时，其语义是复制。只有可复制类型允许这些操作。

```ink
let first: int = 10;
let second = first; // 复制
print(first);       // first 仍然有效
```

不可复制值不能从命名变量按值传递：

```ink
func submit(file: File) {
}

var file = try File.open("data.txt");
submit(file); // 编译错误：File 不可复制
```

应当改为传递引用或指针：

```ink
func inspect(file: const File&) {
}

inspect(file);
file.read(); // 仍然有效
```

一个没有独立源对象的构造表达式可以直接初始化最终目标，包括局部变量、字段、返回位置和参数位置。这属于保证的原地构造，不是移动。

```ink
func create_server() -> Server throws {
    return Server {
        listener: try Socket.listen(8080),
        log: try File.open("server.log"),
    };
}

var server = try create_server(); // 直接构造在 server 的最终存储中
```

对于不可复制类型，不能保证原地构造的代码必须被拒绝，不能退化为隐藏移动。

## 3. `[noncopyable]` 属性

`[noncopyable]` 是无参数的编译器内建语义属性，不是关键字。

```ink
[noncopyable]
struct File {
    handle: OsHandle;

    func destructor(this: File&) {
        os.close(this.handle);
    }
}
```

该属性：

- 禁止类型的隐式复制和复制赋值；
- 禁止从命名变量按值传参；
- 不改变类型的内存布局；
- 可以用于 `struct`、`enum` 和 `newtype`；
- 是公开 API 的组成部分；
- 会进入类型元数据和 API 摘要；
- 不能被下游代码移除或覆盖。

可复制能力按照以下规则确定：

```text
copyable(T) =
    T 没有 [noncopyable]
    && T 没有析构函数
    && T 的所有字段或枚举载荷都可复制
```

直接声明析构函数的类型必须显式标记 `[noncopyable]`，否则是编译错误。这可以防止资源类型在重构过程中意外获得复制能力。

包含不可复制字段或载荷的外层类型自动不可复制，无须层层添加属性：

```ink
[noncopyable]
struct File {
    // ...
}

struct Server {
    log: File;
    port: u16;
}

// Server 自动不可复制
```

外层类型仍可显式添加 `[noncopyable]`，以保证它在将来删除不可复制字段后也不会意外变成可复制类型。

## 4. `Copy` 与显式克隆

基础数值、布尔、字符、原始指针和函数指针是可复制类型。

数组、元组、结构体和枚举在全部组成部分都可复制，且自身没有 `[noncopyable]` 和析构函数时自动可复制。

需要执行以下工作的复制不能隐藏在赋值或按值传参中：

- 分配内存；
- 复制外部资源；
- 增加引用计数；
- 调用可能失败的系统接口；
- 执行任意用户代码。

深复制使用普通的显式 `clone()` 函数：

```ink
var first = String.from("hello");
var second = first;         // 编译错误：String 不可复制
var second = first.clone(); // 显式深复制
```

`clone()` 不会使类型获得可复制能力，也不参与赋值和按值调用的隐式转换。

泛型可以通过 `Copy` 约束要求可复制类型：

```ink
func duplicate<T>(value: const T&) -> (T, T)
where T: Copy {
    return (value, value);
}
```

类型从可复制变为不可复制，或者从不可复制变为可复制，都是公开源码 API 的行为变更。

## 5. 引用、长期地址和程序员责任

`T&` 与 `const T&` 用于短期、非拥有访问。为了使安全代码保证成立，普通引用不能逃逸其允许的调用或词法范围：

- 不能存入全局变量；
- 不能存入长期存在的对象字段；
- 不能被逃逸闭包或异步任务保存；
- 不能作为未建立明确来源关系的普通返回值返回。

需要长期保存对象地址时使用原始指针。保存、复制和访问原始指针不需要特殊关键字；程序员负责满足地址和生命周期前置条件。

```ink
func submit(file: File*) {
    task_queue.store(file);
}

var file = try File.open("data.txt");
submit(&file);
```

调用方必须保证 `file` 比后台任务活得更久。违反该契约属于程序错误，并可能产生 UB。旧地址可能立即触发故障，也可能仍然映射到其他对象；Ink 不承诺悬空指针一定以崩溃结束。

## 6. 容器中的不可复制对象

一般的连续动态值容器 `Vector<T>` 只直接保存可复制元素。不可复制且具有对象身份的资源对象应当通过指针保存，而不是作为会随容器扩容而搬迁的内联值。

```ink
var numbers: Vector<int>;   // 合法
var files: Vector<File>;    // 编译错误：File 不可复制
var files: Vector<File*>;   // 合法，保存非拥有指针
```

`Vector<File*>` 只拥有指针数组，不拥有指针指向的 `File`，销毁容器不会关闭或释放这些文件。

标准库可以提供拥有型指针容器，例如概念上的 `PtrVector<T>`：

```ink
var files: PtrVector<File>;
```

它在内部保存单独分配的 `File*`，扩容时只复制指针，并在容器销毁时逐个销毁和释放对象。确切名称、分配器接口和失败语义属于标准库设计，尚待讨论。

不可复制类型仍然可以作为：

- 局部变量；
- 结构体字段；
- 固定长度数组元素；
- 由稳定地址容器管理的对象；
- 通过保证原地构造产生的返回值。

包含不可复制元素的聚合体自身自动不可复制。
