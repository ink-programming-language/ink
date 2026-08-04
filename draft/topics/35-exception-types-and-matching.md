# 议题 35：异常类、异常接口与捕获匹配

> 状态：已确认，议题 36—43 与 Parser 议题 28 补充
> 确认日期：2026-08-02

## 1. `[exception]` 是内建属性

Ink 使用无参数内建属性 `[exception]` 声明能够参与异常体系的类和接口：

```ink
[exception]
class FileNotFound {
    path: String;
}

[exception]
interface IoError {
    func message() -> StringView;
}
```

`[exception]` 只能附着于 `class` 或 `interface`，不能附着于普通函数、字段、枚举、类型别名或局部声明。重复属性和带参数形式都是编译错误。

该属性是类型检查、异常 ABI 和编译期元数据的一部分，不是用户自定义元数据。

## 2. 异常类

只有异常类的完整对象可以作为新异常抛出：

```ink
throw FileNotFound { path };
```

一个类在以下任一情况下是异常类：

- 自身显式标记 `[exception]`；
- 继承自异常类。

异常资格沿唯一具体继承链自动传播，派生类不能移除：

```ink
[exception]
class IoFailure {
    code: int;
}

class FileNotFound : IoFailure {
    path: String;
} // 自动是异常类
```

直接标记的异常类仍遵守普通类的字段、构造、析构、复制和单一具体继承规则。普通具体父类本身不会因为具有异常派生类而变成可捕获类型。

普通非异常类不能被 `throw`，也不能作为 `catch` 类型。

## 3. 异常接口

异常接口必须在自身声明上显式标记 `[exception]`：

```ink
[exception]
interface RetryableError {
    func retry_after() -> Duration;
}
```

异常接口：

- 不能实例化；
- 不能作为一个新的异常对象直接 `throw`；
- 可以作为 `catch` 的分类目标；
- 继续遵守普通接口无实例字段、无构造函数和无析构函数的规则；
- 可以按照议题 29 提供默认方法；
- 可以多继承其他异常接口。

每个异常接口，包括继承其他异常接口的子接口，都必须显式写 `[exception]`。普通接口不能继承异常接口，异常接口也不能继承普通接口。

这使可捕获接口集合在声明处清晰可见，不会因为普通接口继承图变化而意外扩大异常匹配范围。

## 4. 类与接口的实现限制

只有异常类可以实现异常接口：

```ink
[exception]
interface IoError {}

[exception]
class NetworkTimeout : IoError {}

class Status : IoError {} // 编译错误：普通类不能实现异常接口
```

异常类可以同时实现普通接口，例如格式化、日志或序列化接口。但普通接口不能作为捕获类型：

```ink
[exception]
class NetworkTimeout : IoError, Displayable {}

catch Displayable as value { // 编译错误：Displayable 不是异常接口
}
```

异常类实现多个异常接口时，每个接口都是独立的可捕获分类，不在异常对象内增加接口表字段。

## 5. 合法捕获类型

`catch Type [as name]` 中的 `Type` 只能是：

- 异常类；
- 异常接口。

```ink
try {
    connect();
} catch NetworkTimeout as error {
    handle_timeout(error);
} catch IoError {
    record_io_failure();
}
```

Parser 议题 28 允许类型化处理器省略不需要的绑定。`catch Type { ... }` 与 `catch Type as name { ... }` 使用完全相同的类型匹配规则；前者只是不建立名称。

普通类、普通接口、枚举、裸指针和其他类型不能用作捕获类型。异常接口引用也不能作为一个新的异常值直接抛出；`throw` 需要具体异常类对象。

议题 39 规定一个类型化 `catch` 只能写一个异常类或一个异常接口，不接受类型列表、联合模式或运行时过滤条件。需要把多个异常归入同一处理器时，应声明共同的异常基类或异常接口。

## 6. 类匹配

异常类处理器匹配：

- 动态异常类恰好是处理器类型；
- 动态异常类沿唯一具体继承链派生自处理器类型。

匹配异常基类不要求异常对象自身具有 vptr。异常运行时通过对象外的最小异常描述符检查动态类和具体父类链。

普通父类即使位于异常对象的具体继承链中，也不能作为处理器类型；只有具有异常资格的类声明参与异常类匹配。

## 7. 接口匹配

异常接口处理器匹配动态异常类直接或间接实现的异常接口：

```ink
[exception]
interface IoError {}

[exception]
interface RetryableError {}

[exception]
class NetworkTimeout : IoError, RetryableError {}
```

同一个 `NetworkTimeout` 可以被两种接口处理器捕获。异常描述符保存去重后的异常接口闭包以及构造对应接口表所需的信息。

普通接口实现关系不会进入 catch 匹配集合，即使异常类确实实现了该普通接口。

## 8. 按源码顺序选择第一个处理器

运行时按照 `catch` 在源码中的顺序检查处理器，并执行第一个匹配项：

