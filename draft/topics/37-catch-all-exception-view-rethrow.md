# 议题 37：catch-all、`ExceptionView` 与重新抛出

> 状态：已确认，议题 38—43 补充  
> 确认日期：2026-08-02

## 1. 两种 catch-all 形式

Ink 提供不绑定异常和绑定类型擦除视图的两种 catch-all：

```ink
try {
    run();
} catch {
    recover_without_inspection();
}
```

```ink
try {
    run();
} catch as error {
    log(error.type_name());
}
```

语义分别是：

```text
catch          -> 匹配全部 Ink 异常，不建立绑定
catch as name  -> 匹配全部 Ink 异常，name: const core.ExceptionView&
```

`as` 在这里与 `catch Type as name` 一样表示处理器绑定，不表示类型转换。

catch-all 只匹配议题 34 定义的 Ink 语言异常。它不把原始指针 UB、PDB、trap、`abort`、硬件故障或进程终止转换成可捕获异常。

## 2. 处理器顺序

处理器继续按照议题 35 的源码顺序选择第一个匹配项：

```ink
try {
    run();
} catch IoError as error {
    handle_io(error);
} catch as error {
    log(error.type_name());
}
```

`catch {}` 和 `catch as name` 都覆盖全部 Ink 异常，因此位于它们之后的任何处理器都静态不可达。编译器按照既定规则产生 warning：

```ink
try {
    run();
} catch {
    recover();
} catch IoError as error {
    // warning：该处理器不可达
}
```

语法不强制 catch-all 必须写在最后，但后续处理器不会执行。

议题 39 不允许给 catch-all 或类型化处理器附加过滤表达式，也不允许一个处理器列出多个异常类型。

## 3. `core.ExceptionView` 的定位

`core.ExceptionView` 是编译器与核心运行时共同提供的只读、非拥有、不可复制类型擦除视图。它不是所有异常类的公共基类，也不是异常接口。

议题 38 将该只读性质确定为永久语言规则，而不是第一版的默认选择；不存在可变 `ExceptionView` 或从它取得可写异常载荷的安全 API。

用户不能直接构造 `ExceptionView`，不能从普通对象伪造它，也不能通过它取得 `ExceptionRecord`、载荷裸指针或运行时私有描述符指针。只有 `catch as name` 可以建立当前活动记录的合法视图。

概念表示为：

```text
ExceptionView
    → active ExceptionRecord
        → descriptor
        → payload
```

物理实现通常只需要一个记录地址，并且可以被编译器保持为 SSA 值；该表示不是稳定的用户 ABI。

## 4. 最小通用 API

第一版 `ExceptionView` 保证以下只读查询：

```ink
[nothrow]
func type_name() -> StringView;

[nothrow]
func matches[T]() -> bool;

[nothrow]
func reflection() -> Optional<TypeInfo>;

[nothrow]
func cause() -> Optional<const ExceptionView&>;

[nothrow]
func throw_site() -> ThrowSiteView;

[nothrow]
func traceback() -> TracebackView;
```

这些方法是核心类型或核心运行时 API，不由每个异常类分别实现，也不通过异常对象虚函数分派。

`ExceptionView` 不提供统一的 `message()`、`code()` 或任意异常字段。不同异常需要的业务行为应通过类型化 `catch` 或异常接口表达；`reflection()` 只提供可选类型元数据，不自动暴露当前载荷。议题 40 的 `cause()` 只返回直接原因的只读类型擦除视图。

## 5. `type_name()`

`type_name()` 返回动态异常类的规范化完全限定名称，与议题 22 的字符串身份一致：

```ink
catch as error {
    log(error.type_name());
}
```

概念实现为：

```text
error.record.descriptor.qualified_name
```

该操作：

- 不要求异常类具有 `[reflect]`；
- 不搜索全局动态反射注册表；
- 不分配或复制异常对象；
- 不调用异常对象的虚函数；
- 不把描述符指针、哈希或模块版本编号暴露为稳定类型身份。

返回的 `StringView` 借用当前活动描述符中的名称存储。它继承 `ExceptionView` 的短期不逃逸限制，只能在当前处理器及其同步调用中使用，不能保存到全局、长期字段、逃逸闭包或异步任务。

## 6. `matches[T]()`

`matches[T]()` 使用与 `catch T as name` 完全相同的匹配规则：

```ink
catch as error {
    if error.matches[IoError]() {
        count_io_failure();
    }
}
```

`T` 只能是议题 35 定义的异常类或异常接口；其他类型产生编译错误。匹配包括：

- 动态异常类恰好是 `T`；
- 动态异常类派生自异常类 `T`；
- 动态异常类实现异常接口 `T`。

该查询只返回分类结果，不把载荷转换成可访问的具体引用。需要读取字段或调用接口方法时，应优先在 catch-all 之前使用类型化处理器。

`matches[T]()` 可以使用当前加载代次的描述符指针、哈希或索引加速，但语义身份仍以完全限定字符串名称及兼容模块布局为准。

## 7. `reflection()`

`reflection()` 只在动态异常类显式生成了议题 19 的动态反射信息时返回 `some(TypeInfo)`；否则返回 `none`：

```ink
catch as error {
    if let .some(type) = error.reflection() {
        inspect(type);
    }
}
```

