# 议题 58：异步函数装饰器包围任务执行体

> 状态：已确认，异步 continuation 可调用零次或多次且 decorator 不生成模块生命周期钩子；议题 24、59 补充；Parser 第 36、37 号议题确认装饰器声明与全参数转发语法
> 确认日期：2026-08-02

## 1. 异步装饰器属于任务执行体

普通异步函数装饰器包围异步函数体，而不包围调用表达式中的同步任务构造过程：

```ink
@trace("load")
async func load() -> Data* {
    return await read_data();
}
```

调用 `load()` 时只求值并捕获实参、建立包含完整装饰器状态的 coroutine frame、固定模块版本并返回 `created` 任务。此时不执行 `trace` 生成的前置、后置、`defer` 或其他用户运行时代码。

第一次直接 `await`、组合等待或库级任务驱动开始执行任务后，才进入装饰器和原始函数体：

```text
load()
    → capture arguments
    → construct decorated coroutine frame
    → return created Task::<Data*>

await task
    → enter the decorated coroutine state machine
    → run decorator regions
    → run original async body
    → publish the succeeded result
```

因此从未被驱动的任务既不执行原始函数体，也不执行普通异步装饰器代码。装饰器不能把惰性异步调用重新解释为调用点立即执行的包装函数。

## 2. 同步和异步装饰器显式区分

现有同步装饰器继续使用 `decorator`：

```ink
decorator trace(name: string) {
    log("enter", name);
    const result = function(...);
    log("leave", name);
    return result;
}
```

只能应用于同步函数。异步版本使用显式 `async decorator`：

```ink
async decorator trace(name: string) {
    log("enter", name);
    defer log("leave", name);

    const result = await function(...);
    return result;
}
```

只能应用于 `async func`。类型检查器在装饰器应用处验证函数种类，不能根据装饰器名称、调用点是否出现 `await` 或期待结果类型自动转换同步与异步装饰器。

`async decorator` 描述生成到目标异步状态机中的 continuation region，不是一个运行时可独立调用并返回 `Task::<T>` 的普通异步函数。同步、异步通用的单一装饰器模板以及同名装饰器重载的完整规则留给后续装饰器类型系统议题。

## 3. `await function(...)` 是特殊异步 continuation

异步装饰器的特殊 `function` continuation 保留议题 16 的不可逃逸约束，但必须使用：

```ink
await function(...)
```

进入下一层异步装饰器或原始函数体。显式 `await` 标记控制流可能暂停；省略它是编译错误。

该表达式不调用一个普通异步函数，也不构造中间任务。编译器把它降低为同一个 coroutine frame 内的嵌套 continuation region：

```text
one Task::<T>
    one coroutine frame
        outer decorator state
        inner decorator state
        original body state
```

因此 `await function(...)` 不产生第二个 `Task::<T>`、`Task::<Task::<T>>`、第二次帧分配、隐藏任务移动或第二次面向对象分派。它的逻辑结果类型是目标异步函数声明的 `T`；`void` 目标只执行 `await function(...)` 而不取得值。

`function` 仍然不能保存到变量、字段或全局对象，不能返回、被闭包捕获或传给普通运行时函数。装饰器不能把 continuation 交给另一个任务或调度器。

## 4. 可以跳过或顺序重复进入

异步 decorator 可以在一条正常路径上零次、一次或多次执行 `await function(...)`，从而表达缓存命中、显式短路、普通包装或顺序重试。

每次进入都重新执行下一层 decorator 和最终异步函数体，但仍位于同一个最终 coroutine state machine 中。参数和局部值继续遵守普通复制、移动、借用和跨暂停生命周期检查；已经消费的不可复制参数不能被无条件再次转发。

特殊 continuation 仍不能保存、逃逸、被闭包或其他任务捕获，也不能作为普通 `Task` 交给并发组合器。因此本规则允许当前异步 decorator 顺序控制进入次数，不允许把同一个 continuation 分发给其他任务并发进入。

## 5. 多层装饰器共享一个状态机

```ink
@outer
@inner
async func target() -> Result {
    // original body
}
```

任务执行顺序为：

```text
outer.before
    inner.before
        original body
        possibly suspend and resume many times
    inner.after
outer.after
```

最靠近函数声明的异步装饰器最先包裹原始函数体。每层 continuation 指向下一层，不会通过函数稳定入口递归调用当前装饰器。