```ink
try {
    connect();
} catch RetryableError as error {
    schedule_retry(error.retry_after());
} catch IoError as error {
    log(error.message());
}
```

异常类实现多个异常接口时不定义接口优先级，也不按接口声明顺序选择；处理器源码顺序是唯一选择规则。

一个处理器执行后，同一 `try` 的后续处理器不再执行。处理器自身再次抛出的异常从外层处理区域继续查找，不回到同一处理器列表重新匹配。

议题 40 的 `cause` 记录不参与当前异常的处理器匹配。运行时只使用正在传播的最外层异常记录选择 `catch`；原因链只能由诊断器或显式 `ExceptionView` 查询访问。

## 9. 被遮蔽处理器诊断

编译器能证明某个处理器已经覆盖后续处理器的全部可能值时，对后续处理器产生 warning：

```ink
try {
    read();
} catch IoError as error {
    log(error.message());
} catch NetworkTimeout as error {
    // warning：如果 NetworkTimeout 实现 IoError，则该分支不可到达
}
```

可静态诊断的情况包括：

- 异常基类处理器位于其派生异常类之前；
- 异常接口处理器位于已知实现该接口的具体异常类之前；
- 父异常接口位于其子异常接口之前。

两个可能重叠但互不包含的异常接口不会仅因为存在某个同时实现它们的类而产生不可达 warning。

## 10. 捕获绑定

类型化处理器写出 `as name` 时，捕获绑定固定为运行时所拥有异常对象的只读普通引用：

```text
catch ConcreteError as error  -> error: const ConcreteError&
catch ErrorInterface as error -> error: const ErrorInterface&
```

无绑定的 `catch ConcreteError { ... }` 和 `catch ErrorInterface { ... }` 不创建隐藏引用或占位名称。

捕获不会复制或移动异常对象。议题 36 的异常记录在处理器执行期间保持载荷有效；绑定名称只在对应处理器作用域中可见。作为普通 `const T&` 或 `const Interface&`，引用值可以被返回、存入字段、全局、异步任务或逃逸闭包，但不会延长异常记录生命周期；处理器结束后通过该引用访问载荷属于 UB。

议题 38 确认语言不提供可变捕获形式。字段写入、取得可变引用、调用要求可变接收者的操作，以及通过反射写入载荷都不合法。

议题 43 的失败 `await` 使用共享 `ExceptionBox` 保存同一个只读载荷，但为每个等待者建立独立活动展开记录。类、父类和异常接口匹配仍读取载荷原有的动态描述符，因此与同步 `throw` 使用完全相同的处理器选择规则。

通过异常接口捕获时，绑定使用议题 26 的两字胖引用：

```text
{ exception object address, exception-interface table }
```

异常运行时保持对象所有权，接口绑定只借用它。接口表可以调用异常类实现或默认方法，不要求异常类对象具有 vptr。

## 11. 最小异常描述符

每个可抛出的动态异常类具有对象外的最小描述符，语义上至少提供：

- 动态异常类的规范化完全限定名称；
- 异常具体父类链；
- 去重后的异常接口集合；
- 各异常接口所需的接口表或构造信息；
- 当前模块和布局版本；
- 销毁异常对象所需的入口。

最小异常描述符不要求异常类具有 `[reflect]`，也不会自动生成用户可枚举的字段、方法或元数据表。类型同时使用 `[reflect]` 时，完整 `TypeInfo` 可以复用名称和版本数据，但两种功能在语言层面独立。

描述符、接口表和对象销毁代码必须在异常对象处理完成前保持加载有效。

## 12. 名称身份与热更新

异常类和异常接口遵守议题 22：

- 完全限定字符串名称是语义身份；
- 不提供稳定整数类型 ID；
- 描述符指针、哈希和进程内索引只是缓存；
- 重命名等于删除旧类型并创建新类型。

异常处理器只有在名称和模块布局版本允许建立安全类引用或接口引用时才能匹配。旧版本异常仍在展开或处理时，运行时必须固定其描述符、接口表和销毁入口；不能把旧对象直接解释为不兼容的新布局。

新增或删除异常接口实现会改变该异常类的捕获集合和接口表元数据，属于行为及异常 ABI 变化。

## 13. 成本模型

`[exception]` 不给普通类对象增加 vptr 或隐藏字段。其主要静态成本是：

- 最小异常类型描述符；
- 具体父类和异常接口关系表；
- 实际使用的异常接口表和 thunk；
- 名称及模块版本数据。

异常类处理器通常检查动态类或父类链；异常接口处理器查询异常接口集合并在匹配后构造两字接口引用。实现可以使用当前加载代次的描述符指针和缓存加速，但字符串名称仍是权威身份。

这些成本只影响异常元数据和真正的异常匹配，不给普通函数成功路径增加接口分派。议题 37 的 catch-all 不执行类或接口筛选；带绑定形式只建立一个类型擦除 `ExceptionView`。

## 14. 后续问题

以下内容留给后续议题：

- 跨动态库异常描述符和接口表的精确 ABI。
