# 议题 38：只读异常捕获与载荷不可变性

> 状态：已确认，议题 39—43 补充  
> 确认日期：2026-08-02

## 1. 所有捕获绑定永久只读

Ink 的全部异常捕获绑定都固定为只读短期引用：

```text
catch ConcreteError as error  -> error: const ConcreteError&
catch ErrorInterface as error -> error: const ErrorInterface&
catch as error                -> error: const core.ExceptionView&
```

这不是可以由语法切换的默认模式。语言不提供可变异常捕获，也不允许处理器改变运行时拥有的活动异常载荷。

```ink
try {
    parse();
} catch ParseError as error {
    error.position = 10; // 编译错误：捕获绑定只读
}
```

## 2. 不提供可变捕获语法

以下形式都不是 Ink 语法或不建立可变绑定：

```ink
catch mut ParseError as error    // 非法
catch ParseError& as error       // 非法
catch ParseError as var error    // 非法
catch mut as error               // 非法
```

用户不能通过属性、装饰器、泛型参数、异常接口或重载改变捕获绑定的只读性质。

## 3. 编译期禁止的操作

对于类型化捕获绑定，编译器至少拒绝：

- 给异常对象字段赋值；
- 把绑定转换或传递为普通可变 `T&`；
- 调用要求可变接收者的方法；
- 从只读异常接口引用调用要求可变接收者的接口方法；
- 取得安全可写切片或其他安全可变视图；
- 通过动态反射执行 `set`、`borrow_mut` 或要求可变接收者的调用。

`ExceptionView` 更不暴露载荷地址、字段视图或可写动态引用，因此 catch-all 不能据此修改异常对象。

## 4. `const` 是浅层访问限制

异常捕获遵守 Ink 普通 `const T&` 的限定范围：它禁止通过该捕获路径修改异常对象及其内联子对象，但不会递归冻结异常字段中原始指针所指向的独立外部对象。

```ink
[exception]
class OperationError {
    external_state: State*;
}

catch OperationError as error {
    error.external_state = other;       // 编译错误：修改异常字段
    error.external_state->mark_seen();  // 是否合法由 State* 自身契约决定
}
```

修改独立外部对象不会改变异常记录中的指针字段，但程序员仍负责该原始指针的生命周期、并发和别名契约。

## 5. 显式去除 `const` 不能建立合法可变捕获

议题 11 的 `ptrcast` 允许裸指针显式去除目标类型的 `const`，因此底层代码可能写出：

```ink
catch ParseError as error {
    let pointer: const ParseError* = &error;
    let writable = ptrcast<ParseError*>(pointer);
    writable->position = 10; // UB：写入运行时拥有的活动异常载荷
}
```

活动异常载荷在记录生命周期内是语义上不可写的对象。显式裸指针转换不改变这个事实；通过去除 `const` 的别名写入违反内存访问前置条件并产生 UB。

这与普通安全捕获代码不同：直接字段写入和可变借用在编译期报错。只有程序显式进入裸指针契约并破坏对象只读前置条件时，问题才落入底层 UB。

## 6. 反射不能绕过只读规则

异常类具有 `[reflect]` 时，类型化捕获可以建立只读 `DynamicRef`，但不能将它升级为可变视图：

```ink
catch ReflectedError as error {
    let dynamic = DynamicRef.from(&error);
    let value = property.get[int](dynamic);       // 合法：读取
    property.set[int](dynamic, 10);               // 编译或动态检查失败
    property.borrow_mut[int](dynamic);            // 编译或动态检查失败
}
```

如果调用点的只读性静态可见，编译器直接报错；否则反射适配器必须检查 `DynamicRef` 的可变性并以议题 34 规定的结构化反射异常报告失败，不能执行写入或产生 UB。

议题 37 的 `ExceptionView.reflection()` 只返回可选类型元数据，本身不提供当前异常载荷的 `DynamicRef`。

## 7. `throw;` 保持原始载荷

议题 37 的 `throw;` 复用同一个 `ExceptionRecord`。由于处理器不能修改载荷，外层处理器观察到的异常对象内联状态不会被内层处理器改写：

```ink
try {
    inner();
} catch IoError as error {
    log(error.message());
    throw;
}
```

处理器可以读取、记录、分类异常并产生外部副作用，但不能在重新抛出前给异常对象追加字段或改写消息。

该保证不冻结异常字段中的原始指针所指向的独立外部对象；外部状态仍可能按照自己的契约变化。

## 8. 改变错误语义需要新异常

如果处理器需要对外表达不同的错误内容，必须直接构造并抛出一个新的异常对象：

```ink
try {
    load_config();
} catch ParseError as error {
    throw ConfigurationError {
        position: error.position,
        message: "configuration parsing failed",
    };
}
```

这是一个新的异常记录和新的动态异常身份，不是对原异常的修改，也不是 `throw;`。原异常在离开当前处理过程时按照普通规则销毁。

语言不会自动把原异常保存为新异常的 `cause`。议题 40 只在源码显式写出 `from error` 时，把当前异常记录连接为新异常的原因；议题 41 规定新包装异常和原因异常各自保留自己的抛出位置与 traceback。

## 9. 生命周期仍然不能延长

只读不等于可以长期保存。类型化捕获引用、异常接口引用、`ExceptionView`，以及从它们派生的安全切片、反射句柄和引用都不能：

- 返回给调用者；
- 存入全局或长期对象字段；
- 被逃逸闭包捕获；
- 交给异步任务或其他线程；
- 在处理器结束后继续使用。

议题 43 的 `ExceptionBox` 可以在任务边界长期拥有异常，但普通处理器不能从捕获引用或 `ExceptionView` 构造、复制或取得 box。失败 `await` 建立的捕获绑定仍然只是当前活动展开记录的短期借用，不得保存当前记录的裸借用。

## 10. 成本与实现

普通类型化只读捕获不增加运行时字段、复制、分配、引用计数或动态检查。它主要由前端类型检查和反射已有的借用标志实现：

- 类型化捕获直接产生 `const T&` 或 `const Interface&`；
- catch-all 产生 `const core.ExceptionView&`；
- 反射动态引用保留只读标志；
- `throw;` 继续复用原记录。

LLVM 只接收普通只读访问和异常控制流，不需要新增 IR 指令或修改 LLVM 源码。是否利用异常载荷在语言层面不可写进行别名优化，由 Ink 前端在确保没有合法可写别名后决定。

## 11. 后续问题

本议题的只读捕获规则已经覆盖同步异常和议题 43 的异步任务异常。`ExceptionBox` 的精确公开查询 API 与二进制布局仍由后续核心运行时议题确定。
