# 议题 60：异步覆盖不支持协变结果

> 状态：已确认，`Task::<T>` 对结果类型不变型
> 确认日期：2026-08-02

## 1. 异步覆盖结果必须完全一致

Ink v0 不允许异步虚函数、异步接口实现或异步接口默认覆盖使用协变逻辑结果类型：

```ink
class Animal {}
class Cat : Animal {}

class Factory {
    virtual async func create() -> Animal*;
}

class GoodFactory : Factory {
    override async func create() -> Animal*; // 合法
}

class BadFactory : Factory {
    override async func create() -> Cat*;    // 编译错误
}
```

覆盖检查比较源码声明的逻辑结果类型。除类型别名规范化等普通“同一类型”规则外，基声明和覆盖声明的结果必须完全相同；派生到基类的指针上转型、接口上转型或其他可转换关系都不构成异步结果兼容。

`void` 只能由 `void` 覆盖。同步函数显式返回 `Task::<T>` 仍不是异步覆盖，不参与本议题的结果比较。

## 2. `Task::<T>` 对 `T` 不变型

即使 `Derived*` 可以上转型为 `Base*`，任务类型之间也不存在对应的隐式转换：

```ink
var derived_task: Task::<Cat*> = create_cat();
var base_task: Task::<Animal*> = derived_task; // 编译错误
```

概念规则为：

```text
Task::<S> is compatible with Task::<T>
    only when S and T are the same type
```

原因包括：

- `Task::<T>` 是不可复制对象，不能通过复制建立另一任务；
- 任务身份包含状态、等待者、取消请求和模块固定，不能隐式包装成代理任务；
- 成功状态长期保存准确的 `T`，重复 `await` 从该只读存储复制同一类型；
- 派生指针到基类指针可能需要地址调整，不一定保持相同位模式；
- 结果描述符、调试信息、反射 ABI 和热更新布局必须与准确类型一致；
- 隐式转换任务可能引入第二个任务、堆分配、共享所有权或引用计数。

该不变型规则适用于普通赋值、按值传参、返回、容器元素、泛型实例和反射输出。原始 `Task::<T>*` 也只能指向准确的 `Task::<T>` 对象，不能因为结果类型可转换而重解释成另一任务指针。

## 3. 实现可以在返回前上转型结果

异步覆盖仍然可以创建或取得派生对象，只需把公开结果声明为基契约的准确类型：

```ink
class CatFactory : Factory {
    override async func create() -> Animal* {
        const cat: Cat* = await allocate_cat();
        return cat;
    }
}
```

`Cat*` 按普通类指针上转型规则调整为 `Animal*`，然后写入 `Task::<Animal*>` 的成功结果存储：

```text
Cat*
    → adjust to Animal*
    → store Animal* in Task::<Animal*>
    → publish succeeded
```

转换发生在异步函数完成之前，不转换任务本身。以后每次 `await` 都取得相同的 `Animal*` 值。

调用者需要恢复具体类型时，使用议题 31 的安全动态转换，并继续承担原始指针目标生命周期责任。

## 4. 不生成调用视图专用任务适配器

如果允许协变，同一覆盖至少需要区分：

```text
direct CatFactory call
    → construct Task::<Cat*>

call through Factory*
    → construct Task::<Animal*>
    → convert Cat* result to Animal* at completion
```

这会使调用视图改变任务公开类型、结果存储、完成写入代码、resume/completion 入口、反射结果描述和热更新 ABI。多接口还可能为同一实现要求多个不同基类指针调整。

Ink v0 不生成这种调用视图专用完成 thunk，也不为协变结果建立隐式适配任务。最终覆盖只有与基槽声明结果类型一致的任务构造 ABI。

## 5. 具体调用者使用额外方法取得精确类型

类希望同时提供多态契约和精确派生结果时，显式声明另一个方法：

```ink
class CatFactory : Factory {
    async func create_cat() -> Cat* {
        return await allocate_cat();
    }

    override async func create() -> Animal* {
        return await create_cat();
    }
}
```

调用者明确选择：

