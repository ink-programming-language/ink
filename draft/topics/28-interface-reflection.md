# 议题 28：接口动态反射

> 状态：已确认，议题 29—31、34、56、57 补充  
> 确认日期：2026-08-02

## 1. 接口可以显式生成动态反射信息

接口和接口方法分别使用 `[reflect(...)]` 选择动态反射：

```ink
[reflect(DisplayName("Renderable"))]
interface Renderable {
    [reflect(Category("Rendering"))]
    func render(canvas: Canvas&);

    func is_visible() -> bool;
}
```

- 接口上的 `[reflect]` 生成运行时 `InterfaceInfo`；
- 接口方法上的 `[reflect]` 生成接口方法 `FunctionInfo` 和动态调用适配器；
- 没有 `[reflect]` 的接口方法仍参与普通接口表分派，但不能按名称动态查找；
- 接口方法使用 `[reflect]` 时，所属接口也必须使用 `[reflect]`，否则编译错误；
- 同一接口不能具有两个同名反射函数。

编译期反射不需要这些标记，仍可以查看接口的全部方法和元数据。

## 2. `InterfaceInfo`

运行时按照议题 22 的完全限定名称查找接口：

```ink
if (match .some(interface) = reflection.find_interface("game.Renderable")) {
    print(interface.name);

    if (match .some(display_name) =
        interface.metadata.get[DisplayName]()) {
        print(display_name);
    }

    for (const function in interface.functions) {
        print(function.name);
    }
}
```

`InterfaceInfo` 至少公开：

- 完全限定名称；
- 用户元数据；
- 直接父接口和传递祖先接口；
- 当前接口直接声明的方法以及继承后的有效方法视图；
- 被反射的方法集合；
- 当前加载的被反射实现关系；
- 接口引用 ABI 信息。

接口不是可以独立实例化的类值，因此没有普通对象 `size`。它可以报告接口引用由两个机器字组成，但接口表本身不是对象字段。

## 3. 接口反射调用

接口方法描述符通过接口胖引用调用具体实现：

```ink
class Player : Renderable {
    override func render(canvas: Canvas&) {
        // ...
    }
}

const player = Player();
const renderable: Renderable& = player;

if (match .some(interface) = reflection.find_interface("game.Renderable")) {
    if (match .some(render) = interface.function("render")) {
        render.call[void](renderable, &canvas);
    }
}
```

调用顺序为：

```text
Renderable.render FunctionInfo
    → 验证参数和接收接口
    → 取得 Renderable 接口表
    → 读取 render 槽
    → 调用 Player.render
```

接口反射适配器不能直接绑定某个具体实现函数，因为同一个 `Renderable.render` 描述符必须适用于所有当前和未来实现类。

议题 57 允许接口方法声明为 `[reflect] async func`。其 `FunctionInfo` 记录异步种类和逻辑结果类型；调用者使用同步任务构造 API `call_async[R]`。适配器验证或构造接口视图后，在调用表达式中读取议题 56 的异步接口槽并直接建立最终惰性任务，不能把接口分派推迟到第一次 `await`。

## 4. 实现方法不自动成为类反射函数

接口方法已经 `[reflect]` 时，实现方法不需要添加 `[reflect]`，也能通过接口描述符动态调用：

```ink
[reflect]
interface Renderable {
    [reflect]
    func render(canvas: Canvas&);
}

class Player : Renderable {
    override func render(canvas: Canvas&) {
        // 不生成 Player.render 的类 FunctionInfo
    }
}
```

被反射的是 `game.Renderable.render` 契约。`Player-as-Renderable` 接口表已经提供到具体实现的分派路径。

如果还需要按类名称查找实现方法，则类和实现方法分别显式添加 `[reflect]`：

```ink
[reflect]
class Player : Renderable {
    [reflect(Category("Player"))]
    override func render(canvas: Canvas&) {
        // ...
    }
}
```

这时存在两个独立描述符：

```text
game.Renderable.render
game.Player.render
```

它们可以最终调用同一实现，但名称、声明来源和元数据彼此独立，不自动合并。

## 5. 静态具体对象便捷调用

调用者静态知道具体类时，强类型反射便捷层可以自动建立临时接口视图：

```ink
render.call[void](&player, &canvas);
```

编译器验证 `Player` 实现 `Renderable`，并把该调用降低为：

```ink
const temporary: Renderable& = player;
render.call[void](temporary, &canvas);
```