装饰器局部变量如果跨越 `await function(...)` 或其他暂停点仍然存活，编译器把它们纳入同一个具体 coroutine frame。不同装饰器组合可以产生不同帧布局；调用者仍只依赖统一的 `Task::<T>` 任务构造 ABI。

原始函数体中的 `return` 结束其 continuation region并把逻辑结果交给内层装饰器，而不是越过全部后置代码直接发布任务成功状态。每层装饰器可以在保持公开结果类型不变时检查或修改正常结果。

## 6. 异步装饰器可以等待其他任务

异步装饰器生成的代码属于目标异步状态机，因此除特殊 continuation 外还可以使用普通 `await`：

```ink
async decorator authorize() {
    const allowed = await check_permission();
    if (!allowed) {
        return LoadResult.permission_denied();
    }

    return await function(...);
}
```

`check_permission()` 创建并等待普通子任务；`await function(...)` 进入同一装饰后状态机的下一 continuation。二者具有不同 ABI，编译器不能把特殊 continuation 当作可保存的普通 `Task`。

该装饰器只能应用于逻辑结果 `T` 能够表示权限拒绝状态的目标；类型检查器不为不兼容的结果类型添加隐藏转换。这允许装饰器实现异步权限检查、追踪、指标、限流等待和调用前后资源管理。取消请求仍遵守议题 48—50：装饰器不会自动把请求转换成某个业务结果，也不会自动把请求传播给它等待的子任务，除非生成代码显式采用相应传播形式。

## 7. 显式结果经过完整装饰器链

最终实现的完整装饰器链共同产生目标函数声明的逻辑结果 `T`：

```text
outer async decorator
    → inner async decorator
        → original async body
    → publish T through ordinary return
```

`await function(...)` 取得下一层的普通 `T`。如果 `T` 自身包含业务状态，装饰器可以显式检查并转换该状态，但不会获得隐藏的第二条完成通道：

```ink
async decorator translate_status() {
    const result: LoadResult = await function(...);
    if (result.status() == LoadStatus::NetworkUnavailable) {
        return LoadResult.service_unavailable();
    }

    return result;
}
```

装饰器自己的异步操作同样通过普通返回值表达业务结果。任何一层忽略状态或选择映射状态都属于该层可见的普通控制流；运行时不会自动越过外层装饰器传播某个枚举项。

实参求值、参数捕获、coroutine frame 取得和任务基础状态建立发生在装饰器执行前。需要同步检查可恢复条件时，程序使用显式同步工厂返回普通状态，再决定是否创建任务；普通异步装饰器不能观察这个创建决定。

普通后置语句可以检查下一层返回的 `T`。`defer` 和 RAII 继续按照装饰器 region 与原始函数体的正常嵌套作用域执行清理，不引入另一条结果通道。

## 8. 完整装饰后状态机保持结果类型

异步装饰器不能削弱或改变目标函数的公开签名。编译器必须对完整展开后的状态机检查同一个逻辑结果类型：

```text
outer decorator generated code
    inner decorator generated code
        original async body
    → exactly T
```

装饰器中的前置代码、普通 `await`、特殊 continuation、后置代码、析构和 `defer` 都必须保持显式结果约定。等待的子任务返回项目定义的状态时，生成代码必须像普通用户代码一样检查或转发该值，不能增加隐式失败语义。

装饰器定义本身无法完成编译期展开、返回值类型不兼容或遗漏必需结果时，均按普通编译错误处理。

## 9. 虚函数只执行最终覆盖的装饰器

异步虚分派先在任务创建时选择最终覆盖，再构造该覆盖版本完整的装饰后任务：

```ink
class Base {
    @base_trace
    virtual async func load() -> Data* {
        return await base_load();
    }
}

class Derived : Base {
    @derived_trace
    override async func load() -> Data* {
        return await derived_load();
    }
}
```

通过 `Base*` 调用 `Derived` 对象时：

```text
task creation
    → vtable selects Derived.load construction thunk
    → construct Derived decorated coroutine frame

task execution
    → derived_trace
    → Derived.load body
```

`base_trace` 不沿覆盖关系自动继承。只有派生实现显式调用并等待某个基类实现时，才另外建立该基类实现自己的装饰后任务。这与议题 25 的同步虚函数装饰器规则一致。

虚槽保存最终覆盖的任务构造 thunk，不保存外层装饰器首次恢复地址。第一次 `await` 不重新选择覆盖或装饰器链。

## 10. 接口要求没有可装饰函数体

没有函数体的接口必需方法和抽象类方法不能应用普通同步或异步函数体装饰器：

