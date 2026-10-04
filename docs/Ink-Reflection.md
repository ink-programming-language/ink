# 类型描述与反射

实现集中在 `src/include/ink/execution/reflection/` 与 `src/lib/execution/reflection/`；AOT 表生成位于 `src/lib/backend/llvm/reflection/`。反射目前提供 C++ 宿主 API，没有增加 Ink 源码语法。

## 描述与对象存储

`StorageLayout` 只有 `Size` 和 `Alignment`。`TypeDesc` 关联类型域、类型 ID、种类、名字及目标存储属性，并共享专用描述。`IntegerDesc`、`FloatDesc`、`PointerDesc`、`FunctionDesc`、`ArrayDesc`、`ClassDesc` 继承 `TypeDetails`，不使用 `std::variant`。修改复制出的描述时会分离专用元数据，避免链接改写源镜像。

每个类型域内，每个 class 的实例、数组布局和帧共享一个 `ClassDesc`。其中 `Fields` 是有序 `FieldDesc` 列表，集中保存名字、类型、偏移、可见性、默认初始化函数 ID；`Methods` 保存名字、签名、函数 ID、可见性和接收者权限。它们不引用源码 AST 或 IR 对象。类型表按完整名字查询 class，也支持 `void`、`bool`、`i32`、`u8`、`f64` 等已登记的基础类型名。数字 ID 只在自己的类型域中有效。

对象、数组、字符串头和 VM 槽继续使用目标布局字节。对象内不增加类型头。`ObjectView` 在对象之外关联数据地址、类型表、类型 ID、可写权限，以及可选的受管存储身份。

## VM 宿主 API

创建 `Reflection(Types, Memory, &Machine)`；只访问数据时可省略执行机器。

- `view(Place)` 取得受管对象视图；`view(Type, span<byte>)` 和只读 span 重载用于外部内存。外部字节必须持续有效，且应是合法的该类型表示。
- `field`、`element` 取得字段或数组元素视图，后者检查索引范围。
- `read`、`write` 读写完整值；`getField`、`setField` 按名字访问字段。`value(RuntimeValue)` 明确把传入值绑定到本反射服务的类型域，跨域使用返回 `ForeignContext`。
- `invoke(Object, Name, Arguments)` 按实参类型精确选择公开方法，无隐式转换。多个匹配返回 `AmbiguousMember`；私有成员返回 `AccessDenied`；需要可写接收者的方法拒绝只读视图。
- `construct(Type, Arguments)` 对源码 class 按实参类型选择公开 `__init__`，分配对象后调用构造函数。字段默认值由构造函数执行；参数不再对应字段位置。没有可调用构造函数时返回失败。手工构建且没有生命周期方法的聚合 IR 继续支持底层字段构造 API。
- `destroy(Object)` 调用公开无参 `__del__`；调用方在析构成功后通过同一内存管理器释放存储。视图本身不自动拥有或析构对象，宿主负责每个成功构造的对象只析构一次。

写入复用内存管理器的类型、初始化与可写检查，并保留字符串载荷。受管视图在存储释放后报告 `ExpiredPlace`。类型表保留元数据生命周期，不延长对象生命周期；构造返回的 `ExecutionPlace` 由调用者通过同一内存管理器释放，管理器必须持续有效。

执行机器、反射服务和对象必须使用同一类型域。默认初始化函数的副作用遵循普通执行语义，后续初始化失败不会撤销已经发生的外部副作用。

## 编译与保存

IR class 保留方法归属和字段初始化函数；模块显式登记 class，确保没有被代码引用的类也能保存。IR 文本和二进制版本为 v7。字节码容器 v7 保存反射信息并验证签名、接收者、可见性、函数引用和预算；链接重映射类型 ID 与函数 ID。字节码指令模式仍为 v3。旧版产物需要重新生成。

私有方法可以被枚举，但反射访问接口不允许调用。消费模块可保留私有方法描述而不导入其函数实现。

## AOT 宿主 API

`native_reflection.h` 定义可直接放入原生目标文件的常量描述结构。公共 `NativeTypeDesc` 指向对应的专用描述，字段和方法仍采用相同的元数据含义。生成的描述和包装函数由公开 getter 引用，优化后继续保留。

每个编译单元导出 `_INK2J(Package, Module)` getter，返回 `const NativeModuleDesc *`；用 `ir::reflectionSymbol()` 获取准确名字。反射调用包装使用 `_INK2H(...)`，类身份使用规范的 `_INK2T(c(...))`，规则见 [符号规则](Ink-Symbol-Mangling.md)。表的 `Version` 当前仍为 1，C 描述结构布局不变。

宿主使用 `findNativeType`、`nativeField`、`nativeElement`、`nativeWrite`、`nativeInvoke`、`nativeConstruct` 和 `nativeDestroy`。`nativeConstruct` 按 `__init__` 参数规则调用源码构造函数；`nativeDestroy` 调用 `__del__`，缓冲区仍由宿主管理。`NativeObjectView` 包含模块表、类型 ID、字节地址、可用字节数和可写性；对象和结果缓冲由宿主提供。调用包装函数负责目标布局字节与原生函数参数、返回值的转换，包含嵌套聚合和 bool。

AOT 视图借用内存，字符串头复制后仍引用原载荷，宿主必须保持载荷有效。AOT 接口没有 VM 的受管分配失效检测与执行预算。调用产生的语言运行时错误沿用 AOT 的错误终止路径。