这种调用不要求 `Player` 自身带有 `[reflect]`，因为具体类型和接口转换在编译期已知。

## 6. 完全动态对象

如果调用者只有一个具体类型未知的 `DynamicRef`，运行时必须：

1. 从动态类描述符取得其实现接口列表；
2. 按完全限定接口名称查找 `Player-as-Renderable` 等实现关系；
3. 构造临时接口胖引用；
4. 通过接口槽执行调用。

这种路径要求具体类具有 `[reflect]`，并且其与目标反射接口的实现关系已经注册。否则运行时不能从无类型地址猜测接口实现，必须抛出“接口未实现或未反射”异常。

## 7. 类与接口实现关系

一个 `[reflect]` 类可以枚举它实现的 `[reflect]` 接口：

```ink
if (match .some(player) = reflection.find_type("game.Player")) {
    for (const interface in player.interfaces) {
        print(interface.name);
    }
}

if (player.implements("game.Renderable")) {
    // ...
}
```

一个反射接口也可以枚举当前已加载的反射实现类：

```ink
if (match .some(renderable) = reflection.find_interface("game.Renderable")) {
    for (const implementation in renderable.implementations) {
        print(implementation.name);
    }
}
```

实现关系只有在接口和具体类都具有 `[reflect]` 时才进入动态注册表。普通未反射类仍可实现和调用接口，但不会因为接口被反射而被迫生成完整类描述符。

反向实现列表只表示当前已经成功加载并公开的模块版本，不是所有可能链接或未来加载类型的封闭集合。

## 8. 多接口同名方法

不同接口中的同名反射方法具有不同的完全限定名称和独立描述符：

```ink
[reflect]
interface A {
    [reflect]
    func reset();
}

[reflect]
interface B {
    [reflect]
    func reset();
}
```

```text
game.A.reset
game.B.reset
```

一个类方法可以在普通接口规则允许时同时实现二者。两个接口表槽可以指向同一个实现或适配 thunk，但反射元数据仍分别来自 `A.reset` 和 `B.reset`。

接口默认实现冲突、签名不兼容和类实现规则由议题 29 确定；接口继承后的同源菱形合并、子接口优先查找和声明来源记录由议题 30 确定。反射层不通过声明顺序任意选择。

## 9. 名称与元数据

接口、接口方法和实现类均遵守议题 22：

- 只按完全限定字符串名称建立动态身份；
- 不提供稳定声明 ID；
- 重命名等于删除旧项并创建新项；
- 实现可以缓存哈希和进程内句柄，但碰撞时必须比较完整名称。

接口方法元数据、类实现方法元数据和类自身元数据各自属于对应声明，不进行隐式继承或合并。

## 10. 热更新

以下状态必须在同一个模块更新事务中发布：

- `InterfaceInfo`；
- 接口方法 `FunctionInfo`；
- 具体类—接口实现关系；
- 对应接口表和调用适配器；
- 反射名称注册项。

只改变具体实现代码而不改变接口槽签名时，可以原子替换接口槽入口。增加、删除、重新排序接口方法或改变方法签名属于接口 ABI 变化，必须建立新接口表版本，并迁移或固定旧接口引用。

把接口方法从同步改成异步或从异步改成同步同样属于接口和反射 ABI 变化。异步反射任务创建成功后固定最终实现；默认方法任务还固定规范化接口表布局。新更新只影响以后的 `call_async`，不能重新解释旧任务帧。

卸载模块时，旧实现关系只有在相关接口引用、反射描述符借用和活动调用全部排空后才能移除。

## 11. 成本模型

普通接口调用不经过反射名称查找和动态参数检查。

接口反射调用额外承担：

- 一次名称或描述符查找；
- 参数和接收接口检查；
- 一次反射适配器间接调用；
- 一次正常接口表槽分派。

它不要求复制实现对象、建立拥有型 `Any` 或进行堆分配。接口被 `[reflect]` 不会自动使所有实现类进入反射注册表。

异步接口反射调用还承担 `DynamicTaskOut` 检查和原本就需要的任务构造成本，但不要求第二个任务对象或 `Task<Task<R>>`。任务恢复不再经过反射适配器。

## 12. 后续问题

以下内容留给后续议题：

- 接口属性要求及其反射；
- 接口方法的可见性规则；
- 拥有型接口容器；
- 完全动态对象的安全下转型与接口转换由议题 31 规定；拥有型动态对象 API 仍待定。
