# TODO

1. IR Module 序列化为临时实现，后续需要迭代为正式版。
2. VM Linker 和已有 validator 都为临时实现，后续需要迭代为正式版。
3. 为 VM 的 ZERO 寄存器实现跨平台的硬件只读存储；当前静态 `constexpr` 对象仅在当前 Windows 构建中验证了页面只读，尚无跨平台页面保护保证。
4. VM 尚未实现函数入口的栈峰值检查（含局部、临时及调用区）和动态栈分配检查，Add64 暂不检查 SP 容量。
5. 按 [VM 指令分派性能优化方案](VM-Dispatch-Optimization.md) 迭代优化解释器 `Interpreter::dispatch()` 的寄存器访问、分派机制与预解码执行路径。
