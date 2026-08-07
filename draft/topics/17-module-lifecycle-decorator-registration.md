# 议题 17：模块版本、强类型注册记录与热更新

> 状态：已确认，移除用户可见的模块加载/卸载钩子；议题 18、22、34、36、42—45、55、58、61 与 Module 议题 03 补充
> 确认日期：2026-08-01

## 1. 不提供用户生命周期钩子

Ink 不提供用户可声明或由 decorator 生成的模块加载、卸载 block，也不为它们设置关键字、attribute 或专用 Parser 产生式。

模块映射、全局对象初始化、版本发布、旧版本回收和代码卸载仍由编译器与运行时内部完成，但这些内部阶段不会回调任意用户生命周期函数。

需要普通资源生命周期时，使用 module 全局对象的构造、析构与 RAII；需要路由、RPC、测试发现或反射注册时，使用编译期强类型注册记录。

## 2. 强类型模块注册记录

Decorator 可以在编译期产生结构化、强类型的模块注册记录：

```ink
decorator route(path: string) {
    comptime register_module_item(
        RouteInfo {
            path: path,
            entry: function.entry
        }
    );

    return function(...);
}
```

`register_module_item` 表示编译器提供的编译期注册操作；它接收普通强类型值，不接收源码字符串、AST builder 或任意运行时回调 block。具体注册记录类型及其运行时表由对应工具链或库协议定义。

每条注册记录保存：

- 记录的静态类型和值；
- 产生它的 module 版本；
- 对应源码声明和 decorator application 的词法锚点；
- 记录引用的稳定函数入口或其他静态声明身份。

注册值必须能够在编译期完成构造并编码进模块镜像。不能把运行时对象、当前线程状态或需要任意用户清理代码的资源放入注册记录。

## 3. 自动安装与撤销

模块注册记录属于模块版本，而不属于某次函数调用或对象实例。运行时在发布模块版本时安装记录，在撤销旧版本时按不可伪造的版本所有权移除记录：

```text
prepare module version
    → initialize module globals
    → prepare typed registration records
    → atomically publish entries and records

retire module version
    → stop new entries
    → wait for version users
    → atomically remove owned records
    → destroy module globals
    → release code and data
```

撤销旧版本记录不能删除同一逻辑键的新版本记录。新版本准备失败时，其尚未发布的记录直接丢弃，不运行用户回滚钩子。

## 4. 普通程序启动与退出

普通程序启动顺序为：

```text
load and relocate program
    → initialize Ink runtime
    → initialize module globals in dependency order
    → install module registration records
    → publish module entries
    → call main
```

正常退出时先停止产生新模块调用并移除相应注册记录，再按依赖逆序析构 module 全局对象，最后结束运行时。程序需要可靠完成业务提交或持久化时，仍必须在正常控制流中显式完成，不能依赖进程退出阶段。

## 5. 动态加载与卸载

动态模块在完成目标、ABI 和依赖检查后初始化全局对象并准备注册记录，最后一次性公开模块入口和记录。其他线程不能观察到只公开了一部分的版本。

卸载前必须撤销公开状态、禁止新调用并等待当前版本的活动使用者离开。只要函数指针、回调、异常记录、任务帧或其他入口仍可能进入该版本，运行时就不能释放其代码、数据或描述符。

## 6. 热更新事务

热更新先完整准备新版本，再原子切换：

```text
prepare V2 globals and typed records
    → validate V2
    → atomically publish V2 entries and records
    → stop new calls into V1
    → wait for V1 users
    → remove V1 records
    → destroy and release V1
```

函数稳定入口、动态反射描述符和模块注册记录属于同一次提交。不能出现函数实现已经切换而注册表仍指向旧版本的中间可见状态。

新版本准备失败时继续使用旧版本。已经成功构造的新版本全局对象按普通 RAII 逆序销毁；未发布注册记录直接丢弃。

## 7. Decorator 与稳定入口

Decorator 只静态包装目标函数调用，并可在编译期产生强类型注册记录。它不能插入独立于函数调用而执行的模块生命周期代码。

注册记录中的 `function.entry` 表示最终装饰后函数的稳定公开入口，不是某个版本原始函数体的裸地址。对于异步函数，该入口是最终装饰后任务的构造入口；调用它只构造惰性任务，不立即执行异步 decorator 或函数体。

## 8. 资源生命周期

需要设备、线程、连接或其他运行时资源的 module 使用普通全局 RAII 对象或显式库级管理对象。其构造与析构遵守普通异常、所有权和依赖顺序，不建立第二套生命周期 block。

需要超出全局对象析构能力的进程级服务时，应由 `main` 或显式应用对象管理启动与停止。异常终止、进程崩溃或操作系统强制结束不保证运行全局析构，因此关键持久化仍不能只依赖清理阶段。

## 9. 并发与版本固定

模块发布、热更新提交和注册记录切换形成同一个发布同步点。看到新函数实现的线程也必须看到新版本的注册记录和已经完成初始化的全局状态。

活动异常记录、失败任务、尚未驱动的惰性任务、异步虚调用选择的构造入口以及装饰后 coroutine frame 都会固定对应模块版本。旧版本只能在这些使用者全部释放后回收。

本规范只规定可观察语义，不要求运行时使用活动计数、epoch、hazard pointer 或某一种特定静止状态算法。

## 10. 成本模型

静态注册记录不为每次普通函数调用增加检查。运行时成本只发生在模块版本发布、记录表切换、动态查找或热更新入口间接层本来需要的位置。

未使用动态加载、热更新或运行时注册的程序可以由编译器和链接器静态合并注册表，不要求通用动态模块管理器常驻。

议题 61 规定 Ink module 以源码分发，不承诺稳定的 Ink-to-Ink 二进制模块 ABI。独立二进制库仍必须通过 `extern "C"` 和 C ABI 安全类型建立公开边界。

## 11. 结论

Ink 不向用户暴露模块加载或卸载钩子。模块级可发现信息由编译期强类型注册记录表达，运行时按模块版本自动安装和撤销；普通运行时资源由全局 RAII 对象或显式应用代码管理。
