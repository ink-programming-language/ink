# 议题 11：位模式转换与指针转换

> 状态：已确认，议题 31、32、38 补充
> 确认日期：2026-08-01

## 1. 转换类别分离

Ink 使用不同的内建语法区分数值转换、位模式重解释和裸指针转换：

```ink
cast<u32>(value)          // 数值转换
bitcast<u32>(float_value) // 位模式重解释
ptrcast<Header*>(pointer) // 裸指针转换
```

三者均为不能被遮蔽或重载的编译器内建语法。它们不调用用户定义的构造函数。

## 2. `bitcast<T>(value)`

`bitcast<T>(value)` 原样复制源值的全部位，并把这些位解释为目标类型 T。

Ink v0 的 `bitcast` 只支持：

- 固定宽度整数；
- `int`、`uint` 和 `ptrsize`；
- `f16`、`f32` 和 `f64`。

源类型与目标类型必须具有完全相同的位宽：

```ink
const bits: u32 = bitcast<u32>(1.0f32);
const value: f32 = bitcast<f32>(bits);
```

以下操作不合法：

```ink
bitcast<u64>(1.0f32); // 编译错误：位宽不同
bitcast<bool>(1u8);   // 编译错误：bool 不参与 bitcast
```

裸指针、引用、切片、数组、类、议题 32 的带载荷枚举以及拥有资源的类型不参与 v0 的 `bitcast`。聚合类型的填充位和有效表示规则留给后续议题处理。

`bitcast` 不执行数值转换、范围检查或构造操作，正常降低为寄存器类别变化或无机器指令操作。

## 3. `ptrcast<T>(value)` 的输入与目标

`ptrcast` 用于以下显式转换：

- 任意对象裸指针类型之间转换；
- 对象裸指针转换为 `ptrsize`；
- `ptrsize` 转换为对象裸指针。

```ink
const bytes = ptrcast<byte*>(object_pointer);
const address = ptrcast<ptrsize>(object_pointer);
const restored = ptrcast<Object*>(address);
```

引用和安全切片不能直接作为 `ptrcast` 的输入或目标。函数指针、非默认地址空间指针和成员指针留给各自议题决定。

## 4. 指针转换语义

裸指针之间的 `ptrcast`：

- 保留地址位；
- `null` 转换后仍为 `null`；
- 不检查目标类型的对齐；
- 不检查该地址上是否存在目标类型对象；
- 不开始或结束对象生命周期；
- 不调用构造函数或析构函数；
- 不改变指向存储的实际可写性。

转换本身合法。通过结果指针访问内存时，仍必须满足议题 01 和议题 04 的地址、范围、对齐、对象表示、生命周期与并发前置条件。

```ink
const header = ptrcast<Header*>(bytes);
const value = header->kind; // 只有地址、对齐、生命周期和对象表示均有效时才合法
```

## 5. 指针与 `ptrsize`

对象裸指针转换为 `ptrsize` 时保留目标平台的全部地址位。

将该 `ptrsize` 值未经改变地转换回兼容的对象裸指针类型时，必须恢复相同地址，包括 `null`。

```ink
const address = ptrcast<ptrsize>(pointer);
const restored = ptrcast<byte*>(address);
```

对地址整数进行算术以后仍允许转换成裸指针，但语言不保证所得地址指向有效对象。持有和比较该指针合法；不满足原始内存访问前置条件的解引用可能产生 UB。

## 6. 允许显式去掉 `const`

`ptrcast` 允许增加或去掉裸指针目标类型的 `const` 限定：

```ink
const read_only: const Data* = pointer;
const writable: Data* = ptrcast<Data*>(read_only);
```

去掉 `const` 的转换本身合法且不产生 warning。`const` 限制当前类型所允许的直接访问，不保证底层存储事实上可写。

如果底层对象原本是可写对象，只是当前通过 `const T*` 观察，则去掉 `const` 后可以合法写入：

```ink
var data = Data {};
const view: const Data* = &data;
const writable = ptrcast<Data*>(view);
writable->field = 1; // 合法：底层对象可写
```

如果底层对象或存储本身不可写，去掉 `const` 后进行写入违反内存访问前置条件，并可能产生 UB：

```ink
const writable = ptrcast<Data*>(actually_read_only_storage);
writable->field = 1; // UB：转换合法，但写入不可写存储
```

编译器不能仅凭某个地址通过 `const T*` 传递，就假设该存储不会通过其他别名或显式 `ptrcast` 被修改。

议题 38 对运行时拥有的活动异常载荷建立了更强的对象级只读契约。`ptrcast` 仍可按照本节显式表示去除 `const` 后的裸指针，但通过该指针写入活动异常载荷违反异常运行时前置条件并产生 UB；这不构成语言支持的可变捕获。`ptrcast` 本身不会改变异常记录的所有权或只读状态。

## 7. 隐式限定转换

从 `T*` 到 `const T*` 的转换可以隐式发生，因为它只减少当前访问路径的写权限：

```ink
const writable: Data* = pointer;
const read_only: const Data* = writable;
```

从 `const T*` 到 `T*` 不隐式发生，必须显式使用 `ptrcast<T*>`。

不同对象类型的裸指针之间不隐式转换，即使它们具有相同布局。

## 8. 优化与成本

`bitcast` 和相同地址空间中的 `ptrcast` 不插入运行时有效性、范围或对齐检查。

编译器可以在不改变地址位或位模式的前提下消除转换本身。转换不会授权优化器假设目标对象存在，也不会凭空建立对齐、生命周期、唯一别名或底层可写性事实。

## 9. 与安全动态转换的区别

议题 31 定义的 `try_cast<T>(value)` 根据虚类或接口表关联的动态类型信息检查继承和接口实现关系。`ptrcast` 不执行这些检查，也不因为目标指针类型处于某条继承链上而变成安全下转型。

`ptrcast` 仍只接受议题 3 定义的裸指针和 `ptrsize`，不能把两字接口引用当作一个对象指针重解释。需要从接口视图取得具体类或其他接口时必须使用 `try_cast`。
