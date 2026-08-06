# Module 议题 03：实际 Module 依赖图无环

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 基本规则

Ink v0 要求一次具体构建中激活的 module 依赖图是有向无环图（DAG）。任何直接或间接实际依赖环都非法。

```text
A → B → A       illegal
A → B → C → A   illegal
A → A           illegal
```

本规则适用于同一根 package 内、不同根 package 之间以及跨 package 混合形成的 module 环。

## 2. 依赖边

每个激活的 module import 都从导入者建立一条指向被导入 module 的依赖边：

```ink
import game.graphics.window;
from game.math.vector import Vector;
```

两种导入形式在依赖图上没有区别。导入多个成员、使用别名或重复导入同一个 module 不产生额外语义边；依赖图中相同的起点和终点只保留一条边，各源码导入位置仍保留独立 CST 和名称绑定。

Package 和 subpackage 的目录包含关系不自动建立依赖边。只有实际激活的 module import 建立依赖。

## 3. 条件导入

Parser 议题 05 先收集候选导入，再通过顶层 `comptime` 选择实际导入。无环检查只作用于当前构建激活后的 module 边。

```ink
comptime if (target.os == Os.windows) {
    import platform.windows.runtime;
} else {
    import platform.linux.runtime;
}
```

未选中分支中的候选边不参与当前构建的环检测。不同目标可以产生不同的实际 DAG，但每个具体目标的实际图都必须无环。

控制条件导入的编译期求值本身也不能形成导入选择依赖环。导入条件只能使用已经可用且不依赖本次条件选择的编译期输入和声明。

## 4. 检查时机

一次构建的相关顺序为：

```text
parse source files needed for import selection
→ collect candidate import sites
→ evaluate import guards
→ normalize relative paths
→ form active module dependency graph
→ reject self edges and strongly connected components with more than one module
→ topologically process active modules
```

由于编译期声明区域控制不能创建新的 import，实际依赖图通过该检查后不会在后续 Staged InkIR 声明固定点中增加未知边。

## 5. 生命周期顺序

实际依赖图无环后，议题 18 的 module 生命周期顺序具有确定的拓扑约束：

```text
load:   dependency before importer
unload: importer before dependency
```

加载失败回滚按照实际成功初始化顺序逆序执行。不存在必须让两个 module 彼此先完成初始化的循环要求。

没有依赖路径的 module 仍不保证相对顺序，实现可以并行处理；DAG 只约束边连接的 module。

## 6. 编译期执行与名称绑定

Ink v0 不把一个循环强连通分量合并成隐式超级 module，也不对环中全部 module 执行两阶段声明收集后再尝试互相绑定。

因此 module `A` 通过导入使用 `B` 的类型、函数或编译期声明时，`B` 必须沿无环依赖边先成为可处理依赖。类型级引用、只调用函数而不使用全局状态等情况也不会自动豁免依赖环规则。

未来如果增加明确的 `type-only import` 或独立接口描述机制，需要另行确定它是否建立普通 module 边；本议题不会自动允许现有 import 形成类型环。

## 7. 组织循环关系

需要互相引用的代码应重构为无环结构。常见方式是把共享声明放入第三个基础 module：

```text
before:
    renderer ↔ window

after:
    renderer → graphics.common
    window   → graphics.common
```

或者使用依赖倒置：

```text
backend.interface
↑               ↑
opengl          vulkan
```

如果两个实现天然无法分离，也可以放入同一个 `.ink` module；Module 议题 01 仍规定该 module 只对应一个物理文件。

## 8. 热更新

热更新沿实际 DAG 准备受影响的新 module 版本，按依赖顺序初始化并按逆序撤销旧版本。禁止环避免把整个强连通分量定义为不可拆分更新事务，也避免发布过程中观察到相互只完成一部分初始化的 module。

热更新可以根据反向依赖关系扩大重新构建或重新加载集合，但不能临时忽略依赖边来打破源码中的环。

## 9. 确认结论

Ink v0 的每个具体构建只允许无环的实际 module 依赖图。所有激活的 `import` 和 `from ... import ...` 都建立普通 module 边；未激活候选边不参与检查。无环约束统一支撑名称绑定、编译期执行、生命周期、失败回滚和热更新，不提供隐式强连通分量合并。