```ink
var exact: Task::<Cat*> = cat_factory.create_cat();
var polymorphic: Task::<Animal*> = factory->create();
```

`create()` 是否通过子任务复用 `create_cat()` 由源码决定。编译器不会仅因覆盖关系自动插入代理任务。实现也可以提取同步辅助函数或其他共享逻辑，以避免不必要的子任务。

## 6. 异步接口同样要求准确结果

```ink
interface Factory {
    async func create() -> Animal*;
}

class GoodFactory : Factory {
    override async func create() -> Animal*; // 合法
}

class BadFactory : Factory {
    override async func create() -> Cat*;    // 编译错误
}
```

接口默认方法覆盖、子接口重新声明和具体类实现都必须保留准确逻辑结果类型。接口表异步槽始终只构造接口契约声明的 `Task::<T>`，不根据具体实现类改变结果任务类型。

多个接口具有同名同参数但不同异步结果类型时，一个类方法不能借协变同时满足它们。若普通方法系统又无法通过明确重载或适配方法表达全部要求，则该类产生接口实现冲突；编译器不能按接口视图静默生成多个结果任务适配器。

## 7. 异步反射要求精确结果类型

议题 57 的异步 `FunctionInfo` 记录契约声明的准确逻辑结果类型：

```text
FunctionInfo.result_type = Animal*
```

调用者必须使用相同类型：

```ink
function.call_async[Animal*](receiver); // 合法
function.call_async[Cat*](receiver);    // 结构化反射结果类型错误
```

即使最终动态实现创建的是 `Cat`，反射描述符和 `DynamicTaskOut` 仍只承接 `Task::<Animal*>`。反射层不根据最终覆盖改变调用者期待类型，也不构造协变代理任务。

基类描述符调用派生覆盖、接口描述符调用类实现时都遵守相同规则，因此描述符逻辑结果、虚槽或接口槽和最终任务 ABI 保持一致。

## 8. 热更新

改变异步函数逻辑结果类型始终属于任务构造、分派和反射 ABI 变化：

```text
Animal* → Cat*
Cat*    → Animal*
```

继承关系不能使该变化变成原位兼容更新。旧任务保存准确的旧结果存储与描述符；新版本必须建立新的函数、虚槽或接口表版本，并遵守对象、描述符和调用方迁移规则。

热更新不能把已有 `Task::<Animal*>` 重新解释成 `Task::<Cat*>`，也不能只替换结果转换 thunk 而继续复用不匹配的旧帧 ABI。

## 9. 成本模型

结果类型完全一致使每个异步虚槽和接口槽只需要一种任务构造约定。不需要调用视图专用结果转换、额外任务、共享控制块、完成回调或反射适配层。

派生指针在函数体正常返回时执行的基类上转型属于普通返回值转换成本；它不为任务的创建、等待、取消或销毁增加持续开销。

## 10. 不决定同步协变返回

本议题只规定：

```text
async override logical result types must be identical
```

普通同步虚函数是否允许传统的指针或引用协变返回，留给完整同步覆盖兼容规则。同步函数没有 `Task::<T>` 身份、长期结果存储和任务转换问题，因此未来即使允许同步协变，也不会自动推广到异步覆盖。

同步函数返回 `Task::<Cat*>` 也不能覆盖异步 `func -> Animal*`，因为函数种类和逻辑结果契约都不同。

## 11. LLVM 与运行时实现

该规则不需要修改 LLVM 源码或增加运行时机制。前端在覆盖和接口实现检查时要求异步逻辑结果类型一致，随后让所有调用视图使用同一 `Task::<T>` 任务构造 ABI。

函数体中的 `Derived* → Base*` 转换降低为普通指针地址调整，并在发布成功状态前把最终 `Base*` 写入结果存储。LLVM 无需理解任务泛型方差。

## 12. 后续问题

以下内容留给其他模块：

- 同步虚函数的协变返回和完整覆盖兼容规则；
- 泛型类型参数的方差系统；
- 拥有型指针之间是否存在协变转换；
- 显式任务映射或库级结果转换组合器；
- 多接口同名不同结果要求的显式限定实现语法。