返回的 `TypeInfo` 必须对应异常记录固定的同一模块和布局版本。运行时不能只使用 `type_name()` 在当前全局注册表中查找最新同名类型，再用新描述符解释旧异常载荷。

该查询用于检查类型、声明和用户元数据，不把异常载荷自动转换成 `DynamicRef`，也不授予字段读写权限。反射句柄不能逃逸当前异常处理生命周期。没有 `[reflect]` 的异常仍然可以使用 `type_name()`、`matches[T]()` 和普通异常匹配。

`cause()` 不要求当前异常或原因异常具有 `[reflect]`。它在存在直接原因时返回 `some(const ExceptionView&)`，否则返回 `none`；返回引用继承当前处理器的短期不逃逸限制。完整原因链语义由议题 40 规定。

议题 41 的 `throw_site()` 始终返回当前记录的轻量抛出位置视图；`traceback()` 返回当前配置实际捕获的只读栈帧视图，完整捕获关闭或失败时可以为空。二者都不要求 `[reflect]`，也不能逃逸异常处理生命周期。

议题 42 的未捕获异常观察钩子接收同一个只读 `ExceptionView`。该视图只在钩子调用期间有效；钩子返回后程序仍然终止。

议题 43 不提供从普通 `ExceptionView` 到拥有型 `ExceptionBox` 的用户转换。只有任务边界可以接管运行时已经拥有的活动记录；对失败任务执行 `await` 后，`catch as error` 仍只建立当前展开记录的短期 `ExceptionView`，不能据此延长 box 或载荷的生命周期。

## 8. 通用消息不是内建能力

Ink 不强制所有异常实现统一的 `message()`。需要可显示异常时，程序可以声明显式异常接口：

```ink
[exception]
interface DisplayError {
    func message() -> StringView;
}

try {
    run();
} catch DisplayError as error {
    log(error.message());
} catch as error {
    log(error.type_name());
}
```

这样不需要为所有异常对象增加公共基类、vptr 或无意义的消息字段，也不会把普通格式化接口自动变成可捕获分类。

## 9. `throw;` 重新抛出当前异常

`throw;` 表示重新抛出词法上最近一层正在处理的异常：

```ink
try {
    run();
} catch IoError as error {
    log(error.message());
    throw;
}
```

它只在 `catch` 处理器的当前函数体上下文中合法。在普通函数、处理器之外或处理器内部声明的另一个可独立调用函数中使用时产生编译错误；调用普通辅助函数不会把“当前异常”作为隐式动态状态传给该函数。

嵌套处理器中，`throw;` 选择最近的处理器：

```ink
try {
    outer();
} catch OuterError as outer_error {
    try {
        inner();
    } catch InnerError as inner_error {
        throw; // 重新抛出 InnerError
    }

    throw; // 重新抛出 OuterError
}
```

## 10. 重新抛出复用记录

`throw;` 复用议题 36 的当前 `ExceptionRecord`：

- 不分配新记录；
- 不复制或移动载荷；
- 不要求异常类可复制；
- 保留原始动态类、接口集合、描述符和模块版本；
- 清理当前处理器中已经构造的局部对象；
- 从当前处理区域之外继续搜索处理器，不重新匹配同一个处理器列表。

`throw error;` 不是重新抛出。它表示创建一个新异常，并按照议题 36 从命名变量按值复制；`ExceptionView` 不是异常类，因此以下代码始终非法：

```ink
catch as error {
    throw error; // 编译错误：ExceptionView 不能作为异常对象抛出
}
```

## 11. 处理完成与生命周期

处理器正常完成且没有执行 `throw;` 时，当前异常被视为已处理。运行时随后按照议题 36：

1. 使所有捕获引用和 `ExceptionView` 失效；
2. 调用最派生异常对象的 `destroy_entry`；
3. 释放异常记录存储；
4. 解除对应模块版本固定。

任何从 `ExceptionView` 派生出的安全字符串视图、反射句柄或引用都不得越过该生命周期边界。

## 12. `[nothrow]`

`catch {}` 和 `catch as name` 可以为 `[nothrow]` 静态证明提供全覆盖处理器，但处理器本身仍必须保证没有异常逃逸：

```ink
[nothrow]
func boundary() {
    try {
        may_throw();
    } catch as error {
        log_nothrow(error.type_name());
    }
}
```

如果处理器执行 `throw;`、抛出新异常或者调用可能让异常逃逸的普通函数，外层函数不能通过 `[nothrow]` 检查。

## 13. 成本与 LLVM 降低

catch-all 不需要逐项检查异常类或接口。无绑定的 `catch {}` 不构造用户可见视图；`catch as name` 只建立一个短期记录视图，不分配、不复制载荷，也不增加引用计数。

`type_name()` 和 `matches[T]()` 读取已经存在的最小异常描述符。`reflection()` 仅在实际调用时查询记录固定的反射关联。没有异常发生时，这些操作不给正常路径增加执行成本。

LLVM 已有异常处理 IR 足以实现这些语义：catch-all 由 Ink personality 和目标 catch-all/handler 形式匹配，`throw;` 恢复同一个展开对象并继续展开，`ExceptionView` 由前端在处理器入口建立。无需修改 LLVM 源码。

## 14. 后续问题

以下内容仍留给后续议题：

- catch-all 在 freestanding 或禁用动态反射配置中的核心库裁剪；
- 跨动态库的 `ExceptionView` 和反射句柄精确 ABI。
