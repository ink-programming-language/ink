# Ink 语言设计议题

Ink 采用逐项讨论、逐项落稿的设计方式。每个议题独立保存；只有全部议题完成并经过一致性检查后，才合并生成 `language-draft.md`。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 运行时检查、底层契约与 UB | 已确认，议题 04 修订 | [`topics/01-safety-model.md`](./topics/01-safety-model.md) |
| 02 | 值、复制与不可复制类型 | 已确认 | [`topics/02-values-copy-noncopyable.md`](./topics/02-values-copy-noncopyable.md) |
| 03 | 析构、RAII 与 `defer` | 已确认，议题 06 修订 | [`topics/03-raii-destructor-defer.md`](./topics/03-raii-destructor-defer.md) |
| 04 | 指针、引用、数组与切片 | 已确认 | [`topics/04-pointers-references-arrays-slices.md`](./topics/04-pointers-references-arrays-slices.md) |
| 05 | 基础类型与 `ptrsize` | 已确认 | [`topics/05-primitive-types-ptrsize.md`](./topics/05-primitive-types-ptrsize.md) |
| 06 | 构造函数、隐式构造与字面量初始化 | 已确认 | [`topics/06-constructors-implicit-initialization.md`](./topics/06-constructors-implicit-initialization.md) |
| 07 | 整数溢出与回绕算术 | 已确认 | [`topics/07-integer-overflow-wrapping.md`](./topics/07-integer-overflow-wrapping.md) |
| 08 | 平台相关行为（PDB） | 已确认 | [`topics/08-platform-dependent-behavior.md`](./topics/08-platform-dependent-behavior.md) |
| 09 | 整数除法与移位 | 已确认 | [`topics/09-integer-division-shifts.md`](./topics/09-integer-division-shifts.md) |
| 10 | 整数转换与 `cast` | 已确认，`as` 用途待定 | [`topics/10-integer-conversions-cast.md`](./topics/10-integer-conversions-cast.md) |
| 11 | 位模式转换与指针转换 | 已确认 | [`topics/11-bitcast-ptrcast.md`](./topics/11-bitcast-ptrcast.md) |
| 12 | 浮点语义与显式 fast-math | 已确认，细分规则待定 | [`topics/12-floating-point-fast-math.md`](./topics/12-floating-point-fast-math.md) |
| 13 | 整数与浮点转换 | 已确认 | [`topics/13-integer-floating-conversions.md`](./topics/13-integer-floating-conversions.md) |
| 14 | 浮点运行环境与 subnormal | 已确认 | [`topics/14-floating-environment-subnormal.md`](./topics/14-floating-environment-subnormal.md) |
| 15 | fast-math 与有限值契约 | 已确认，属性作用域待定 | [`topics/15-fast-math-finite-contract.md`](./topics/15-fast-math-finite-contract.md) |
| 16 | 内建属性与函数装饰器 | 已确认，议题 19 补充 | [`topics/16-attributes-function-decorators.md`](./topics/16-attributes-function-decorators.md) |
| 17 | 模块生命周期、装饰器注册与热更新 | 已确认，议题 18、22 补充 | [`topics/17-module-lifecycle-decorator-registration.md`](./topics/17-module-lifecycle-decorator-registration.md) |
| 18 | 模块生命周期钩子排序 | 已确认，依赖环规则待定 | [`topics/18-module-lifecycle-hook-order.md`](./topics/18-module-lifecycle-hook-order.md) |
| 19 | 编译期反射、动态反射与自定义元数据 | 已确认，议题 20、21、22 补充 | [`topics/19-reflection-runtime-metadata.md`](./topics/19-reflection-runtime-metadata.md) |
| 20 | 反射访问权限与封装 | 已确认 | [`topics/20-reflection-access-control.md`](./topics/20-reflection-access-control.md) |
| 21 | 动态反射值传递与调用 ABI | 已确认，议题 22 补充，精确二进制布局待定 | [`topics/21-dynamic-reflection-value-abi.md`](./topics/21-dynamic-reflection-value-abi.md) |
| 22 | 基于名称的动态反射身份 | 已确认 | [`topics/22-name-based-reflection-identity.md`](./topics/22-name-based-reflection-identity.md) |

“讨论中”的议题不会提前写入设计文件。后续议题在开始讨论时追加到本表，在确认后创建对应文件。
