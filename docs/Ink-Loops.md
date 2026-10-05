# 循环

Ink 支持 `while`、三段式 `for` 和定长数组 `for … in`。普通函数与 `comptime func` 使用同一套循环 IR，可解释执行、编译为字节码，或通过 LLVM 生成本机代码。

```ink
var Sum = 0;
for (var I = 0; I < 10; I++)
{
    if (I == 2) continue;
    Sum += I;
    if (Sum > 20) break;
}
while (Sum < 42) ++Sum;
```

`while` 在每轮开始前求值条件，条件必须为 `bool`。三段式 `for` 按初始化、条件、循环体、步进的顺序执行；初始化仅执行一次，三个部分均可省略。省略条件等同 `true`。初始化和步进中的逗号分隔条目从左向右执行。整数 `++`、`--` 和 `+=` 支持运行时执行；前缀更新返回新值，后缀更新返回旧值，算术按整数位宽回绕。其他运算的可用范围仍由对应表达式实现决定。

`break` 退出最近一层循环；`continue` 跳过当前轮剩余语句，三段式 `for` 先执行步进，`while` 重新判断条件，`for … in` 前进到下一元素。`return` 直接退出函数。循环体可以是块或单条语句。每轮有独立的局部作用域，三段式 `for` 的初始化声明只在该循环内可见，循环体中的声明不对步进部分可见。

```ink
var Sum = 0;
for ([First, Tail...] in [[1, 2, 3], [4, 5, 6]])
{
    Sum += First;
    for (Item in Tail) Sum += Item;
}
```

`for … in` 的迭代对象目前是定长数组，支持空数组、多维数组、函数返回数组和类元素数组。迭代表达式只求值一次，遍历其值快照；循环体修改原数组不会改变后续遍历值。绑定是本轮独立的可变值副本，修改绑定不会写回原数组。`_` 忽略元素；数组模式支持递归解构、尾部 `Name...` 和 `_...`，命名尾部仍为定长数组。模式的长度和元素类型必须匹配，同一模式不得重复绑定名字。元组类型、字符串、切片和自定义迭代器协议尚未接入，不能用作替代的迭代对象。

正常结束、`continue`、`break` 和 `return` 都按逆构造顺序析构离开的作用域内已初始化对象。`continue` 保留初始化部分和迭代快照，`break` 销毁它们。迭代条件和步进产生的临时对象在各自完整表达式结束时析构。

循环可能执行零次，因此仅在循环体内赋值的变量不能据此视为循环后的确定初始化变量。无条件循环退出时合并各个 `break` 路径的初始化状态；步进部分合并正常到达和 `continue` 路径。无退出路径的无限循环不要求补写不可达的返回语句，解释器和编译期执行仍受步骤预算限制。

`comptime while` 和 `comptime for` 在分析阶段执行循环头并静态展开循环体；`comptime for … in` 同样按数组常量展开。完整 `comptime` 块中的普通循环在分析阶段执行。静态展开可包含运行时循环，运行时循环也可包含静态展开；`break`/`continue` 必须控制同一执行阶段中最近的循环。运行时分支不能条件性退出外层静态循环，`comptime` 块也不能直接控制外层运行时循环，违规报告 `INK-S0059`。

测试入口：`SemanticLoopTest.*` 覆盖执行、作用域、初始化、编译期循环和错误诊断；`AOTClassTest.Loops`、`AOTClassTest.LoopLifecycle` 对同一源码核对解释器、字节码和 LLVM O0/O2 的退出码与输出。

`src/testcase/execution/programs/loops_while.ink`、`loops_for.ink`、`loops_for_in.ink`、`loops_effects.ink`、`loops_lifecycle.ink` 和 `loops_comptime.ink` 均可通过 `inkc --interpret -i FILE` 独立运行。六个程序共包含 39 项具名检查，每项先输出用例名，结果正确时输出 `PASS`，失败时输出 `FAIL` 并返回 1。`ExecutionSourceTest.Loops.Main.*` 同时验证源码解释和保存后的字节码执行，核对完整 stdout、空 stderr 和退出码 0；执行顺序和析构顺序也包含在 stdout 断言中。

```powershell
ctest --test-dir build -C Release -R '^ExecutionSourceTest[.]Loops[.]' --output-on-failure
```