```ink
interface Loader {
    @trace
    async func load() -> Data*; // 编译错误：没有函数体可包围
}

class AbstractLoader {
    @trace
    virtual async func load() -> Data*; // 编译错误：抽象函数没有函数体可包围
}
```

允许这种写法会产生“契约级拦截全部实现”的新语义，而现有装饰器只包围当前声明自己的函数体。若未来需要接口级拦截器，应使用不同的显式机制。

具有函数体的接口异步默认方法可以应用 `async decorator`：

```ink
interface Loader {
    async func load() -> Data*;

    @trace_default
    async func reload() -> Data* {
        return await load();
    }
}
```

接口槽选择默认实现时，任务包含该默认方法自己的装饰器链和议题 56 的规范化胖接收者。具体类覆盖默认方法后，只运行类覆盖自己的装饰器，不自动运行默认方法装饰器。

## 11. 反射只构造最终装饰后任务

议题 57 的 `FunctionInfo.call_async::<R>` 在调用点完成反射检查和普通虚或接口分派；状态为 `Ready` 时，在 `AsyncReflectionCall::<R>` 中直接构造最终实现的装饰后任务：

```text
call_async
    → reflection validation
    → return a rejection status without a task, or continue
    → static, virtual, or interface dispatch
    → construct selected decorated Task::<R>
    → return Ready with the created task

await task
    → selected implementation decorators
    → selected implementation body
```

反射适配器本身不进入异步装饰器，也不增加描述符来源处的装饰器层。描述符来自基类或接口时，仍然只执行最终选中实现自身的装饰器链。

创建期反射校验在任务建立前返回显式状态，不能由目标函数装饰器处理。任务创建成功后，反射参数数组和描述符借用不进入装饰器帧。

## 12. 热更新固定完整装饰后版本

异步任务创建时固定最终选择版本的：

- 完整装饰器展开结果；
- 原始函数体；
- coroutine frame 布局；
- resume、destroy 和完成状态发布入口；
- 类型元数据、调试信息和必要接口表。

```text
construct V1 decorated task
    → hot update publishes V2 decorators and body
    → await old task
    → execute V1 decorators and V1 body
```

新任务使用 V2。旧任务不能执行 V1 前置装饰代码后切换到 V2 函数体或后置装饰代码。任务完全销毁前，运行时必须保持旧装饰器生成代码、帧析构入口和所需模块状态有效。

Decorator 不生成模块加载或卸载代码。它在编译期产生的强类型注册记录属于对应 module 版本，与某个任务是否被驱动无关。注册记录引用异步目标时，`function.entry` 表示最终装饰后惰性任务的稳定构造入口；调用该入口不直接进入 coroutine body 或首次 resume。

## 13. 成本模型

异步装饰器不增加第二个任务对象或强制额外帧分配。它会使最终 coroutine frame 包含跨暂停点存活的装饰器局部状态，并增加装饰器生成代码本身的执行、子任务等待以及显式结果检查和转换成本。

从未驱动的任务不支付装饰器运行时代码成本，但其具体帧布局可能已经为装饰器跨暂停状态预留存储。编译器可以合并不重叠的帧槽、内联 continuation region 和删除无可观察作用的生成代码，但不能提前执行装饰器副作用、改变源码确定的 continuation 进入次数或绕过显式结果检查。

普通未装饰异步函数、同步函数和其他任务不因为语言支持 `async decorator` 增加运行时字段或分派成本。

## 14. LLVM 与运行时实现

该设计不需要修改 LLVM 源码。前端在 HIR/MIR 中把异步装饰器降低为嵌套 continuation region，并把 `await function(...)` 转换为同一协程状态机内部的 region 进入和结果传递。

coroutine lowering 统一计算原始函数体和全部装饰器跨暂停状态所需帧布局。最终状态机通过普通分支和结果存储发布 `T`；装饰器对业务状态的检查与转换不需要额外的运行时控制流协议。

LLVM 无需把特殊 `function` continuation 表示成运行时函数指针或 `Task::<T>`。展开来源映射必须同时保留被装饰函数、装饰器应用和装饰器定义位置，以支持诊断和调试器。

## 15. 后续问题

以下内容留给独立议题：

- 议题 59 已确认 v0 不提供任务构造装饰器，创建期行为使用显式同步任务工厂；
- 同一个装饰器定义跨同步和异步目标复用的类型系统；
- 异步装饰器静态状态的热更新迁移；
- 生成器和异步生成器的 continuation 装饰器；
- 装饰器参数、异构参数包和返回类型约束的完整规则。
