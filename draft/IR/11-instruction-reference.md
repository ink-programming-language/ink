# 指令

- [`const.int`](#instruction-const-int)
- [`const.bool`](#instruction-const-bool)
- [`const.float`](#instruction-const-float)
- [`const.null`](#instruction-const-null)
- [`const.unit`](#instruction-const-unit)
- [`const.symbol_addr`](#instruction-const-symbol-addr)
- [`const.function`](#instruction-const-function)
- [`cf.br`](#instruction-cf-br)
- [`cf.cond_br`](#instruction-cf-cond-br)
- [`cf.switch`](#instruction-cf-switch)
- [`cf.select`](#instruction-cf-select)
- [`cf.return`](#instruction-cf-return)
- [`cf.unreachable`](#instruction-cf-unreachable)
- [`arith.add`](#instruction-arith-add)
- [`arith.sub`](#instruction-arith-sub)
- [`arith.mul`](#instruction-arith-mul)
- [`arith.neg`](#instruction-arith-neg)
- [`arith.and`](#instruction-arith-and)
- [`arith.or`](#instruction-arith-or)
- [`arith.xor`](#instruction-arith-xor)
- [`arith.not`](#instruction-arith-not)
- [`arith.cmp`](#instruction-arith-cmp)
- [`pdb.sdiv`](#instruction-pdb-sdiv)
- [`pdb.udiv`](#instruction-pdb-udiv)
- [`pdb.srem`](#instruction-pdb-srem)
- [`pdb.urem`](#instruction-pdb-urem)
- [`pdb.shl`](#instruction-pdb-shl)
- [`pdb.lshr`](#instruction-pdb-lshr)
- [`pdb.ashr`](#instruction-pdb-ashr)
- [`pdb.fptosi`](#instruction-pdb-fptosi)
- [`pdb.fptoui`](#instruction-pdb-fptoui)
- [`fp.add`](#instruction-fp-add)
- [`fp.sub`](#instruction-fp-sub)
- [`fp.mul`](#instruction-fp-mul)
- [`fp.div`](#instruction-fp-div)
- [`fp.neg`](#instruction-fp-neg)
- [`fp.fma`](#instruction-fp-fma)
- [`fp.cmp`](#instruction-fp-cmp)
- [`fp.assume_finite`](#instruction-fp-assume-finite)
- [`cast.int`](#instruction-cast-int)
- [`cast.int_to_float`](#instruction-cast-int-to-float)
- [`cast.float`](#instruction-cast-float)
- [`cast.bit`](#instruction-cast-bit)
- [`cast.ptr`](#instruction-cast-ptr)
- [`cast.ptr_access`](#instruction-cast-ptr-access)
- [`cast.ref_access`](#instruction-cast-ref-access)
- [`cast.class_up`](#instruction-cast-class-up)
- [`cast.interface_make`](#instruction-cast-interface-make)
- [`cast.interface_up`](#instruction-cast-interface-up)
- [`cast.try_class`](#instruction-cast-try-class)
- [`cast.try_interface`](#instruction-cast-try-interface)
- [`enum.value`](#instruction-enum-value)
- [`enum.init_variant`](#instruction-enum-init-variant)
- [`enum.discriminant`](#instruction-enum-discriminant)
- [`enum.is_variant`](#instruction-enum-is-variant)
- [`slice.init`](#instruction-slice-init)
- [`slice.data`](#instruction-slice-data)
- [`slice.length`](#instruction-slice-length)
- [`slice.index`](#instruction-slice-index)
- [`slice.subslice`](#instruction-slice-subslice)
- [`slice.init_empty`](#instruction-slice-init-empty)
- [`mem.alloca`](#instruction-mem-alloca)
- [`mem.global_place`](#instruction-mem-global-place)
- [`mem.load`](#instruction-mem-load)
- [`mem.store`](#instruction-mem-store)
- [`mem.load_unaligned`](#instruction-mem-load-unaligned)
- [`mem.store_unaligned`](#instruction-mem-store-unaligned)
- [`place.deref`](#instruction-place-deref)
- [`place.from_ref`](#instruction-place-from-ref)
- [`place.as_alive`](#instruction-place-as-alive)
- [`place.as_uninitialized`](#instruction-place-as-uninitialized)
- [`place.field`](#instruction-place-field)
- [`place.base`](#instruction-place-base)
- [`place.tuple_element`](#instruction-place-tuple-element)
- [`place.array_element`](#instruction-place-array-element)
- [`place.enum_payload`](#instruction-place-enum-payload)
- [`place.addr`](#instruction-place-addr)
- [`place.borrow`](#instruction-place-borrow)
- [`raw.load`](#instruction-raw-load)
- [`raw.store`](#instruction-raw-store)
- [`raw.memcpy`](#instruction-raw-memcpy)
- [`raw.memmove`](#instruction-raw-memmove)
- [`raw.memset`](#instruction-raw-memset)
- [`ptr.offset`](#instruction-ptr-offset)
- [`ptr.byte_offset`](#instruction-ptr-byte-offset)
- [`ptr.cmp`](#instruction-ptr-cmp)
- [`obj.init.begin`](#instruction-obj-init-begin)
- [`obj.init`](#instruction-obj-init)
- [`obj.init.copy`](#instruction-obj-init-copy)
- [`obj.init.commit`](#instruction-obj-init-commit)
- [`obj.assign.copy`](#instruction-obj-assign-copy)
- [`obj.destroy`](#instruction-obj-destroy)
- [`obj.destroy_dynamic`](#instruction-obj-destroy-dynamic)
- [`call.direct`](#instruction-call-direct)
- [`call.indirect`](#instruction-call-indirect)
- [`call.virtual`](#instruction-call-virtual)
- [`call.interface`](#instruction-call-interface)
- [`call.invoke`](#instruction-call-invoke)
- [`eh.entry`](#instruction-eh-entry)
- [`eh.match`](#instruction-eh-match)
- [`eh.payload`](#instruction-eh-payload)
- [`eh.end_catch`](#instruction-eh-end-catch)
- [`eh.throw`](#instruction-eh-throw)
- [`eh.throw_copy`](#instruction-eh-throw-copy)
- [`eh.throw_from`](#instruction-eh-throw-from)
- [`eh.rethrow`](#instruction-eh-rethrow)
- [`eh.resume`](#instruction-eh-resume)
- [`rt.trap`](#instruction-rt-trap)
- [`rt.fatal`](#instruction-rt-fatal)
- [`rt.version.pin`](#instruction-rt-version-pin)
- [`rt.version.unpin`](#instruction-rt-version-unpin)
- [`rt.version.transfer_pin`](#instruction-rt-version-transfer-pin)
- [`rt.version.current_owner`](#instruction-rt-version-current-owner)
- [`async.call`](#instruction-async-call)
- [`async.invoke`](#instruction-async-invoke)
- [`async.await`](#instruction-async-await)
- [`async.await_copy`](#instruction-async-await-copy)
- [`async.task.drive_once`](#instruction-async-task-drive-once)
- [`async.task.publish_success`](#instruction-async-task-publish-success)
- [`async.task.publish_failure`](#instruction-async-task-publish-failure)
- [`async.task.destroy`](#instruction-async-task-destroy)
- [`async.cancel.request`](#instruction-async-cancel-request)
- [`async.cancel.is_requested`](#instruction-async-cancel-is-requested)
- [`async.continuation_invoke`](#instruction-async-continuation-invoke)
- [`reflect.lookup_type`](#instruction-reflect-lookup-type)
- [`reflect.lookup_interface`](#instruction-reflect-lookup-interface)
- [`reflect.lookup_function`](#instruction-reflect-lookup-function)
- [`reflect.lookup_member`](#instruction-reflect-lookup-member)
- [`reflect.snapshot.clone`](#instruction-reflect-snapshot-clone)
- [`reflect.snapshot.release`](#instruction-reflect-snapshot-release)
- [`reflect.call`](#instruction-reflect-call)
- [`decorator.region`](#instruction-decorator-region)
- [`decorator.continuation_invoke`](#instruction-decorator-continuation-invoke)
- [`decorator.continuation_yield`](#instruction-decorator-continuation-yield)
- [`abi.call`](#instruction-abi-call)
- [`abi.invoke`](#instruction-abi-invoke)
- [`ct.register_module_item`](#instruction-ct-register-module-item)
- [`stage.force_value`](#instruction-stage-force-value)
- [`stage.force_block`](#instruction-stage-force-block)
- [`stage.select_if`](#instruction-stage-select-if)
- [`stage.select_match`](#instruction-stage-select-match)
- [`stage.expand_for`](#instruction-stage-expand-for)
- [`stage.expand_while`](#instruction-stage-expand-while)
- [`stage.instantiate`](#instruction-stage-instantiate)

<a id="instruction-const-int"></a>
## `const.int`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.int {constant = #cN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P2 `Constant` — `ConstantId`。Canonical text：`{constant = #cN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从ConstantTable物化一个类型与位宽准确匹配的整数SSA常量。类型/CFG约束：Constant kind int，result type/bit width 精确匹配。

### 用法

当程序语义需要从ConstantTable物化一个类型与位宽准确匹配的整数SSA常量时使用`const.int`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“Constant kind int，result type/bit width 精确匹配”。选择S1/P2变体时，必须同时满足`NullaryResult`的arity、`Constant`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

ConstantLike, Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `257` (`0x0101`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-const-bool"></a>
## `const.bool`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.bool {constant = #cN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P2 `Constant` — `ConstantId`。Canonical text：`{constant = #cN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从ConstantTable物化一个bool SSA常量。类型/CFG约束：Constant kind bool，result bool。

### 用法

当程序语义需要从ConstantTable物化一个bool SSA常量时使用`const.bool`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“Constant kind bool，result bool”。选择S1/P2变体时，必须同时满足`NullaryResult`的arity、`Constant`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

ConstantLike, Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `258` (`0x0102`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-const-float"></a>
## `const.float`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.float {constant = #cN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P2 `Constant` — `ConstantId`。Canonical text：`{constant = #cN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从ConstantTable物化一个格式与位模式准确匹配的浮点SSA常量。类型/CFG约束：Constant kind float，format/bits 精确匹配。

### 用法

当程序语义需要从ConstantTable物化一个格式与位模式准确匹配的浮点SSA常量时使用`const.float`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“Constant kind float，format/bits 精确匹配”。选择S1/P2变体时，必须同时满足`NullaryResult`的arity、`Constant`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

ConstantLike, Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `259` (`0x0103`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-const-null"></a>
## `const.null`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.null {constant = #cN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P2 `Constant` — `ConstantId`。Canonical text：`{constant = #cN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

物化指定raw pointer类型的null SSA常量。类型/CFG约束：Constant kind null，result raw pointer。

### 用法

当程序语义需要物化指定raw pointer类型的null SSA常量时使用`const.null`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“Constant kind null，result raw pointer”。选择S1/P2变体时，必须同时满足`NullaryResult`的arity、`Constant`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

ConstantLike, Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `260` (`0x0104`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-const-unit"></a>
## `const.unit`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.unit {constant = #cN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P2 `Constant` — `ConstantId`。Canonical text：`{constant = #cN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

物化Core唯一的零载荷unit SSA值。类型/CFG约束：Constant kind unit，result unit。

### 用法

当程序语义需要物化Core唯一的零载荷unit SSA值时使用`const.unit`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“Constant kind unit，result unit”。选择S1/P2变体时，必须同时满足`NullaryResult`的arity、`Constant`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

ConstantLike, Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `261` (`0x0105`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-const-symbol-addr"></a>
## `const.symbol_addr`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.symbol_addr {symbol = @symbol, addend = I} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P4 `SymbolAddend` — `SymbolId, Addend : I`。Canonical text：`{symbol = @symbol, addend = I}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

物化带显式加数的已解析数据符号raw地址。类型/CFG约束：symbol relocation，result compatible raw pointer。

### 用法

当程序语义需要物化带显式加数的已解析数据符号raw地址时使用`const.symbol_addr`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“symbol relocation，result compatible raw pointer”。选择S1/P4变体时，必须同时满足`NullaryResult`的arity、`SymbolAddend`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

ConstantLike, Speculatable, SymbolUser

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `262` (`0x0106`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-const-function"></a>
## `const.function`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = const.function {symbol = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P3 `Symbol` — `SymbolId`。Canonical text：`{symbol = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

物化绑定已解析函数符号与准确函数类型的函数指针常量。类型/CFG约束：symbol function signature 精确匹配，取得 stable entry。

### 用法

当程序语义需要物化绑定已解析函数符号与准确函数类型的函数指针常量时使用`const.function`；从ConstantTable把已验证常量物化为准确标量SSA值时使用；具体前置条件为“symbol function signature 精确匹配，取得 stable entry”。选择S1/P3变体时，必须同时满足`NullaryResult`的arity、`Symbol`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

ConstantLike, Speculatable, SymbolUser

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `263` (`0x0107`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cf-br"></a>
## `cf.br`

### 格式

Renderer：`branch`。Canonical renderer template：

`cf.br EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S10 `OneSuccessor` — `R0 O* G0 S1 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把控制无条件转移到唯一successor并传递其edge arguments。类型/CFG约束：successor role branch；全部 edge args existing_value。

### 用法

当程序语义需要把控制无条件转移到唯一successor并传递其edge arguments时使用`cf.br`；构造显式CFG边、合流或函数控制终点时使用；具体前置条件为“successor role branch；全部 edge args existing_value”。选择S10/P1变体时，必须同时满足`OneSuccessor`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, NoFallthrough

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `513` (`0x0201`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cf-cond-br"></a>
## `cf.cond_br`

### 格式

Renderer：`branch`。Canonical renderer template：

`cf.cond_br %condition, TRUE_EDGE, FALSE_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S11 `TwoSuccessors` — `R0 O* G0 S2 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按bool条件在true与false successor之间选择控制流。类型/CFG约束：operand 0 bool；successor true/false；其余 operands 只经 edge args 引用。

### 用法

当程序语义需要按bool条件在true与false successor之间选择控制流时使用`cf.cond_br`；构造显式CFG边、合流或函数控制终点时使用；具体前置条件为“operand 0 bool；successor true/false；其余 operands 只经 edge args 引用”。选择S11/P1变体时，必须同时满足`TwoSuccessors`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, NoFallthrough

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `514` (`0x0202`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cf-switch"></a>
## `cf.switch`

### 格式

Renderer：`switch`。Canonical renderer template：

`cf.switch %key [case #cN -> CASE_EDGE, ...] default DEFAULT_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S12 `ManySuccessors` — `R0 O* G0 S* D0`。

Payload：P8 `Switch` — `CaseCount : U, repeat (ConstantId, CaseSuccessorOrdinal : U), DefaultSuccessorOrdinal : U`。Canonical text：switch renderer打印`[case #cN -> EDGE, ...] default EDGE`，不打印dictionary。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按整数或规范enum判别值在有序case与required default之间分派控制流。类型/CFG约束：key 为 integer/enum discriminant；case unique 且按 constant canonical order；default 最后。

### 用法

当程序语义需要按整数或规范enum判别值在有序case与required default之间分派控制流时使用`cf.switch`；构造显式CFG边、合流或函数控制终点时使用；具体前置条件为“key 为 integer/enum discriminant；case unique 且按 constant canonical order；default 最后”。选择S12/P8变体时，必须同时满足`ManySuccessors`的arity、`Switch`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, NoFallthrough, Variadic

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `515` (`0x0203`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cf-select"></a>
## `cf.select`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cf.select %operand0, %operand1, %operand2 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S4 `TernaryResult` — `R1 O3 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按bool条件在两个同型SSA值之间执行无分支选择。类型/CFG约束：`(bool,T,T)->T`，T 为同一 scalar 或 unit。

### 用法

当程序语义需要按bool条件在两个同型SSA值之间执行无分支选择时使用`cf.select`；构造显式CFG边、合流或函数控制终点时使用；具体前置条件为“`(bool,T,T)->T`，T 为同一 scalar 或 unit”。选择S4/P1变体时，必须同时满足`TernaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `516` (`0x0204`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cf-return"></a>
## `cf.return`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`cf.return [%value] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S9 `VariadicEffect` — `R0 O* G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

结束当前函数并返回与逻辑结果通道匹配的零个或一个值。类型/CFG约束：void/destination result O0；value/unit result O1 且精确匹配；never 禁止。

### 用法

当程序语义需要结束当前函数并返回与逻辑结果通道匹配的零个或一个值时使用`cf.return`；构造显式CFG边、合流或函数控制终点时使用；具体前置条件为“void/destination result O0；value/unit result O1 且精确匹配；never 禁止”。选择S9/P1变体时，必须同时满足`VariadicEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Control

### 阶段

SC

### Traits

Terminator, NoFallthrough

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `517` (`0x0205`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cf-unreachable"></a>
## `cf.unreachable`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`cf.unreachable : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S5 `NullaryEffect` — `R0 O0 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

声明当前控制点在所有合法动态执行中不可到达。类型/CFG约束：verifier 证明无合法运行时到达路径。

### 用法

当程序语义需要声明当前控制点在所有合法动态执行中不可到达时使用`cf.unreachable`；构造显式CFG边、合流或函数控制终点时使用；具体前置条件为“verifier 证明无合法运行时到达路径”。选择S5/P1变体时，必须同时满足`NullaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Control

### 阶段

SC

### Traits

Terminator, NoNormalReturn, NoFallthrough

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `518` (`0x0206`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-add"></a>
## `arith.add`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.add %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对同型定宽整数或ptrsize执行模2^N加法。类型/CFG约束：`(I,I)->I` 同一整数，mod 2^N。

### 用法

当程序语义需要对同型定宽整数或ptrsize执行模2^N加法时使用`arith.add`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I,I)->I` 同一整数，mod 2^N”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `769` (`0x0301`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-sub"></a>
## `arith.sub`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.sub %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对同型定宽整数或ptrsize执行模2^N减法。类型/CFG约束：`(I,I)->I` 同一整数，结果为左操作数减右操作数并按mod 2^N截断。

### 用法

当程序语义需要对同型定宽整数或ptrsize执行模2^N减法时使用`arith.sub`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I,I)->I` 同一整数，结果为左操作数减右操作数并按mod 2^N截断”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `770` (`0x0302`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-mul"></a>
## `arith.mul`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.mul %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对同型定宽整数或ptrsize执行模2^N乘法。类型/CFG约束：`(I,I)->I` 同一整数，结果为完整乘积的低N位即mod 2^N。

### 用法

当程序语义需要对同型定宽整数或ptrsize执行模2^N乘法时使用`arith.mul`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I,I)->I` 同一整数，结果为完整乘积的低N位即mod 2^N”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `771` (`0x0303`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-neg"></a>
## `arith.neg`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.neg %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

计算定宽整数或ptrsize的模2^N加法逆元。类型/CFG约束：`(I)->I`。

### 用法

当程序语义需要计算定宽整数或ptrsize的模2^N加法逆元时使用`arith.neg`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I)->I`”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `772` (`0x0304`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-and"></a>
## `arith.and`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.and %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对同型定宽整数或ptrsize执行逐位与。类型/CFG约束：`(I,I)->I`。

### 用法

当程序语义需要对同型定宽整数或ptrsize执行逐位与时使用`arith.and`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I,I)->I`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `773` (`0x0305`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-or"></a>
## `arith.or`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.or %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对同型定宽整数或ptrsize执行逐位或。类型/CFG约束：`(I,I)->I`。

### 用法

当程序语义需要对同型定宽整数或ptrsize执行逐位或时使用`arith.or`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I,I)->I`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `774` (`0x0306`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-xor"></a>
## `arith.xor`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.xor %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对同型定宽整数或ptrsize执行逐位异或。类型/CFG约束：`(I,I)->I`。

### 用法

当程序语义需要对同型定宽整数或ptrsize执行逐位异或时使用`arith.xor`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I,I)->I`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `775` (`0x0307`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-not"></a>
## `arith.not`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.not %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对定宽整数或ptrsize执行逐位取反。类型/CFG约束：`(I)->I`。

### 用法

当程序语义需要对定宽整数或ptrsize执行逐位取反时使用`arith.not`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(I)->I`”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `776` (`0x0308`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-arith-cmp"></a>
## `arith.cmp`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = arith.cmp %operand0, %operand1 {predicate = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P5 `IntegerPredicate` — `Enum<IntegerPredicateTag>`。Canonical text：`{predicate = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按已注册整数predicate比较两个同型整数、ptrsize或bool值并产生bool。类型/CFG约束：`(T,T)->bool`；T为同一整数且predicate与signedness相容，或T为同一bool且predicate仅eq/ne。

### 用法

当程序语义需要按已注册整数predicate比较两个同型整数、ptrsize或bool值并产生bool时使用`arith.cmp`；对定宽整数或ptrsize执行已定义的算术/比较语义时使用；具体前置条件为“`(T,T)->bool`；T为同一整数且predicate与signedness相容，或T为同一bool且predicate仅eq/ne”。选择S3/P5变体时，必须同时满足`BinaryResult`的arity、`IntegerPredicate`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `777` (`0x0309`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-sdiv"></a>
## `pdb.sdiv`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.sdiv %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行有符号整数除法并产生向零截断的商。类型/CFG约束：`(iN,iN)->iN`。

### 用法

当程序语义需要按PDB边界规则执行有符号整数除法并产生向零截断的商时使用`pdb.sdiv`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(iN,iN)->iN`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1025` (`0x0401`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-udiv"></a>
## `pdb.udiv`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.udiv %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行无符号整数除法并产生商。类型/CFG约束：`(uN,uN)->uN`。

### 用法

当程序语义需要按PDB边界规则执行无符号整数除法并产生商时使用`pdb.udiv`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(uN,uN)->uN`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1026` (`0x0402`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-srem"></a>
## `pdb.srem`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.srem %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行有符号整数除法并产生余数。类型/CFG约束：`(iN,iN)->iN`。

### 用法

当程序语义需要按PDB边界规则执行有符号整数除法并产生余数时使用`pdb.srem`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(iN,iN)->iN`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1027` (`0x0403`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-urem"></a>
## `pdb.urem`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.urem %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行无符号整数除法并产生余数。类型/CFG约束：`(uN,uN)->uN`。

### 用法

当程序语义需要按PDB边界规则执行无符号整数除法并产生余数时使用`pdb.urem`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(uN,uN)->uN`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1028` (`0x0404`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-shl"></a>
## `pdb.shl`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.shl %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行定宽整数左移。类型/CFG约束：`(I,ptrsize)->I`。

### 用法

当程序语义需要按PDB边界规则执行定宽整数左移时使用`pdb.shl`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(I,ptrsize)->I`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1029` (`0x0405`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-lshr"></a>
## `pdb.lshr`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.lshr %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行零扩展逻辑右移。类型/CFG约束：`(uN/ptrsize,ptrsize)->same`。

### 用法

当程序语义需要按PDB边界规则执行零扩展逻辑右移时使用`pdb.lshr`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(uN/ptrsize,ptrsize)->same`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1030` (`0x0406`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-ashr"></a>
## `pdb.ashr`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.ashr %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB边界规则执行符号扩展算术右移。类型/CFG约束：`(iN,ptrsize)->iN`。

### 用法

当程序语义需要按PDB边界规则执行符号扩展算术右移时使用`pdb.ashr`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(iN,ptrsize)->iN`”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1031` (`0x0407`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-fptosi"></a>
## `pdb.fptosi`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.fptosi %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB浮点边界规则把浮点值转换为有符号整数。类型/CFG约束：`(F)->iN`。

### 用法

当程序语义需要按PDB浮点边界规则把浮点值转换为有符号整数时使用`pdb.fptosi`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(F)->iN`”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1032` (`0x0408`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-pdb-fptoui"></a>
## `pdb.fptoui`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = pdb.fptoui %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按PDB浮点边界规则把浮点值转换为无符号整数。类型/CFG约束：`(F)->uN/ptrsize`。

### 用法

当程序语义需要按PDB浮点边界规则把浮点值转换为无符号整数时使用`pdb.fptoui`；需要按PDB表处理目标相关边界行为时使用；具体前置条件为“`(F)->uN/ptrsize`”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TargetDependent, PdbBoundary, MayTrap

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.1](./10-schema-registry.md#131-constcfarith-与-pdb)，OpcodeTag `1033` (`0x0409`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-add"></a>
## `fp.add`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.add %operand0, %operand1 [{fast = [FAST_MATH_FLAGS]}] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P7 `FastMath` — `Bits<FastMathFlagBit>`。Canonical text：空集不打印payload；非空打印`{fast = [name, ...]}`，按bit递增。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TargetContext与fast-math标志执行浮点加法。类型/CFG约束：`(F,F)->F`，fast flags 受 TargetContext 验证。

### 用法

当程序语义需要按TargetContext与fast-math标志执行浮点加法时使用`fp.add`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F,F)->F`，fast flags 受 TargetContext 验证”。选择S3/P7变体时，必须同时满足`BinaryResult`的arity、`FastMath`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1281` (`0x0501`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-sub"></a>
## `fp.sub`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.sub %operand0, %operand1 [{fast = [FAST_MATH_FLAGS]}] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P7 `FastMath` — `Bits<FastMathFlagBit>`。Canonical text：空集不打印payload；非空打印`{fast = [name, ...]}`，按bit递增。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TargetContext与fast-math标志执行浮点减法。类型/CFG约束：`(F,F)->F` 同一浮点格式，执行左操作数减右操作数并按TargetContext strict模式与已验证fast flags舍入。

### 用法

当程序语义需要按TargetContext与fast-math标志执行浮点减法时使用`fp.sub`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F,F)->F` 同一浮点格式，执行左操作数减右操作数并按TargetContext strict模式与已验证fast flags舍入”。选择S3/P7变体时，必须同时满足`BinaryResult`的arity、`FastMath`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1282` (`0x0502`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-mul"></a>
## `fp.mul`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.mul %operand0, %operand1 [{fast = [FAST_MATH_FLAGS]}] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P7 `FastMath` — `Bits<FastMathFlagBit>`。Canonical text：空集不打印payload；非空打印`{fast = [name, ...]}`，按bit递增。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TargetContext与fast-math标志执行浮点乘法。类型/CFG约束：`(F,F)->F` 同一浮点格式，执行乘法并按TargetContext strict模式与已验证fast flags舍入。

### 用法

当程序语义需要按TargetContext与fast-math标志执行浮点乘法时使用`fp.mul`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F,F)->F` 同一浮点格式，执行乘法并按TargetContext strict模式与已验证fast flags舍入”。选择S3/P7变体时，必须同时满足`BinaryResult`的arity、`FastMath`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1283` (`0x0503`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-div"></a>
## `fp.div`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.div %operand0, %operand1 [{fast = [FAST_MATH_FLAGS]}] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P7 `FastMath` — `Bits<FastMathFlagBit>`。Canonical text：空集不打印payload；非空打印`{fast = [name, ...]}`，按bit递增。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TargetContext与fast-math标志执行浮点除法。类型/CFG约束：`(F,F)->F` 同一浮点格式，执行除法并按TargetContext strict模式与已验证fast flags舍入；浮点除零不是Ink trap。

### 用法

当程序语义需要按TargetContext与fast-math标志执行浮点除法时使用`fp.div`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F,F)->F` 同一浮点格式，执行除法并按TargetContext strict模式与已验证fast flags舍入；浮点除零不是Ink trap”。选择S3/P7变体时，必须同时满足`BinaryResult`的arity、`FastMath`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1284` (`0x0504`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-neg"></a>
## `fp.neg`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.neg %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

翻转浮点值的符号语义以执行浮点取负。类型/CFG约束：`(F)->F`，只翻转 sign bit。

### 用法

当程序语义需要翻转浮点值的符号语义以执行浮点取负时使用`fp.neg`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F)->F`，只翻转 sign bit”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1285` (`0x0505`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-fma"></a>
## `fp.fma`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.fma %operand0, %operand1, %operand2 [{fast = [FAST_MATH_FLAGS]}] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S4 `TernaryResult` — `R1 O3 G0 S0 D0`。

Payload：P7 `FastMath` — `Bits<FastMathFlagBit>`。Canonical text：空集不打印payload；非空打印`{fast = [name, ...]}`，按bit递增。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TargetContext与fast-math标志执行单次舍入的浮点乘加。类型/CFG约束：`(F,F,F)->F`。

### 用法

当程序语义需要按TargetContext与fast-math标志执行单次舍入的浮点乘加时使用`fp.fma`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F,F,F)->F`”。选择S4/P7变体时，必须同时满足`TernaryResult`的arity、`FastMath`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1286` (`0x0506`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-cmp"></a>
## `fp.cmp`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.cmp %operand0, %operand1 {predicate = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P6 `FloatPredicate` — `Enum<FloatPredicateTag>`。Canonical text：`{predicate = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按已注册浮点predicate与TargetContext比较两个同型浮点值并产生bool。类型/CFG约束：`(F,F)->bool`。

### 用法

当程序语义需要按已注册浮点predicate与TargetContext比较两个同型浮点值并产生bool时使用`fp.cmp`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F,F)->bool`”。选择S3/P6变体时，必须同时满足`BinaryResult`的arity、`FloatPredicate`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1287` (`0x0507`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-fp-assume-finite"></a>
## `fp.assume_finite`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = fp.assume_finite %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把已验证的浮点有限性事实显式建立为后续优化可依赖的证明。类型/CFG约束：`(F)->F`；输入必须已证明 finite。

### 用法

当程序语义需要把已验证的浮点有限性事实显式建立为后续优化可依赖的证明时使用`fp.assume_finite`；按TargetContext浮点模式执行浮点运算时使用；具体前置条件为“`(F)->F`；输入必须已证明 finite”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1288` (`0x0508`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-int"></a>
## `cast.int`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.int %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在中央允许的整数宽度与符号解释之间执行准确整数转换。类型/CFG约束：integer 到 integer；result type 给出宽度/signedness。

### 用法

当程序语义需要在中央允许的整数宽度与符号解释之间执行准确整数转换时使用`cast.int`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“integer 到 integer；result type 给出宽度/signedness”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1537` (`0x0601`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-int-to-float"></a>
## `cast.int_to_float`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.int_to_float %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把整数或ptrsize数值转换为目标浮点格式。类型/CFG约束：integer 到 float，RNE。

### 用法

当程序语义需要把整数或ptrsize数值转换为目标浮点格式时使用`cast.int_to_float`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“integer 到 float，RNE”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1538` (`0x0602`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-float"></a>
## `cast.float`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.float %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在两个已注册浮点格式之间执行精度转换。类型/CFG约束：float 到 float。

### 用法

当程序语义需要在两个已注册浮点格式之间执行精度转换时使用`cast.float`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“float 到 float”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1539` (`0x0603`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-bit"></a>
## `cast.bit`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.bit %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在中央允许的两个同位宽标量类型之间重解释位模式。类型/CFG约束：等width bitcast；operand/result只允许同宽fixed integer、`ptrsize`或float；明确排除`bool`、raw/function pointer、reference/place、aggregate、runtime handle/object与exception。

### 用法

当程序语义需要在中央允许的两个同位宽标量类型之间重解释位模式时使用`cast.bit`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“等width bitcast；operand/result只允许同宽fixed integer、`ptrsize`或float；明确排除`bool`、raw/function pointer、reference/place、aggregate、runtime handle/object与exception”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1540` (`0x0604`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-ptr"></a>
## `cast.ptr`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.ptr %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在兼容address space内改变raw pointer的静态pointee类型。类型/CFG约束：raw pointer/ptrsize 间或 compatible raw pointer。

### 用法

当程序语义需要在兼容address space内改变raw pointer的静态pointee类型时使用`cast.ptr`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“raw pointer/ptrsize 间或 compatible raw pointer”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1541` (`0x0605`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-ptr-access"></a>
## `cast.ptr_access`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.ptr_access %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

收窄raw pointer的静态访问权限且保持地址不变。类型/CFG约束：`ptr<rw,T,A> -> ptr<ro,T,A>`。

### 用法

当程序语义需要收窄raw pointer的静态访问权限且保持地址不变时使用`cast.ptr_access`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“`ptr<rw,T,A> -> ptr<ro,T,A>`”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1542` (`0x0606`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-ref-access"></a>
## `cast.ref_access`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.ref_access %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

收窄reference的静态访问权限且保持对象身份不变。类型/CFG约束：`ref<rw,T> -> ref<ro,T>`。

### 用法

当程序语义需要收窄reference的静态访问权限且保持对象身份不变时使用`cast.ref_access`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“`ref<rw,T> -> ref<ro,T>`”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.2](./10-schema-registry.md#132-fp-与-cast)，OpcodeTag `1543` (`0x0607`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-class-up"></a>
## `cast.class_up`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = cast.class_up %operand0 {base = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

沿已验证class继承边把对象view上转为指定base class。类型/CFG约束：class ref -> declared concrete base ref；保持 access/lifetime。

### 用法

当程序语义需要沿已验证class继承边把对象view上转为指定base class时使用`cast.class_up`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“class ref -> declared concrete base ref；保持 access/lifetime”。选择S2/P9变体时，必须同时满足`UnaryResult`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable, SymbolUser

### 来源

[Schema Registry §14.1](./10-schema-registry.md#141-dynamic-cast)，OpcodeTag `1601` (`0x0641`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-interface-make"></a>
## `cast.interface_make`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`cast.interface_make OPERANDS {interface = @symbol} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S15 `DestinationEffect` — `R0 O* G0 S0 D1`。

Payload：P18 `InterfaceMake` — `InterfaceSymbolId, DestinationPayload`。Canonical text：`{interface = @symbol}`后打印required destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

为对象构造目标interface reference并提交required destination。类型/CFG约束：object ref + destination；constructed type 是普通 address-only interface_reference；唯一 runtime commit。

### 用法

当程序语义需要为对象构造目标interface reference并提交required destination时使用`cast.interface_make`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“object ref + destination；constructed type 是普通 address-only interface_reference；唯一 runtime commit”。选择S15/P18变体时，必须同时满足`DestinationEffect`的arity、`InterfaceMake`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_metadata), W(typed), BeginLifetime, TargetDependent, RT(interface_lookup)

### 阶段

SC

### Traits

DestinationChannel, RuntimeSemantic, SymbolUser

### 来源

[Schema Registry §14.1](./10-schema-registry.md#141-dynamic-cast)，OpcodeTag `1602` (`0x0642`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-interface-up"></a>
## `cast.interface_up`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`cast.interface_up OPERANDS {ancestor = @symbol} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S15 `DestinationEffect` — `R0 O* G0 S0 D1`。

Payload：P19 `InterfaceUp` — `AncestorInterfaceSymbolId, DestinationPayload`。Canonical text：`{ancestor = @symbol}`后打印required destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把interface reference上转为已验证ancestor interface并提交destination。类型/CFG约束：readable interface-reference place + destination；ancestor registered；唯一 runtime commit。

### 用法

当程序语义需要把interface reference上转为已验证ancestor interface并提交destination时使用`cast.interface_up`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“readable interface-reference place + destination；ancestor registered；唯一 runtime commit”。选择S15/P19变体时，必须同时满足`DestinationEffect`的arity、`InterfaceUp`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed), R(runtime_metadata), W(typed), BeginLifetime, TargetDependent

### 阶段

SC

### Traits

DestinationChannel, RuntimeSemantic, SymbolUser

### 来源

[Schema Registry §14.1](./10-schema-registry.md#141-dynamic-cast)，OpcodeTag `1603` (`0x0643`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-try-class"></a>
## `cast.try_class`

### 格式

Renderer：`generic_operation`。Canonical renderer template（变体 1）：

`%result0 = cast.try_class %operand0 {target = @symbol, result_form = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P20 `TryClass` — `TargetClassSymbolId, Enum<TryClassResultFormTag>, Opt<DestinationPayload>`。Canonical text：`{target = @symbol, result_form = name}`后按present打印destination clause。

Renderer：`generic_operation`。Canonical renderer template（变体 2）：

`cast.try_class OPERANDS {target = @symbol, result_form = name} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S15 `DestinationEffect` — `R0 O* G0 S0 D1`。

Payload：P20 `TryClass` — `TargetClassSymbolId, Enum<TryClassResultFormTag>, Opt<DestinationPayload>`。Canonical text：`{target = @symbol, result_form = name}`后按present打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对class或interface view执行可失败的运行时class转换。类型/CFG约束：source 有效 class/interface view；raw-pointer form 失败为 null；optional-reference form destination 构造 Some/None。

### 用法

当程序语义需要对class或interface view执行可失败的运行时class转换时使用`cast.try_class`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“source 有效 class/interface view；raw-pointer form 失败为 null；optional-reference form destination 构造 Some/None”。选择S2/P20变体时，必须同时满足`UnaryResult`的arity、`TryClass`的字段顺序和该opcode的type/CFG rule；选择S15/P20变体时，必须同时满足`DestinationEffect`的arity、`TryClass`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_metadata), RT(dynamic_cast), TargetDependent；object 再加 W(typed), BeginLifetime

### 阶段

SC

### Traits

DestinationChannel, RuntimeSemantic, SymbolUser

### 来源

[Schema Registry §14.1](./10-schema-registry.md#141-dynamic-cast)，OpcodeTag `1604` (`0x0644`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-cast-try-interface"></a>
## `cast.try_interface`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`cast.try_interface OPERANDS {target = @symbol} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S15 `DestinationEffect` — `R0 O* G0 S0 D1`。

Payload：P21 `TryInterface` — `TargetInterfaceSymbolId, DestinationPayload`。Canonical text：`{target = @symbol}`后打印required destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

对class或interface view执行可失败的运行时interface转换并写入Optional destination。类型/CFG约束：source 有效 class/interface view；ordinary Optional<InterfaceRef> destination 构造 Some/None。

### 用法

当程序语义需要对class或interface view执行可失败的运行时interface转换并写入Optional destination时使用`cast.try_interface`；在中央转换闭集允许的两种准确类型之间转换时使用；具体前置条件为“source 有效 class/interface view；ordinary Optional<InterfaceRef> destination 构造 Some/None”。选择S15/P21变体时，必须同时满足`DestinationEffect`的arity、`TryInterface`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_metadata), W(typed), BeginLifetime, RT(dynamic_cast), TargetDependent

### 阶段

SC

### Traits

DestinationChannel, RuntimeSemantic, SymbolUser

### 来源

[Schema Registry §14.1](./10-schema-registry.md#141-dynamic-cast)，OpcodeTag `1605` (`0x0645`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-enum-value"></a>
## `enum.value`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = enum.value {variant = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

物化TargetABI已分类为单标量的无载荷enum variant值。类型/CFG约束：selector 是无 payload variant；result 是 scalar enum。

### 用法

当程序语义需要物化TargetABI已分类为单标量的无载荷enum variant值时使用`enum.value`；建立、查询或读取enum判别与payload时使用；具体前置条件为“selector 是无 payload variant；result 是 scalar enum”。选择S1/P9变体时，必须同时满足`NullaryResult`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable, SymbolUser

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `1793` (`0x0701`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-enum-init-variant"></a>
## `enum.init_variant`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`enum.init_variant %operand0 {variant = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在active enum初始化事务中选择并建立指定活动variant。类型/CFG约束：operand `!place<init,E>` active；variant 属于 E。

### 用法

当程序语义需要在active enum初始化事务中选择并建立指定活动variant时使用`enum.init_variant`；建立、查询或读取enum判别与payload时使用；具体前置条件为“operand `!place<init,E>` active；variant 属于 E”。选择S6/P9变体时，必须同时满足`UnaryEffect`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed), BeginLifetime, TargetDependent

### 阶段

SC

### Traits

Lifetime, SymbolUser

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `1794` (`0x0702`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-enum-discriminant"></a>
## `enum.discriminant`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = enum.discriminant %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

读取enum的语义分支序号而不暴露物理tag或niche编码。类型/CFG约束：scalar enum 或 readable enum place -> ptrsize。

### 用法

当程序语义需要读取enum的语义分支序号而不暴露物理tag或niche编码时使用`enum.discriminant`；建立、查询或读取enum判别与payload时使用；具体前置条件为“scalar enum 或 readable enum place -> ptrsize”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure 或 R(typed), TargetDependent

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `1795` (`0x0703`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-enum-is-variant"></a>
## `enum.is_variant`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = enum.is_variant %operand0 {variant = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

判断enum当前活动分支是否为指定variant。类型/CFG约束：scalar enum 或 readable enum place -> bool。

### 用法

当程序语义需要判断enum当前活动分支是否为指定variant时使用`enum.is_variant`；建立、查询或读取enum判别与payload时使用；具体前置条件为“scalar enum 或 readable enum place -> bool”。选择S2/P9变体时，必须同时满足`UnaryResult`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure 或 R(typed), TargetDependent

### 阶段

SC

### Traits

SymbolUser

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `1796` (`0x0704`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-slice-init"></a>
## `slice.init`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`slice.init %operand0, %operand1, %operand2 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S8 `TernaryEffect` — `R0 O3 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从首元素place与长度初始化非空安全slice的data、length及borrow generation。类型/CFG约束：`(%dst:!place<init,slice<A,T>>, %first:!place<ro/rw,T>, %length:ptrsize)->()`；length > 0；first 捕获当前 ObjectLifetimeGeneration。

### 用法

当程序语义需要从首元素place与长度初始化非空安全slice的data、length及borrow generation时使用`slice.init`；建立或使用generation-aware安全slice时使用；具体前置条件为“`(%dst:!place<init,slice<A,T>>, %first:!place<ro/rw,T>, %length:ptrsize)->()`；length > 0；first 捕获当前 ObjectLifetimeGeneration”。选择S8/P1变体时，必须同时满足`TernaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed), W(typed), BeginLifetime

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `2049` (`0x0801`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-slice-data"></a>
## `slice.data`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = slice.data %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

读取安全slice的物理data raw pointer并显式丢弃checked borrow generation。类型/CFG约束：readable slice place -> compatible raw pointer；显式丢失 generation authority。

### 用法

当程序语义需要读取安全slice的物理data raw pointer并显式丢弃checked borrow generation时使用`slice.data`；建立或使用generation-aware安全slice时使用；具体前置条件为“readable slice place -> compatible raw pointer；显式丢失 generation authority”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `2050` (`0x0802`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-slice-length"></a>
## `slice.length`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = slice.length %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

读取安全slice的ptrsize元素数量。类型/CFG约束：readable slice place -> ptrsize。

### 用法

当程序语义需要读取安全slice的ptrsize元素数量时使用`slice.length`；建立或使用generation-aware安全slice时使用；具体前置条件为“readable slice place -> ptrsize”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `2051` (`0x0803`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-slice-index"></a>
## `slice.index`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = slice.index %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在bounds与generation验证后取得安全slice中指定元素的place。类型/CFG约束：readable slice place + ptrsize -> element place；按TargetContext sizeof(T)计算地址；结果携带 borrow authority + selected generation。

### 用法

当程序语义需要在bounds与generation验证后取得安全slice中指定元素的place时使用`slice.index`；建立或使用generation-aware安全slice时使用；具体前置条件为“readable slice place + ptrsize -> element place；按TargetContext sizeof(T)计算地址；结果携带 borrow authority + selected generation”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed), MayTrap, TargetDependent

### 阶段

SC

### Traits

PlaceProducing

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `2052` (`0x0804`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-slice-subslice"></a>
## `slice.subslice`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`slice.subslice OPERANDS : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S9 `VariadicEffect` — `R0 O* G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在bounds与generation验证后派生安全slice的连续子区间。类型/CFG约束：dst init slice、source readable slice、begin/end ptrsize；按TargetContext sizeof(T)计算data；结果保留所选子范围 generation set。

### 用法

当程序语义需要在bounds与generation验证后派生安全slice的连续子区间时使用`slice.subslice`；建立或使用generation-aware安全slice时使用；具体前置条件为“dst init slice、source readable slice、begin/end ptrsize；按TargetContext sizeof(T)计算data；结果保留所选子范围 generation set”。选择S9/P1变体时，必须同时满足`VariadicEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed), W(typed), BeginLifetime, MayTrap, TargetDependent

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `2053` (`0x0805`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-slice-init-empty"></a>
## `slice.init_empty`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`slice.init_empty %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把active destination初始化为canonical null加零长度的空slice。类型/CFG约束：`!place<init,slice<A,T>> -> ()`；建立 canonical `(null,0)` 与空 BorrowGenerationSet。

### 用法

当程序语义需要把active destination初始化为canonical null加零长度的空slice时使用`slice.init_empty`；建立或使用generation-aware安全slice时使用；具体前置条件为“`!place<init,slice<A,T>> -> ()`；建立 canonical `(null,0)` 与空 BorrowGenerationSet”。选择S6/P1变体时，必须同时满足`UnaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed), BeginLifetime

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.3](./10-schema-registry.md#133-enum-与-generation-aware-slice)，OpcodeTag `2054` (`0x0806`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-mem-alloca"></a>
## `mem.alloca`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = mem.alloca : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在当前函数frame中分配未初始化的typed stack storage并产生init place。类型/CFG约束：result `!place<init,T>`，T SizedObjectType。

### 用法

当程序语义需要在当前函数frame中分配未初始化的typed stack storage并产生init place时使用`mem.alloca`；访问已验证typed allocation/global/object storage时使用；具体前置条件为“result `!place<init,T>`，T SizedObjectType”。选择S1/P1变体时，必须同时满足`NullaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

A(stack)

### 阶段

SC

### Traits

PlaceProducing

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2305` (`0x0901`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-mem-global-place"></a>
## `mem.global_place`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = mem.global_place {symbol = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P3 `Symbol` — `SymbolId`。Canonical text：`{symbol = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

取得已完成对应lifecycle激活的global或thread-local对象place。类型/CFG约束：symbol global；普通 use 只产生 matching generation 的 borrow-authority place，绝不产生 owner。

### 用法

当程序语义需要取得已完成对应lifecycle激活的global或thread-local对象place时使用`mem.global_place`；访问已验证typed allocation/global/object storage时使用；具体前置条件为“symbol global；普通 use 只产生 matching generation 的 borrow-authority place，绝不产生 owner”。选择S1/P3变体时，必须同时满足`NullaryResult`的arity、`Symbol`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

PlaceProducing, SymbolUser

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2306` (`0x0902`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-mem-load"></a>
## `mem.load`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = mem.load %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从Alive且对齐的typed place读取一个允许按值加载的值。类型/CFG约束：readable Alive place of scalar/unit -> T。

### 用法

当程序语义需要从Alive且对齐的typed place读取一个允许按值加载的值时使用`mem.load`；访问已验证typed allocation/global/object storage时使用；具体前置条件为“readable Alive place of scalar/unit -> T”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2307` (`0x0903`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-mem-store"></a>
## `mem.store`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`mem.store %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把一个同型值写入Alive且对齐的可写typed place。类型/CFG约束：`(T,!place<rw,T>)->()`，Alive scalar/unit。

### 用法

当程序语义需要把一个同型值写入Alive且对齐的可写typed place时使用`mem.store`；访问已验证typed allocation/global/object storage时使用；具体前置条件为“`(T,!place<rw,T>)->()`，Alive scalar/unit”。选择S7/P1变体时，必须同时满足`BinaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2308` (`0x0904`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-mem-load-unaligned"></a>
## `mem.load_unaligned`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = mem.load_unaligned %operand0 {alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P11 `Alignment` — `Alignment : U`。Canonical text：`{alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按显式alignment从允许非对齐访问的typed place读取值。类型/CFG约束：readable Alive place -> scalar。

### 用法

当程序语义需要按显式alignment从允许非对齐访问的typed place读取值时使用`mem.load_unaligned`；访问已验证typed allocation/global/object storage时使用；具体前置条件为“readable Alive place -> scalar”。选择S2/P11变体时，必须同时满足`UnaryResult`的arity、`Alignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2309` (`0x0905`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-mem-store-unaligned"></a>
## `mem.store_unaligned`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`mem.store_unaligned %operand0, %operand1 {alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P11 `Alignment` — `Alignment : U`。Canonical text：`{alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按显式alignment向允许非对齐访问的typed place写入值。类型/CFG约束：value + writable Alive place。

### 用法

当程序语义需要按显式alignment向允许非对齐访问的typed place写入值时使用`mem.store_unaligned`；访问已验证typed allocation/global/object storage时使用；具体前置条件为“value + writable Alive place”。选择S7/P11变体时，必须同时满足`BinaryEffect`的arity、`Alignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2310` (`0x0906`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-deref"></a>
## `place.deref`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.deref %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把满足pointee与lifetime前置条件的raw pointer解释为typed place capability。类型/CFG约束：compatible raw pointer -> borrow-authority place；T 非 SealedRuntimeStorage；raw 前置成立时捕获当前 ObjectLifetimeGeneration。

### 用法

当程序语义需要把满足pointee与lifetime前置条件的raw pointer解释为typed place capability时使用`place.deref`；派生或重绑定typed place capability时使用；具体前置条件为“compatible raw pointer -> borrow-authority place；T 非 SealedRuntimeStorage；raw 前置成立时捕获当前 ObjectLifetimeGeneration”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

PlaceProducing

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2561` (`0x0A01`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-from-ref"></a>
## `place.from_ref`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.from_ref %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从reference借用语义派生访问权限不增强的typed place。类型/CFG约束：reference -> same access/type place。

### 用法

当程序语义需要从reference借用语义派生访问权限不增强的typed place时使用`place.from_ref`；派生或重绑定typed place capability时使用；具体前置条件为“reference -> same access/type place”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

PlaceProducing

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2562` (`0x0A02`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-as-alive"></a>
## `place.as_alive`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.as_alive %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把路径上已证明Alive的init place重绑定为ro或rw place的新capability generation。类型/CFG约束：`!place<init,T>` -> `!place<ro/rw,T>`；当前 path 已有 commit/destination normal Alive proof。

### 用法

当程序语义需要把路径上已证明Alive的init place重绑定为ro或rw place的新capability generation时使用`place.as_alive`；派生或重绑定typed place capability时使用；具体前置条件为“`!place<init,T>` -> `!place<ro/rw,T>`；当前 path 已有 commit/destination normal Alive proof”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

PlaceProducing, CapabilityRebind

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2563` (`0x0A03`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-as-uninitialized"></a>
## `place.as_uninitialized`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.as_uninitialized %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把路径上已销毁且可重建的owner place重绑定为fresh init capability。类型/CFG约束：owner-authority `!place<rw,T>` -> `!place<init,T>`；destroy 后准确 state 为 AllocatedUninitialized，无 live borrow/child capability；readonly 非法。

### 用法

当程序语义需要把路径上已销毁且可重建的owner place重绑定为fresh init capability时使用`place.as_uninitialized`；派生或重绑定typed place capability时使用；具体前置条件为“owner-authority `!place<rw,T>` -> `!place<init,T>`；destroy 后准确 state 为 AllocatedUninitialized，无 live borrow/child capability；readonly 非法”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

PlaceProducing, CapabilityRebind

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2564` (`0x0A04`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-field"></a>
## `place.field`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.field %operand0 {field = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

投影到class或struct中由symbol指定的字段子对象place。类型/CFG约束：field 属于准确 class；保留 path/access 上限。

### 用法

当程序语义需要投影到class或struct中由symbol指定的字段子对象place时使用`place.field`；派生或重绑定typed place capability时使用；具体前置条件为“field 属于准确 class；保留 path/access 上限”。选择S2/P9变体时，必须同时满足`UnaryResult`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

PlaceProducing, SymbolUser

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2565` (`0x0A05`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-base"></a>
## `place.base`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.base %operand0 {base = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

投影到class对象中由symbol指定的base子对象place。类型/CFG约束：concrete declared base path。

### 用法

当程序语义需要投影到class对象中由symbol指定的base子对象place时使用`place.base`；派生或重绑定typed place capability时使用；具体前置条件为“concrete declared base path”。选择S2/P9变体时，必须同时满足`UnaryResult`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

PlaceProducing, SymbolUser

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2566` (`0x0A06`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-tuple-element"></a>
## `place.tuple_element`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.tuple_element %operand0 {index = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P10 `StaticIndex` — `Index : U`。Canonical text：`{index = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按静态index投影到tuple的元素子对象place。类型/CFG约束：static tuple index 合法。

### 用法

当程序语义需要按静态index投影到tuple的元素子对象place时使用`place.tuple_element`；派生或重绑定typed place capability时使用；具体前置条件为“static tuple index 合法”。选择S2/P10变体时，必须同时满足`UnaryResult`的arity、`StaticIndex`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

PlaceProducing

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2567` (`0x0A07`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-array-element"></a>
## `place.array_element`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.array_element %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按动态ptrsize index与bounds证明投影到array元素place。类型/CFG约束：array place + ptrsize -> element place。

### 用法

当程序语义需要按动态ptrsize index与bounds证明投影到array元素place时使用`place.array_element`；派生或重绑定typed place capability时使用；具体前置条件为“array place + ptrsize -> element place”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent, MayTrap

### 阶段

SC

### Traits

PlaceProducing

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2568` (`0x0A08`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-enum-payload"></a>
## `place.enum_payload`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.enum_payload %operand0 {variant = @symbol} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P9 `SelectorSymbol` — `SymbolId`。Canonical text：key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在匹配活动variant证明下投影到enum payload子对象place。类型/CFG约束：active/partially initialized variant proof。

### 用法

当程序语义需要在匹配活动variant证明下投影到enum payload子对象place时使用`place.enum_payload`；派生或重绑定typed place capability时使用；具体前置条件为“active/partially initialized variant proof”。选择S2/P9变体时，必须同时满足`UnaryResult`的arity、`SelectorSymbol`的字段顺序和该opcode的type/CFG rule。结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

PlaceProducing, SymbolUser

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2569` (`0x0A09`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-addr"></a>
## `place.addr`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.addr %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在NoEscape与lifetime约束下取得typed place对应的raw地址。类型/CFG约束：nonsealed place -> compatible raw pointer；丢失 lifecycle authority。

### 用法

当程序语义需要在NoEscape与lifetime约束下取得typed place对应的raw地址时使用`place.addr`；派生或重绑定typed place capability时使用；具体前置条件为“nonsealed place -> compatible raw pointer；丢失 lifecycle authority”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2570` (`0x0A0A`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-place-borrow"></a>
## `place.borrow`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = place.borrow %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从Alive typed place建立权限不增强且捕获当前generation的非空reference。类型/CFG约束：Alive place -> non-owning reference。

### 用法

当程序语义需要从Alive typed place建立权限不增强且捕获当前generation的非空reference时使用`place.borrow`；派生或重绑定typed place capability时使用；具体前置条件为“Alive place -> non-owning reference”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2571` (`0x0A0B`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-raw-load"></a>
## `raw.load`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = raw.load %operand0 {alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P11 `Alignment` — `Alignment : U`。Canonical text：`{alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按显式alignment从合法raw地址读取允许raw访问的标量位模式。类型/CFG约束：compatible raw pointer -> scalar T；访问范围与 active/partial SealedRuntimeStorage 不相交。

### 用法

当程序语义需要按显式alignment从合法raw地址读取允许raw访问的标量位模式时使用`raw.load`；显式采用raw byte/pointer语义且已承担其前置条件时使用；具体前置条件为“compatible raw pointer -> scalar T；访问范围与 active/partial SealedRuntimeStorage 不相交”。选择S2/P11变体时，必须同时满足`UnaryResult`的arity、`Alignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(raw)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2817` (`0x0B01`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-raw-store"></a>
## `raw.store`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`raw.store %operand0, %operand1 {alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P11 `Alignment` — `Alignment : U`。Canonical text：`{alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按显式alignment向合法raw地址写入允许raw访问的标量位模式。类型/CFG约束：scalar T + compatible raw pointer；访问范围与 active/partial SealedRuntimeStorage 不相交。

### 用法

当程序语义需要按显式alignment向合法raw地址写入允许raw访问的标量位模式时使用`raw.store`；显式采用raw byte/pointer语义且已承担其前置条件时使用；具体前置条件为“scalar T + compatible raw pointer；访问范围与 active/partial SealedRuntimeStorage 不相交”。选择S7/P11变体时，必须同时满足`BinaryEffect`的arity、`Alignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(raw)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2818` (`0x0B02`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-raw-memcpy"></a>
## `raw.memcpy`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`raw.memcpy %operand0, %operand1, %operand2 {dst_alignment = N, src_alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S8 `TernaryEffect` — `R0 O3 G0 S0 D0`。

Payload：P12 `RawTransferAlignment` — `DestinationAlignment : U, SourceAlignment : U`。Canonical text：`{dst_alignment = N, src_alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在已证明不重叠的raw byte范围间复制准确byte count。类型/CFG约束：dst, src, byte count；未授权重叠非法；两范围均与 active/partial sealed range 不相交。

### 用法

当程序语义需要在已证明不重叠的raw byte范围间复制准确byte count时使用`raw.memcpy`；显式采用raw byte/pointer语义且已承担其前置条件时使用；具体前置条件为“dst, src, byte count；未授权重叠非法；两范围均与 active/partial sealed range 不相交”。选择S8/P12变体时，必须同时满足`TernaryEffect`的arity、`RawTransferAlignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(raw), W(raw)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2819` (`0x0B03`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-raw-memmove"></a>
## `raw.memmove`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`raw.memmove %operand0, %operand1, %operand2 {dst_alignment = N, src_alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S8 `TernaryEffect` — `R0 O3 G0 S0 D0`。

Payload：P12 `RawTransferAlignment` — `DestinationAlignment : U, SourceAlignment : U`。Canonical text：`{dst_alignment = N, src_alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在允许重叠的raw byte范围间移动准确byte count。类型/CFG约束：dst, src, byte count；允许彼此重叠；两范围均与 active/partial sealed range 不相交。

### 用法

当程序语义需要在允许重叠的raw byte范围间移动准确byte count时使用`raw.memmove`；显式采用raw byte/pointer语义且已承担其前置条件时使用；具体前置条件为“dst, src, byte count；允许彼此重叠；两范围均与 active/partial sealed range 不相交”。选择S8/P12变体时，必须同时满足`TernaryEffect`的arity、`RawTransferAlignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(raw), W(raw)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2820` (`0x0B04`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-raw-memset"></a>
## `raw.memset`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`raw.memset %operand0, %operand1, %operand2 {alignment = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S8 `TernaryEffect` — `R0 O3 G0 S0 D0`。

Payload：P11 `Alignment` — `Alignment : U`。Canonical text：`{alignment = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

用单字节pattern填充合法raw byte范围。类型/CFG约束：dst, u8 byte, byte count；范围与 active/partial sealed range 不相交。

### 用法

当程序语义需要用单字节pattern填充合法raw byte范围时使用`raw.memset`；显式采用raw byte/pointer语义且已承担其前置条件时使用；具体前置条件为“dst, u8 byte, byte count；范围与 active/partial sealed range 不相交”。选择S8/P11变体时，必须同时满足`TernaryEffect`的arity、`Alignment`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(raw)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `2821` (`0x0B05`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-ptr-offset"></a>
## `ptr.offset`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = ptr.offset %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按目标sizeof(T)缩放元素偏移并计算同型raw pointer。类型/CFG约束：raw pointer + ptrsize elements -> same pointer type；T sized。

### 用法

当程序语义需要按目标sizeof(T)缩放元素偏移并计算同型raw pointer时使用`ptr.offset`；在扁平目标地址语义下执行raw pointer位移或比较时使用；具体前置条件为“raw pointer + ptrsize elements -> same pointer type；T sized”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure, TargetDependent

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `3073` (`0x0C01`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-ptr-byte-offset"></a>
## `ptr.byte_offset`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = ptr.byte_offset %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按原始byte offset计算同address-space raw pointer。类型/CFG约束：raw pointer + ptrsize bytes -> same pointer type。

### 用法

当程序语义需要按原始byte offset计算同address-space raw pointer时使用`ptr.byte_offset`；在扁平目标地址语义下执行raw pointer位移或比较时使用；具体前置条件为“raw pointer + ptrsize bytes -> same pointer type”。选择S3/P1变体时，必须同时满足`BinaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `3074` (`0x0C02`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-ptr-cmp"></a>
## `ptr.cmp`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = ptr.cmp %operand0, %operand1 {predicate = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S3 `BinaryResult` — `R1 O2 G0 S0 D0`。

Payload：P5 `IntegerPredicate` — `Enum<IntegerPredicateTag>`。Canonical text：`{predicate = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按允许的地址predicate比较两个兼容address-space raw pointer。类型/CFG约束：compatible address spaces；predicate 仅 eq/ne/ult/ule/ugt/uge。

### 用法

当程序语义需要按允许的地址predicate比较两个兼容address-space raw pointer时使用`ptr.cmp`；在扁平目标地址语义下执行raw pointer位移或比较时使用；具体前置条件为“compatible address spaces；predicate 仅 eq/ne/ult/ule/ugt/uge”。选择S3/P5变体时，必须同时满足`BinaryResult`的arity、`IntegerPredicate`的字段顺序和该opcode的type/CFG rule。仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

Speculatable

### 来源

[Schema Registry §13.4](./10-schema-registry.md#134-memplaceraw-与-ptr)，OpcodeTag `3075` (`0x0C03`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-init-begin"></a>
## `obj.init.begin`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.init.begin %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

为未初始化root或合法child storage建立fresh对象初始化事务。类型/CFG约束：owner-authority init place；建立 fresh active transaction。

### 用法

当程序语义需要为未初始化root或合法child storage建立fresh对象初始化事务时使用`obj.init.begin`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“owner-authority init place；建立 fresh active transaction”。选择S6/P1变体时，必须同时满足`UnaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

BeginLifetime

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3329` (`0x0D01`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-init"></a>
## `obj.init`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.init %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在active事务中用单标量或unit值初始化非sealed leaf对象。类型/CFG约束：owner transaction 内 nonsealed init place + scalar/unit value，顺序固定 dst,value；active aggregate parent的direct leaf原子建立Alive/fresh leaf generation并登记path/order，root scalar的generation仍由commit建立。

### 用法

当程序语义需要在active事务中用单标量或unit值初始化非sealed leaf对象时使用`obj.init`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“owner transaction 内 nonsealed init place + scalar/unit value，顺序固定 dst,value；active aggregate parent的direct leaf原子建立Alive/fresh leaf generation并登记path/order，root scalar的generation仍由commit建立”。选择S7/P1变体时，必须同时满足`BinaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed), BeginLifetime

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3330` (`0x0D02`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-init-copy"></a>
## `obj.init.copy`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.init.copy %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在active事务中按语义组成部分复制初始化Copyable address-only对象。类型/CFG约束：owner transaction 内 nonsealed init dst + readable Alive address-only Copyable src；`CopyConstructionEnabled(T)`；原子为每个descendant建立fresh generation/ActiveObjectTree并分别登记InitializedLeafMask/CommittedChildMask，外层仍待commit；source generation 匹配。

### 用法

当程序语义需要在active事务中按语义组成部分复制初始化Copyable address-only对象时使用`obj.init.copy`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“owner transaction 内 nonsealed init dst + readable Alive address-only Copyable src；`CopyConstructionEnabled(T)`；原子为每个descendant建立fresh generation/ActiveObjectTree并分别登记InitializedLeafMask/CommittedChildMask，外层仍待commit；source generation 匹配”。选择S7/P1变体时，必须同时满足`BinaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed), W(typed), BeginLifetime

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3331` (`0x0D03`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-init-commit"></a>
## `obj.init.commit`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.init.commit %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

验证完整初始化并把事务root提交为fresh Alive对象generation。类型/CFG约束：owner-authority nonsealed transaction root 完整且无 active child；唯一 commit并建立 fresh ObjectLifetimeGeneration。

### 用法

当程序语义需要验证完整初始化并把事务root提交为fresh Alive对象generation时使用`obj.init.commit`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“owner-authority nonsealed transaction root 完整且无 active child；唯一 commit并建立 fresh ObjectLifetimeGeneration”。选择S6/P1变体时，必须同时满足`UnaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed), BeginLifetime

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3332` (`0x0D04`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-assign-copy"></a>
## `obj.assign.copy`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.assign.copy %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在两个Alive address-only对象之间执行普通Copyable赋值。类型/CFG约束：nonsealed writable Alive dst + readable Alive Copyable src；`CopyAssignmentEnabled(T)`；enum只在所有payload递归assign/construct且NeedsDestroy=false时允许分支切换，原子终结旧payload generation tree并建立fresh新tree；authority/generation 均匹配。

### 用法

当程序语义需要在两个Alive address-only对象之间执行普通Copyable赋值时使用`obj.assign.copy`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“nonsealed writable Alive dst + readable Alive Copyable src；`CopyAssignmentEnabled(T)`；enum只在所有payload递归assign/construct且NeedsDestroy=false时允许分支切换，原子终结旧payload generation tree并建立fresh新tree；authority/generation 均匹配”。选择S7/P1变体时，必须同时满足`BinaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(typed), W(typed)；enum分支可不同时再加EndLifetime, BeginLifetime

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3333` (`0x0D05`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-destroy"></a>
## `obj.destroy`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.destroy %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按静态完整类型执行nothrow析构链并结束对象lifetime generation。类型/CFG约束：owner-authority `!place<ro/rw,T>` Alive；T nonsealed、静态完整；nominal class从准确NominalSemanticProperties.Destructor（若present）开始并按声明逆序执行唯field/base链，全链nothrow；按析构顺序终结全部descendant generation后终结root generation。

### 用法

当程序语义需要按静态完整类型执行nothrow析构链并结束对象lifetime generation时使用`obj.destroy`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“owner-authority `!place<ro/rw,T>` Alive；T nonsealed、静态完整；nominal class从准确NominalSemanticProperties.Destructor（若present）开始并按声明逆序执行唯field/base链，全链nothrow；按析构顺序终结全部descendant generation后终结root generation”。选择S6/P1变体时，必须同时满足`UnaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed), EndLifetime, CallSummary

### 阶段

SC

### Traits

Lifetime

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3334` (`0x0D06`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-obj-destroy-dynamic"></a>
## `obj.destroy_dynamic`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`obj.destroy_dynamic %operand0, %operand1 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按匹配vtable与版本选择完整动态析构闭包并结束对象lifetime。类型/CFG约束：owner-authority nonsealed complete object + matching stable vtable；选择当前兼容析构entry，按动态析构顺序终结全部descendant generation后终结root generation。

### 用法

当程序语义需要按匹配vtable与版本选择完整动态析构闭包并结束对象lifetime时使用`obj.destroy_dynamic`；显式推进对象初始化、赋值、销毁或rollback状态机时使用；具体前置条件为“owner-authority nonsealed complete object + matching stable vtable；选择当前兼容析构entry，按动态析构顺序终结全部descendant generation后终结root generation”。选择S7/P1变体时，必须同时满足`BinaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(typed), EndLifetime, CallSummary, RT(dynamic_destroy), RT(version_select), MayDiverge

### 阶段

SC

### Traits

Lifetime, RuntimeSemantic

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3335` (`0x0D07`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-call-direct"></a>
## `call.direct`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= %result0 = call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
CONSTRUCTOR ::= call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = initializing_receiver} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S13 `CallNoUnwind` — `R0|1 O* G0 S0 D0|1`。

Payload：P14 `Call` — `CallPayload`。Canonical text：call renderer按§15.5打印callee variant和`{function_type, entry_identity, calling_convention, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

调用由SymbolId直接确定且签名、entry identity与ABI匹配的nothrow Ink目标。类型/CFG约束：CalleeKind=direct；ink `ExecutableInkTarget`；nothrow；constructor只用concrete initializing_receiver destination；result form/role 精确匹配。

### 用法

当程序语义需要调用由SymbolId直接确定且签名、entry identity与ABI匹配的nothrow Ink目标时使用`call.direct`；调用Ink逻辑签名并保留callee/destination/异常契约时使用；具体前置条件为“CalleeKind=direct；ink `ExecutableInkTarget`；nothrow；constructor只用concrete initializing_receiver destination；result form/role 精确匹配”。选择S13/P14变体时，必须同时满足`CallNoUnwind`的arity、`Call`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary；stable 再加 RT(version_select)

### 阶段

SC

### Traits

CallLike, DestinationChannel, SymbolUser

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3585` (`0x0E01`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-call-indirect"></a>
## `call.indirect`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= call.indirect %callee([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= %result0 = call.indirect %callee([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= call.indirect %callee([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S13 `CallNoUnwind` — `R0|1 O* G0 S0 D0|1`。

Payload：P14 `Call` — `CallPayload`。Canonical text：call renderer按§15.5打印callee variant和`{function_type, entry_identity, calling_convention, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

通过准确函数指针调用签名与ABI匹配的nothrow Ink目标。类型/CFG约束：CalleeKind=indirect；function value/signature 精确且provenance只能到达`ExecutableInkTarget`；独立 nothrow proof。

### 用法

当程序语义需要通过准确函数指针调用签名与ABI匹配的nothrow Ink目标时使用`call.indirect`；调用Ink逻辑签名并保留callee/destination/异常契约时使用；具体前置条件为“CalleeKind=indirect；function value/signature 精确且provenance只能到达`ExecutableInkTarget`；独立 nothrow proof”。选择S13/P14变体时，必须同时满足`CallNoUnwind`的arity、`Call`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary

### 阶段

SC

### Traits

CallLike, DestinationChannel

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3586` (`0x0E02`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-call-virtual"></a>
## `call.virtual`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= call.virtual %receiver {slot = @symbol}([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= %result0 = call.virtual %receiver {slot = @symbol}([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= call.virtual %receiver {slot = @symbol}([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S13 `CallNoUnwind` — `R0|1 O* G0 S0 D0|1`。

Payload：P14 `Call` — `CallPayload`。Canonical text：call renderer按§15.5打印callee variant和`{function_type, entry_identity, calling_convention, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

经已验证class slot与版本表分派nothrow virtual Ink调用。类型/CFG约束：CalleeKind=virtual；receiver/slot/signature/version table 匹配；slot concrete且final target满足`ExecutableInkTarget`、nothrow。

### 用法

当程序语义需要经已验证class slot与版本表分派nothrow virtual Ink调用时使用`call.virtual`；调用Ink逻辑签名并保留callee/destination/异常契约时使用；具体前置条件为“CalleeKind=virtual；receiver/slot/signature/version table 匹配；slot concrete且final target满足`ExecutableInkTarget`、nothrow”。选择S13/P14变体时，必须同时满足`CallNoUnwind`的arity、`Call`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary, RT(virtual_dispatch)

### 阶段

SC

### Traits

CallLike, DestinationChannel, SymbolUser

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3587` (`0x0E03`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-call-interface"></a>
## `call.interface`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= call.interface %receiver {method = @symbol}([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= %result0 = call.interface %receiver {method = @symbol}([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= call.interface %receiver {method = @symbol}([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S13 `CallNoUnwind` — `R0|1 O* G0 S0 D0|1`。

Payload：P14 `Call` — `CallPayload`。Canonical text：call renderer按§15.5打印callee variant和`{function_type, entry_identity, calling_convention, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

经已验证interface method映射分派nothrow Ink调用。类型/CFG约束：CalleeKind=interface；interface place/method/signature 匹配；binding concrete且final target满足`ExecutableInkTarget`、nothrow。

### 用法

当程序语义需要经已验证interface method映射分派nothrow Ink调用时使用`call.interface`；调用Ink逻辑签名并保留callee/destination/异常契约时使用；具体前置条件为“CalleeKind=interface；interface place/method/signature 匹配；binding concrete且final target满足`ExecutableInkTarget`、nothrow”。选择S13/P14变体时，必须同时满足`CallNoUnwind`的arity、`Call`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary, RT(interface_dispatch)

### 阶段

SC

### Traits

CallLike, DestinationChannel, SymbolUser

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3588` (`0x0E04`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-call-invoke"></a>
## `call.invoke`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= call.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= call.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= call.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
CONSTRUCTOR ::= call.invoke callee_kind = direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = initializing_receiver} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S14 `CallInvoke` — `R0 O* G0 S2 D0|1`。

Payload：P14 `Call` — `CallPayload`。Canonical text：call renderer按§15.5打印callee variant和`{function_type, entry_identity, calling_convention, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

调用可能产生Ink unwind的目标并显式分流normal与unwind successor。类型/CFG约束：callee kind direct/indirect/virtual/interface/reflection；复用对应`ExecutableInkTarget`/concrete-dispatch规则；恰有 normal/unwind；scalar normal 用 result(0)，address normal 不带 result。

### 用法

当程序语义需要调用可能产生Ink unwind的目标并显式分流normal与unwind successor时使用`call.invoke`；调用Ink逻辑签名并保留callee/destination/异常契约时使用；具体前置条件为“callee kind direct/indirect/virtual/interface/reflection；复用对应`ExecutableInkTarget`/concrete-dispatch规则；恰有 normal/unwind；scalar normal 用 result(0)，address normal 不带 result”。选择S14/P14变体时，必须同时满足`CallInvoke`的arity、`Call`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary, MayUnwind, Control；kind-specific RT effect

### 阶段

SC

### Traits

Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, NoFallthrough

### 来源

[Schema Registry §13.5](./10-schema-registry.md#135-obj-与-call)，OpcodeTag `3589` (`0x0E05`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-entry"></a>
## `eh.entry`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = eh.entry %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在landing block入口物化该unwind边唯一传入的active exception token。类型/CFG约束：只在 unwind successor 首部；incoming -> active exception。

### 用法

当程序语义需要在landing block入口物化该unwind边唯一传入的active exception token时使用`eh.entry`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“只在 unwind successor 首部；incoming -> active exception”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

RT(exception), Control

### 阶段

SC

### Traits

OwnedHandle

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3841` (`0x0F01`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-match"></a>
## `eh.match`

### 格式

Renderer：`eh_match`。Canonical renderer template：

`eh.match %active [!tN -> ^bbN(%active), ..., catch_all -> ^bbC(%active)] unmatched ^bbU(%active) : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S12 `ManySuccessors` — `R0 O* G0 S* D0`。

Payload：P15 `EhMatch` — `HandlerCount : U, repeat (Enum<ExceptionHandlerKindTag>, Opt<ExceptionTypeId>, HandlerSuccessorOrdinal), UnmatchedSuccessorOrdinal`。Canonical text：eh_match renderer打印`[!tN -> EDGE, ..., catch_all -> EDGE] unmatched EDGE`，不打印dictionary/count/ordinal。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按有序typed handler与可选catch_all匹配active exception并保留unmatched传播边。类型/CFG约束：active token；handler source order，catch-all 最后，unmatched required。

### 用法

当程序语义需要按有序typed handler与可选catch_all匹配active exception并保留unmatched传播边时使用`eh.match`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“active token；handler source order，catch-all 最后，unmatched required”。选择S12/P15变体时，必须同时满足`ManySuccessors`的arity、`EhMatch`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

RT(exception_match), Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3842` (`0x0F02`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-payload"></a>
## `eh.payload`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = eh.payload %operand0 {as = !tN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P16 `EhPayload` — `AsTypeId`。Canonical text：`{as = !tN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在匹配证明下把active exception payload读取为指定类型view。类型/CFG约束：active token -> readonly nonescaping payload view。

### 用法

当程序语义需要在匹配证明下把active exception payload读取为指定类型view时使用`eh.payload`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“active token -> readonly nonescaping payload view”。选择S2/P16变体时，必须同时满足`UnaryResult`的arity、`EhPayload`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(exception)

### 阶段

SC

### Traits

—

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3843` (`0x0F03`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-end-catch"></a>
## `eh.end_catch`

### 格式

Renderer：`eh_end_catch`。Canonical renderer template：

`eh.end_catch %active -> NORMAL_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S10 `OneSuccessor` — `R0 O* G0 S1 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

结束当前catch对exception token的线性所有权并转移到normal successor。类型/CFG约束：consume当前ActiveUnwindRecord；唯一normal successor；若引用共享ExceptionBox则只release当前引用，最后一个owner才销毁payload/cause/pin。

### 用法

当程序语义需要结束当前catch对exception token的线性所有权并转移到normal successor时使用`eh.end_catch`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“consume当前ActiveUnwindRecord；唯一normal successor；若引用共享ExceptionBox则只release当前引用，最后一个owner才销毁payload/cause/pin”。选择S10/P1变体时，必须同时满足`OneSuccessor`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

ExceptionDestroySummary, RT(exception), EndLifetime, Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3844` (`0x0F04`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-throw"></a>
## `eh.throw`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`eh.throw OPERANDS {exception_type = @symbol, constructor = @symbol, argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P17 `ThrowTarget` — `ExceptionTypeSymbolId, ConstructorSymbolId, ArgumentCount : U`。Canonical text：`{exception_type = @symbol, constructor = @symbol, argument_count = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

用指定异常类型与constructor从参数构造并抛出新的exception token。类型/CFG约束：constructor args；新 exception payload/record。

### 用法

当程序语义需要用指定异常类型与constructor从参数构造并抛出新的exception token时使用`eh.throw`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“constructor args；新 exception payload/record”。选择S21/P17变体时，必须同时满足`CompletionTerminator`的arity、`ThrowTarget`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

ExceptionCreateSummary, A(exception), W(exception), BeginLifetime, RT(exception), MayUnwind, Control

### 阶段

SC

### Traits

Terminator, NoNormalReturn, NoFallthrough, SymbolUser

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3845` (`0x0F05`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-throw-copy"></a>
## `eh.throw_copy`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`eh.throw_copy OPERANDS : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

复制可抛出异常对象并把副本封装为新的exception token。类型/CFG约束：readable Alive source，其T满足ExceptionPayloadClass、Copyable且`CopyConstructionEnabled(T)`；payload按obj.init.copy的完整descendant-generation规则建立。

### 用法

当程序语义需要复制可抛出异常对象并把副本封装为新的exception token时使用`eh.throw_copy`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“readable Alive source，其T满足ExceptionPayloadClass、Copyable且`CopyConstructionEnabled(T)`；payload按obj.init.copy的完整descendant-generation规则建立”。选择S21/P1变体时，必须同时满足`CompletionTerminator`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

ExceptionCreateSummary, A(exception), R(typed), W(exception), BeginLifetime, RT(exception), MayUnwind, Control

### 阶段

SC

### Traits

Terminator, NoNormalReturn, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3846` (`0x0F06`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-throw-from"></a>
## `eh.throw_from`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`eh.throw_from OPERANDS {exception_type = @symbol, constructor = @symbol, argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P17 `ThrowTarget` — `ExceptionTypeSymbolId, ConstructorSymbolId, ArgumentCount : U`。Canonical text：`{exception_type = @symbol, constructor = @symbol, argument_count = N}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在保留cause链的同时构造并抛出新的exception token。类型/CFG约束：前ArgumentCount个constructor args + 最后active cause；成功转移cause，constructor失败则release旧token后传播新异常。

### 用法

当程序语义需要在保留cause链的同时构造并抛出新的exception token时使用`eh.throw_from`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“前ArgumentCount个constructor args + 最后active cause；成功转移cause，constructor失败则release旧token后传播新异常”。选择S21/P17变体时，必须同时满足`CompletionTerminator`的arity、`ThrowTarget`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

ExceptionCreateSummary, ExceptionDestroySummary, A(exception), W(exception), BeginLifetime, RT(exception), MayUnwind, Control

### 阶段

SC

### Traits

Terminator, NoNormalReturn, NoFallthrough, SymbolUser

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3847` (`0x0F07`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-rethrow"></a>
## `eh.rethrow`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`eh.rethrow OPERANDS : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把当前active exception token原样重新抛向外层unwind边界。类型/CFG约束：consume active token，跳过当前 handler list。

### 用法

当程序语义需要把当前active exception token原样重新抛向外层unwind边界时使用`eh.rethrow`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“consume active token，跳过当前 handler list”。选择S21/P1变体时，必须同时满足`CompletionTerminator`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

RT(exception), MayUnwind, Control

### 阶段

SC

### Traits

Terminator, NoNormalReturn, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3848` (`0x0F08`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-eh-resume"></a>
## `eh.resume`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`eh.resume OPERANDS : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从cleanup路径继续传播尚未被catch消费的exception token。类型/CFG约束：consume active token，交外层 continuation。

### 用法

当程序语义需要从cleanup路径继续传播尚未被catch消费的exception token时使用`eh.resume`；在线性exception token与显式unwind CFG上操作时使用；具体前置条件为“consume active token，交外层 continuation”。选择S21/P1变体时，必须同时满足`CompletionTerminator`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

RT(exception), MayUnwind, Control

### 阶段

SC

### Traits

Terminator, NoNormalReturn, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `3849` (`0x0F09`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-rt-trap"></a>
## `rt.trap`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`rt.trap OPERANDS {kind = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P13 `TrapKind` — `Enum<TrapKindTag>`。Canonical text：`{kind = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TrapKind触发可诊断且不返回的runtime trap语义。类型/CFG约束：立即产生trap completion；不可 catch，不运行语言 cleanup。

### 用法

当程序语义需要按TrapKind触发可诊断且不返回的runtime trap语义时使用`rt.trap`；请求已注册runtime终止、trap或其他语义处理时使用；具体前置条件为“立即产生trap completion；不可 catch，不运行语言 cleanup”。选择S21/P13变体时，必须同时满足`CompletionTerminator`的arity、`TrapKind`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

RT(trap), MayTrap, Control

### 阶段

SC

### Traits

Terminator, RuntimeSemantic, NoNormalReturn, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `4097` (`0x1001`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-rt-fatal"></a>
## `rt.fatal`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`rt.fatal [%active] {kind = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P13 `TrapKind` — `Enum<TrapKindTag>`。Canonical text：`{kind = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TrapKind触发进程级不可恢复且不返回的runtime fatal语义。类型/CFG约束：O0 或 O1 active exception；fail-fast。

### 用法

当程序语义需要按TrapKind触发进程级不可恢复且不返回的runtime fatal语义时使用`rt.fatal`；请求已注册runtime终止、trap或其他语义处理时使用；具体前置条件为“O0 或 O1 active exception；fail-fast”。选择S21/P13变体时，必须同时满足`CompletionTerminator`的arity、`TrapKind`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

RT(fatal), MayDiverge, Control

### 阶段

SC

### Traits

Terminator, RuntimeSemantic, NoNormalReturn, NoFallthrough

### 来源

[Schema Registry §13.6](./10-schema-registry.md#136-ehtrap-与-fatal)，OpcodeTag `4098` (`0x1002`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-rt-version-pin"></a>
## `rt.version.pin`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = rt.version.pin %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从当前borrowed version owner建立独立线性owned version pin。类型/CFG约束：version_owner -> linear version_pin；不选择 current version。

### 用法

当程序语义需要从当前borrowed version owner建立独立线性owned version pin时使用`rt.version.pin`；请求已注册runtime终止、trap或其他语义处理时使用；具体前置条件为“version_owner -> linear version_pin；不选择 current version”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), W(runtime_registry), RT(version_pin)

### 阶段

SC

### Traits

OwnedHandle, RuntimeSemantic

### 来源

[Schema Registry §14.5](./10-schema-registry.md#145-hot-reload-pin)，OpcodeTag `4161` (`0x1041`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-rt-version-unpin"></a>
## `rt.version.unpin`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`rt.version.unpin %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

消费并释放一条owned version pin。类型/CFG约束：consume version_pin；覆盖 uses/transfer 已结束。

### 用法

当程序语义需要消费并释放一条owned version pin时使用`rt.version.unpin`；请求已注册runtime终止、trap或其他语义处理时使用；具体前置条件为“consume version_pin；覆盖 uses/transfer 已结束”。选择S6/P1变体时，必须同时满足`UnaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), W(runtime_registry), RT(version_pin)

### 阶段

SC

### Traits

OwnedHandle, RuntimeSemantic

### 来源

[Schema Registry §14.5](./10-schema-registry.md#145-hot-reload-pin)，OpcodeTag `4162` (`0x1042`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-rt-version-transfer-pin"></a>
## `rt.version.transfer_pin`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`rt.version.transfer_pin %operand0, %operand1 {owner_kind = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S7 `BinaryEffect` — `R0 O2 G0 S0 D0`。

Payload：P31 `TransferPin` — `Enum<PinOwnerKindTag>`。Canonical text：`{owner_kind = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把owned version pin的所有权转移到指定runtime owner种类。类型/CFG约束：consume pin并把唯一 release responsibility 交给匹配 Alive owner。

### 用法

当程序语义需要把owned version pin的所有权转移到指定runtime owner种类时使用`rt.version.transfer_pin`；请求已注册runtime终止、trap或其他语义处理时使用；具体前置条件为“consume pin并把唯一 release responsibility 交给匹配 Alive owner”。选择S7/P31变体时，必须同时满足`BinaryEffect`的arity、`TransferPin`的字段顺序和该opcode的type/CFG rule。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), W(runtime_registry), RT(version_pin)

### 阶段

SC

### Traits

OwnedHandle, RuntimeSemantic

### 来源

[Schema Registry §14.5](./10-schema-registry.md#145-hot-reload-pin)，OpcodeTag `4163` (`0x1043`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-rt-version-current-owner"></a>
## `rt.version.current_owner`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = rt.version.current_owner : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S1 `NullaryResult` — `R1 O0 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

物化当前activation已有version owner的不可逃逸borrowed view。类型/CFG约束：仅 version_local 或 inherited-pin compiler helper entry；物化当前 activation 的 borrowed version_owner，不选择/新建 pin。

### 用法

当程序语义需要物化当前activation已有version owner的不可逃逸borrowed view时使用`rt.version.current_owner`；请求已注册runtime终止、trap或其他语义处理时使用；具体前置条件为“仅 version_local 或 inherited-pin compiler helper entry；物化当前 activation 的 borrowed version_owner，不选择/新建 pin”。选择S1/P1变体时，必须同时满足`NullaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。借用handle不得逃逸其owner/version/generation覆盖范围。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Pure

### 阶段

SC

### Traits

BorrowedHandle, RuntimeSemantic

### 来源

[Schema Registry §14.5](./10-schema-registry.md#145-hot-reload-pin)，OpcodeTag `4164` (`0x1044`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-call"></a>
## `async.call`

### 格式

Renderer：`call`。Canonical renderer template：

`async.call callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S15 `DestinationEffect` — `R0 O* G0 S0 D1`。

Payload：P26 `AsyncCall` — `CallPayload`，destination required 且 role=result。Canonical text：call renderer按§15.5打印`callee_kind`、callee variant和CallPayload dictionary，再打印required result destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

以同步nothrow construction构造sealed Task destination而不执行其异步body。类型/CFG约束：async callee；T=void/never或closed runtime-representable Copyable且CopyConstructionEnabled、无NoEscape；`construction_nothrow=true`；required Task<T> destination role=result；schema原子执行唯一Task-specific sealed commit。

### 用法

当程序语义需要以同步nothrow construction构造sealed Task destination而不执行其异步body时使用`async.call`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“async callee；T=void/never或closed runtime-representable Copyable且CopyConstructionEnabled、无NoEscape；`construction_nothrow=true`；required Task<T> destination role=result；schema原子执行唯一Task-specific sealed commit”。选择S15/P26变体时，必须同时满足`DestinationEffect`的arity、`AsyncCall`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

AsyncConstructionSummary, A(task_frame), W(typed), BeginLifetime, RT(task_construct)；stable 再加 RT(version_select)

### 阶段

SC

### Traits

CallLike, DestinationChannel, RuntimeSemantic

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4353` (`0x1101`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-invoke"></a>
## `async.invoke`

### 格式

Renderer：`call`。Canonical renderer template：

`async.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S16 `DestinationInvoke` — `R0 O* G0 S2 D1`。

Payload：P26 `AsyncCall` — `CallPayload`，destination required 且 role=result。Canonical text：call renderer按§15.5打印`callee_kind`、callee variant和CallPayload dictionary，再打印required result destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

以normal或unwind结果构造sealed Task destination并显式处理同步construction failure。类型/CFG约束：async callee；T约束同 async.call；Task<T> destination；normal原子执行Task-specific sealed commit，unwind原子rollback；只表示同步construction completion/failure。

### 用法

当程序语义需要以normal或unwind结果构造sealed Task destination并显式处理同步construction failure时使用`async.invoke`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“async callee；T约束同 async.call；Task<T> destination；normal原子执行Task-specific sealed commit，unwind原子rollback；只表示同步construction completion/failure”。选择S16/P26变体时，必须同时满足`DestinationInvoke`的arity、`AsyncCall`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

AsyncConstructionSummary, A(task_frame), W(typed), BeginLifetime, RT(task_construct), MayUnwind, Control；stable 再加 RT(version_select)

### 阶段

SC

### Traits

Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4354` (`0x1102`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-await"></a>
## `async.await`

### 格式

Renderer：`await`。Canonical renderer template：

`async.await %task normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S18 `AwaitBranch` — `R0 O1 G0 S2 D0`，normal 可产生 result(0)。

Payload：P27 `Await` — `AwaitedLogicalTypeId`。Canonical text：不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

驱动并等待Task完成，在normal边产生可按值结果或在unwind边产生exception。类型/CFG约束：rw Task<T> place；created状态会drive；normal对unit/scalar产生result(0)，void/never无result；never normal successor不可达；unwind exception。

### 用法

当程序语义需要驱动并等待Task完成，在normal边产生可按值结果或在unwind边产生exception时使用`async.await`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“rw Task<T> place；created状态会drive；normal对unit/scalar产生result(0)，void/never无result；never normal successor不可达；unwind exception”。选择S18/P27变体时，必须同时满足`AwaitBranch`的arity、`Await`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。跨暂停存活的place、generation、cleanup、pin和Task状态必须frame-resident并在resume/destroy两路闭合。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TaskBodyEffect, R(task_state), W(task_state), A(exception), RT(async_suspend), MayUnwind, MayDiverge, Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, MaySuspend, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4355` (`0x1103`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-await-copy"></a>
## `async.await_copy`

### 格式

Renderer：`await`。Canonical renderer template：

`async.await_copy %task to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S19 `AwaitDestination` — `R0 O1 G0 S2 D1`。

Payload：P27 `Await` — `AwaitedLogicalTypeId`。Canonical text：不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

驱动并等待Task完成，把address-only Copyable结果写入required destination。类型/CFG约束：rw Task<T> + required T destination；created状态会drive；T address-only、Copyable且CopyConstructionEnabled，succeeded时按obj.init.copy的完整descendant-generation规则构造并原子commit；normal不产生edge value。

### 用法

当程序语义需要驱动并等待Task完成，把address-only Copyable结果写入required destination时使用`async.await_copy`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“rw Task<T> + required T destination；created状态会drive；T address-only、Copyable且CopyConstructionEnabled，succeeded时按obj.init.copy的完整descendant-generation规则构造并原子commit；normal不产生edge value”。选择S19/P27变体时，必须同时满足`AwaitDestination`的arity、`Await`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。跨暂停存活的place、generation、cleanup、pin和Task状态必须frame-resident并在resume/destroy两路闭合。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TaskBodyEffect, R(task_state), R(task_result), W(task_state), W(typed), BeginLifetime, A(exception), RT(async_suspend), MayUnwind, MayDiverge, Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, DestinationChannel, MaySuspend, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4356` (`0x1104`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-task-drive-once"></a>
## `async.task.drive_once`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`async.task.drive_once %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P27 `Await` — `AwaitedLogicalTypeId`。Canonical text：不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

把created或suspended Task至多推进一个合作式执行步骤。类型/CFG约束：compiler/runtime generated；created -> pending，执行到 suspend/final。

### 用法

当程序语义需要把created或suspended Task至多推进一个合作式执行步骤时使用`async.task.drive_once`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“compiler/runtime generated；created -> pending，执行到 suspend/final”。选择S6/P27变体时，必须同时满足`UnaryEffect`的arity、`Await`的字段顺序和该opcode的type/CFG rule。跨暂停存活的place、generation、cleanup、pin和Task状态必须frame-resident并在resume/destroy两路闭合。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TaskBodyEffect, R(task_state), W(task_state), RT(task_drive), MayTrap, MayDiverge

### 阶段

SC

### Traits

RuntimeSemantic, MaySuspend

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4357` (`0x1105`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-task-publish-success"></a>
## `async.task.publish_success`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`async.task.publish_success OPERANDS {result_mode = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P28 `PublishResult` — `Enum<ResultPassingModeTag>, LogicalResultTypeId`。Canonical text：`{result_mode = name}`；logical result type从task/value operand与channel signature唯一重建并核对。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在线性Task状态机中发布已完成的void、scalar或已提交address-only结果。类型/CFG约束：void O1；value/object O2；result mode/type 精确；pending -> succeeded；object operand 是 Task 内部已 Alive ResultStorage。

### 用法

当程序语义需要在线性Task状态机中发布已完成的void、scalar或已提交address-only结果时使用`async.task.publish_success`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“void O1；value/object O2；result mode/type 精确；pending -> succeeded；object operand 是 Task 内部已 Alive ResultStorage”。选择S21/P28变体时，必须同时满足`CompletionTerminator`的arity、`PublishResult`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(task_state), W(task_state), RT(task_publish), Control；nonunit value 再加 W(task_result)

### 阶段

SC

### Traits

Terminator, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4358` (`0x1106`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-task-publish-failure"></a>
## `async.task.publish_failure`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`async.task.publish_failure OPERANDS : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在线性Task状态机中发布owned exception作为失败结果。类型/CFG约束：Task place + active exception；consume token，pending -> failed。

### 用法

当程序语义需要在线性Task状态机中发布owned exception作为失败结果时使用`async.task.publish_failure`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“Task place + active exception；consume token，pending -> failed”。选择S21/P1变体时，必须同时满足`CompletionTerminator`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(task_state), W(task_state), A(exception_box), RT(task_publish), Control

### 阶段

SC

### Traits

Terminator, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4359` (`0x1107`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-task-destroy"></a>
## `async.task.destroy`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`async.task.destroy %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P27 `Await` — `AwaitedLogicalTypeId`。Canonical text：不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按TaskDestroySummary关闭并销毁sealed Task frame与其剩余状态。类型/CFG约束：owner-authority Task<T> place；非 pending 按TaskDestroySummary清理并使storage uninitialized；pending走schema-defined fatal且不完成。

### 用法

当程序语义需要按TaskDestroySummary关闭并销毁sealed Task frame与其剩余状态时使用`async.task.destroy`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“owner-authority Task<T> place；非 pending 按TaskDestroySummary清理并使storage uninitialized；pending走schema-defined fatal且不完成”。选择S6/P27变体时，必须同时满足`UnaryEffect`的arity、`Await`的字段顺序和该opcode的type/CFG rule。使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TaskDestroySummary, R(task_state), W(task_state), W(typed), EndLifetime, D(task_frame), RT(task_destroy), RT(fatal), MayDiverge

### 阶段

SC

### Traits

Lifetime, RuntimeSemantic

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4360` (`0x1108`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-cancel-request"></a>
## `async.cancel.request`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`async.cancel.request %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P27 `Await` — `AwaitedLogicalTypeId`。Canonical text：不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

原子设置Task的cooperative cancellation request状态。类型/CFG约束：rw Task<T>；nothrow、幂等、线程安全 request publish。

### 用法

当程序语义需要原子设置Task的cooperative cancellation request状态时使用`async.cancel.request`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“rw Task<T>；nothrow、幂等、线程安全 request publish”。选择S6/P27变体时，必须同时满足`UnaryEffect`的arity、`Await`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

W(task_state), RT(cancel_request)

### 阶段

SC

### Traits

RuntimeSemantic

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4361` (`0x1109`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-cancel-is-requested"></a>
## `async.cancel.is_requested`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = async.cancel.is_requested %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P27 `Await` — `AwaitedLogicalTypeId`。Canonical text：不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

读取Task当前是否已收到cooperative cancellation request。类型/CFG约束：ro/rw Task<T> -> bool；不 drive Task。

### 用法

当程序语义需要读取Task当前是否已收到cooperative cancellation request时使用`async.cancel.is_requested`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“ro/rw Task<T> -> bool；不 drive Task”。选择S2/P27变体时，必须同时满足`UnaryResult`的arity、`Await`的字段顺序和该opcode的type/CFG rule。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(task_state), RT(cancel_query)

### 阶段

SC

### Traits

RuntimeSemantic

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4362` (`0x110A`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-async-continuation-invoke"></a>
## `async.continuation_invoke`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= async.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= async.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= async.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S14 `CallInvoke` — `R0 O* G0 S2 D0|1`。

Payload：P30 `Continuation` — `NextLayer : U, FunctionTypeId, Opt<DestinationPayload>`。Canonical text：`{next_layer = N, function_type = !tN}`后按present打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在同一Task与frame内调用下一层async decorator continuation并分流normal/unwind。类型/CFG约束：staged async decorator 下一层；同一 Task/frame；normal/unwind；optional destination原样转发；不可逃逸或并发。

### 用法

当程序语义需要在同一Task与frame内调用下一层async decorator continuation并分流normal/unwind时使用`async.continuation_invoke`；构造、驱动、等待、发布或销毁sealed Task状态时使用；具体前置条件为“staged async decorator 下一层；同一 Task/frame；normal/unwind；optional destination原样转发；不可逃逸或并发”。选择S14/P30变体时，必须同时满足`CallInvoke`的arity、`Continuation`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。跨暂停存活的place、generation、cleanup、pin和Task状态必须frame-resident并在resume/destroy两路闭合。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

TaskBodyEffect, RT(async_suspend), MayDiverge, Control；effective next-layer BehaviorContract允许时才加MayUnwind

### 阶段

S

### Traits

Terminator, InvokeLike, EdgeProducing, DestinationChannel, MaySuspend, NoFallthrough

### 来源

[Schema Registry §14.3](./10-schema-registry.md#143-asynctask-与-cancellation)，OpcodeTag `4363` (`0x110B`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-lookup-type"></a>
## `reflect.lookup_type`

### 格式

Renderer：`lookup`。Canonical renderer template：

`reflect.lookup_type %qualified_name {module = "canonical.module"} found FOUND_EDGE missing MISSING_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S17 `LookupBranch` — `R0 O* G0 S2 D0`，found edge 产生 result(0)。

Payload：P22 `ReflectLookup` — `CanonicalModuleIdentity : Str`。Canonical text：`{module = "canonical.module"}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按模块与字符串名称查询并在found边产生owned type snapshot。类型/CFG约束：String place；found/missing；found 产生 owned type_snapshot。

### 用法

当程序语义需要按模块与字符串名称查询并在found边产生owned type snapshot时使用`reflect.lookup_type`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“String place；found/missing；found 产生 owned type_snapshot”。选择S17/P22变体时，必须同时满足`LookupBranch`的arity、`ReflectLookup`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), RT(reflection_lookup), RT(version_select), Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4609` (`0x1201`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-lookup-interface"></a>
## `reflect.lookup_interface`

### 格式

Renderer：`lookup`。Canonical renderer template：

`reflect.lookup_interface %qualified_name {module = "canonical.module"} found FOUND_EDGE missing MISSING_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S17 `LookupBranch` — `R0 O* G0 S2 D0`，found edge 产生 result(0)。

Payload：P22 `ReflectLookup` — `CanonicalModuleIdentity : Str`。Canonical text：`{module = "canonical.module"}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按模块与字符串名称查询并在found边产生owned interface snapshot。类型/CFG约束：String place；found 产生 owned interface_snapshot。

### 用法

当程序语义需要按模块与字符串名称查询并在found边产生owned interface snapshot时使用`reflect.lookup_interface`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“String place；found 产生 owned interface_snapshot”。选择S17/P22变体时，必须同时满足`LookupBranch`的arity、`ReflectLookup`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), RT(reflection_lookup), RT(version_select), Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4610` (`0x1202`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-lookup-function"></a>
## `reflect.lookup_function`

### 格式

Renderer：`lookup`。Canonical renderer template：

`reflect.lookup_function %qualified_name {module = "canonical.module", expected_function_type = !tN} found FOUND_EDGE missing MISSING_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S17 `LookupBranch` — `R0 O* G0 S2 D0`，found edge 产生 result(0)。

Payload：P23 `ReflectLookupFunction` — `CanonicalModuleIdentity : Str, ExpectedFunctionTypeId`。Canonical text：`{module = "canonical.module", expected_function_type = !tN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

按模块、字符串名称与准确函数类型查询owned function snapshot。类型/CFG约束：String place + exact expected signature；found 产生 owned function_snapshot。

### 用法

当程序语义需要按模块、字符串名称与准确函数类型查询owned function snapshot时使用`reflect.lookup_function`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“String place + exact expected signature；found 产生 owned function_snapshot”。选择S17/P23变体时，必须同时满足`LookupBranch`的arity、`ReflectLookupFunction`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), RT(reflection_lookup), RT(version_select), Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4611` (`0x1203`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-lookup-member"></a>
## `reflect.lookup_member`

### 格式

Renderer：`lookup`。Canonical renderer template：

`reflect.lookup_member %owner, %name {member_kind = name} found FOUND_EDGE missing MISSING_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S17 `LookupBranch` — `R0 O* G0 S2 D0`，found edge 产生 result(0)。

Payload：P24 `ReflectLookupMember` — `Enum<ReflectionMemberKindTag>`。Canonical text：`{member_kind = name}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在owner snapshot中按member kind与名称查询owner-bounded borrowed member view。类型/CFG约束：owner snapshot + String place；found 产生 owner-bounded borrowed member_view。

### 用法

当程序语义需要在owner snapshot中按member kind与名称查询owner-bounded borrowed member view时使用`reflect.lookup_member`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“owner snapshot + String place；found 产生 owner-bounded borrowed member_view”。选择S17/P24变体时，必须同时满足`LookupBranch`的arity、`ReflectLookupMember`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。借用handle不得逃逸其owner/version/generation覆盖范围。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(reflection_snapshot), RT(reflection_lookup), Control

### 阶段

SC

### Traits

Terminator, EdgeProducing, BorrowedHandle, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4612` (`0x1204`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-snapshot-clone"></a>
## `reflect.snapshot.clone`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`%result0 = reflect.snapshot.clone %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S2 `UnaryResult` — `R1 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从现有owned reflection snapshot建立同版本的独立owned clone。类型/CFG约束：owned K_snapshot -> 第二个相同 kind linear owner。

### 用法

当程序语义需要从现有owned reflection snapshot建立同版本的独立owned clone时使用`reflect.snapshot.clone`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“owned K_snapshot -> 第二个相同 kind linear owner”。选择S2/P1变体时，必须同时满足`UnaryResult`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), W(runtime_registry), RT(version_pin)

### 阶段

SC

### Traits

OwnedHandle, RuntimeSemantic

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4613` (`0x1205`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-snapshot-release"></a>
## `reflect.snapshot.release`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`reflect.snapshot.release %operand0 : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P1 `Empty` — 空。Canonical text：不打印payload。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

消费并释放一条owned reflection snapshot handle。类型/CFG约束：consume snapshot owner；全部 borrowed view 已结束。

### 用法

当程序语义需要消费并释放一条owned reflection snapshot handle时使用`reflect.snapshot.release`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“consume snapshot owner；全部 borrowed view 已结束”。选择S6/P1变体时，必须同时满足`UnaryEffect`的arity、`Empty`的字段顺序和该opcode的type/CFG rule。产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

R(runtime_registry), W(runtime_registry), RT(version_pin)

### 阶段

SC

### Traits

OwnedHandle, RuntimeSemantic

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4614` (`0x1206`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-reflect-call"></a>
## `reflect.call`

### 格式

Renderer：`reflect_call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= reflect.call %snapshot receiver(none|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= %result0 = reflect.call %snapshot receiver(none|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= reflect.call %snapshot receiver(none|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S13 `CallNoUnwind` — `R0|1 O* G0 S0 D0|1`。

Payload：P25 `ReflectCall` — `ExpectedFunctionTypeId, ReceiverPresent : Bool, ArgumentCount : U, AdapterNothrow : Bool, Opt<DestinationPayload>`。Canonical text：reflect_call renderer打印`%snapshot receiver(none/%vN) args([%vN, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = BOOL}`，方括号内argument list整体可空，再按present打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

通过pinned function snapshot与已验证adapter执行nothrow reflection调用。类型/CFG约束：sync pinned snapshot；target nothrow且AdapterNothrow由verifier从exact signature、typed place/generation和pinned layout证明；void/scalar/destination form精确。

### 用法

当程序语义需要通过pinned function snapshot与已验证adapter执行nothrow reflection调用时使用`reflect.call`；通过受版本固定的runtime reflection schema查询或调用时使用；具体前置条件为“sync pinned snapshot；target nothrow且AdapterNothrow由verifier从exact signature、typed place/generation和pinned layout证明；void/scalar/destination form精确”。选择S13/P25变体时，必须同时满足`CallNoUnwind`的arity、`ReflectCall`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary, RT(reflection_dispatch)；destination 再加 W(typed), BeginLifetime

### 阶段

SC

### Traits

CallLike, DestinationChannel, RuntimeSemantic

### 来源

[Schema Registry §14.2](./10-schema-registry.md#142-reflection)，OpcodeTag `4615` (`0x1207`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-decorator-region"></a>
## `decorator.region`

### 格式

Renderer：`region`。Canonical renderer template：

```text
decorator.region OPERANDS {decorator_kind = name, layer = N, function_type = !tN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN) region(decorator_body, origin(#oN)) {
  BLOCKS
}
```

结构约束：S20 `SingleRegion` — `R0 O* G1 S0 D0`。

Payload：P29 `DecoratorRegion` — `Enum<DecoratorKindTag>, Layer : U, FunctionTypeId`。Canonical text：`{decorator_kind = name, layer = N, function_type = !tN}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

承载一个staged decorator body region及其准确入口、出口与层级签名。类型/CFG约束：single-entry nested decorator_body；entry/exit signature 精确；continuation capability非 SSA。

### 用法

当程序语义需要承载一个staged decorator body region及其准确入口、出口与层级签名时使用`decorator.region`；执行staged decorator region或其continuation时使用；具体前置条件为“single-entry nested decorator_body；entry/exit signature 精确；continuation capability非 SSA”。选择S20/P29变体时，必须同时满足`SingleRegion`的arity、`DecoratorRegion`的字段顺序和该opcode的type/CFG rule。nested region的role、entry/exit signature与terminator必须匹配owner schema，不能把region改写成自由attribute。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

nested region effect upper bound, Control

### 阶段

S

### Traits

HasRegions

### 来源

[Schema Registry §14.4](./10-schema-registry.md#144-decorator-continuation)，OpcodeTag `4865` (`0x1301`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-decorator-continuation-invoke"></a>
## `decorator.continuation_invoke`

### 格式

Renderer：`call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
FORWARDED_CONSTRUCTOR ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) to %destination {destination_role = initializing_receiver} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S14 `CallInvoke` — `R0 O* G0 S2 D0|1`。

Payload：P30 `Continuation` — `NextLayer : U, FunctionTypeId, Opt<DestinationPayload>`。Canonical text：`{next_layer = N, function_type = !tN}`后按present打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

激活下一层sync decorator continuation并分流normal与unwind。类型/CFG约束：next sync layer fresh activation；normal/unwind；optional destination 原样转发。

### 用法

当程序语义需要激活下一层sync decorator continuation并分流normal与unwind时使用`decorator.continuation_invoke`；执行staged decorator region或其continuation时使用；具体前置条件为“next sync layer fresh activation；normal/unwind；optional destination 原样转发”。选择S14/P30变体时，必须同时满足`CallInvoke`的arity、`Continuation`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

CallSummary, Control；effective next-layer BehaviorContract 允许时才加 MayUnwind

### 阶段

S

### Traits

Terminator, InvokeLike, EdgeProducing, DestinationChannel, NoFallthrough

### 来源

[Schema Registry §14.4](./10-schema-registry.md#144-decorator-continuation)，OpcodeTag `4866` (`0x1302`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-decorator-continuation-yield"></a>
## `decorator.continuation_yield`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`decorator.continuation_yield [%value] {result_mode = name} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S21 `CompletionTerminator` — `R0 O* G0 S0 D0`，NoFallthrough。

Payload：P33 `ResultMode` — `Enum<ResultPassingModeTag>, LogicalResultTypeId`。Canonical text：`{result_mode = name}`；logical result type从包围decorator signature唯一重建并核对。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

从decorator body向包围应用返回指定result passing mode的值或空通道。类型/CFG约束：value/result_destination/void 与包围 signature 匹配；address form只传播 committed proof。

### 用法

当程序语义需要从decorator body向包围应用返回指定result passing mode的值或空通道时使用`decorator.continuation_yield`；执行staged decorator region或其continuation时使用；具体前置条件为“value/result_destination/void 与包围 signature 匹配；address form只传播 committed proof”。选择S21/P33变体时，必须同时满足`CompletionTerminator`的arity、`ResultMode`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

Control

### 阶段

S

### Traits

Terminator, NoFallthrough

### 来源

[Schema Registry §14.4](./10-schema-registry.md#144-decorator-continuation)，OpcodeTag `4867` (`0x1303`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-abi-call"></a>
## `abi.call`

### 格式

Renderer：`abi_call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= abi.call C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= %result0 = abi.call C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256"HEX64", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= abi.call C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S13 `CallNoUnwind` — `R0|1 O* G0 S0 D0|1`。

Payload：P32 `AbiCall` — `AbiCallPayload`。Canonical text：abi_call renderer按§15.5打印C callee variant和`{c_function_type, bridge, expected_ink_function_type, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

经已注册C ABI bridge调用已证明不会产生Ink unwind的foreign目标。类型/CFG约束：direct/indirect C target + registered bridge；仅已证明不产生 Ink unwind；result form 精确。

### 用法

当程序语义需要经已注册C ABI bridge调用已证明不会产生Ink unwind的foreign目标时使用`abi.call`；穿过已注册C ABI bridge且保留外部effect上界时使用；具体前置条件为“direct/indirect C target + registered bridge；仅已证明不产生 Ink unwind；result form 精确”。选择S13/P32变体时，必须同时满足`CallNoUnwind`的arity、`AbiCall`的字段顺序和该opcode的type/CFG rule。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

ExternalEffectSummary, RT(ffi)

### 阶段

SC

### Traits

CallLike, DestinationChannel, RuntimeSemantic

### 来源

[Schema Registry §14.6](./10-schema-registry.md#146-external-abi)，OpcodeTag `5121` (`0x1401`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-abi-invoke"></a>
## `abi.invoke`

### 格式

Renderer：`abi_call`。Canonical renderer template：

```text
VOID_OR_NEVER ::= abi.invoke C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256"HEX64", explicit_argument_count = N} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
UNIT_OR_SCALAR ::= abi.invoke C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256"HEX64", explicit_argument_count = N} normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
ADDRESS_RESULT ::= abi.invoke C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256"HEX64", explicit_argument_count = N} to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)
```

结构约束：S14 `CallInvoke` — `R0 O* G0 S2 D0|1`。

Payload：P32 `AbiCall` — `AbiCallPayload`。Canonical text：abi_call renderer按§15.5打印C callee variant和`{c_function_type, bridge, expected_ink_function_type, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

经已注册failure bridge调用foreign目标并把失败规范化到Ink unwind边。类型/CFG约束：registered failure bridge；恰有 normal/unwind；raw foreign exception不得直接成为 token。

### 用法

当程序语义需要经已注册failure bridge调用foreign目标并把失败规范化到Ink unwind边时使用`abi.invoke`；穿过已注册C ABI bridge且保留外部effect上界时使用；具体前置条件为“registered failure bridge；恰有 normal/unwind；raw foreign exception不得直接成为 token”。选择S14/P32变体时，必须同时满足`CallInvoke`的arity、`AbiCall`的字段顺序和该opcode的type/CFG rule。它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码。destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

ExternalEffectSummary, RT(ffi), MayUnwind, Control

### 阶段

SC

### Traits

Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, RuntimeSemantic, NoFallthrough

### 来源

[Schema Registry §14.6](./10-schema-registry.md#146-external-abi)，OpcodeTag `5122` (`0x1402`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-ct-register-module-item"></a>
## `ct.register_module_item`

### 格式

Renderer：`generic_operation`。Canonical renderer template：

`ct.register_module_item %operand0 {value_form = VALUE_FORM, source_backed_callsite = {producer_symbol = @symbol, source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], callee_identity = core_opcode(ct.register_module_item), source_backed_callsite_key = sha256"HEX64"}} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)`

结构约束：S6 `UnaryEffect` — `R0 O1 G0 S0 D0`。

Payload：P34 `ModuleRegistration` — `Enum<RegistrationValueFormTag>, SourceBackedCallsite : SourceBackedCallsiteIdentityPayload`。Canonical text：`{value_form = name, source_backed_callsite = {完整结构化字段...}}`。

大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。

### 作用

在active decorator application中有序发射一个typed module registration记录。类型/CFG约束：value form operand 为 scalar/unit T；object form为 `!place<ro,T>`；两者必须生成满足 StaticRegistrationEncodable(T, Constant) 的 frozen constant。

### 用法

当程序语义需要在active decorator application中有序发射一个typed module registration记录时使用`ct.register_module_item`；在ComptimeWorld中产生有序语义输出时使用；具体前置条件为“value form operand 为 scalar/unit T；object form为 `!place<ro,T>`；两者必须生成满足 StaticRegistrationEncodable(T, Constant) 的 frozen constant”。选择S6/P34变体时，必须同时满足`UnaryEffect`的arity、`ModuleRegistration`的字段顺序和该opcode的type/CFG rule。执行必须保留application内源码顺序，不可删除、复制、CSE或越过其他semantic emit。result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造。

### 效果

DeclarationSink(module_registration)；object form 再加 R(typed)

### 阶段

S

### Traits

OrderedSemanticEmit

### 来源

[Schema Registry §14.7](./10-schema-registry.md#147-comptime-module-registration)，OpcodeTag `5377` (`0x1501`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。

<a id="instruction-stage-force-value"></a>
## `stage.force_value`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.force_value, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

完整elaborate并执行template，要求恰1个Known typed result，canonicalize后原子提交

### 用法

Inputs：0..*个显式eager operand，类型/顺序由value template入口schema决定；template captures不重复放入Inputs。TemplateRefs：恰1个`RegionKind=value` template。Outputs：`ResultTypes`恰1项，等于Known result、Value sink与所有PlanResult use的ExpectedType。Sink：`value`。comptime expression、泛型实参及attribute/decorator实参等强制编译期值位置。

### 效果

选中template的实际ComptimeWorld effects；Residual依赖、trap、未处理异常、缺capability或预算耗尽均失败

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `1`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.2](./05-staging-comptime.md#52-stageforce_value)。

<a id="instruction-stage-force-block"></a>
## `stage.force_block`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.force_block, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

在ComptimeWorld完整执行实际路径；不得产生residual CFG，control target不得逃出template/内部activation

### 用法

Inputs：0..*个显式eager operand；template captures延迟到activation解析。TemplateRefs：恰1个statement或declaration-region template。Outputs：`ResultTypes`空。Sink：与template RegionKind匹配的`statement`、`top_level_declaration`或member sink。StatementRegion的`comptime { ... }`，或declaration region中无条件提交静态声明的comptime block。

### 效果

选中block实际effects；comptime地址、host handle、临时引用和跨template control transfer不得逃逸

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `2`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.3](./05-staging-comptime.md#53-stageforce_block)。

<a id="instruction-stage-select-if"></a>
## `stage.select_if`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.select_if, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

条件准确求值一次，只elaborate选中source-backed template；缺else只能用于非value sink并产生空输出

### 用法

Inputs：恰1个表示condition的eager PlanInput，ExpectedType为bool；若为PlanResult，只有该direct producer进入dependency vector，transitive依赖由producer自身闭合；branch captures不在Inputs。TemplateRefs：then template必有，else template可选，按源码顺序。Outputs：value sink时恰1项且then/else结果类型相同；其他sink为空。Sink：与then/else共同RegionKind匹配的sink。结构化comptime if；选中statement body可按world residualize，declaration body提交声明。

### 效果

选择控制及选中template实际effects；未选分支无绑定、类型检查、依赖执行或effect

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `3`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.4](./05-staging-comptime.md#54-stageselect_if)。

<a id="instruction-stage-select-match"></a>
## `stage.select_match`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.select_match, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

按源码arm顺序选择首个Known匹配；pattern/guard/bindings必须编译期确定

### 用法

Inputs：恰1个表示selector的eager PlanInput；若为PlanResult只记录direct producer；arm captures不在Inputs。TemplateRefs：每个源码arm恰1个template，非空并按SourceOrdinal严格递增。Outputs：value sink时恰1项且所有arm结果类型相同；其他sink为空。Sink：与所有arm RegionKind匹配的sink。结构化comptime match，保持穷尽性和重复pattern的独立语义验证。

### 效果

选择控制及选中arm实际effects；未选arm无绑定、类型检查、依赖执行或effect

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `4`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.5](./05-staging-comptime.md#55-stageselect_match)。

<a id="instruction-stage-expand-for"></a>
## `stage.expand_for`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.expand_for, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

按iterable语义顺序逐项以准确元素类型重新elaborate，每轮建立fresh binding与IterationIdentity

### 用法

Inputs：恰1个表示finite、Known、deterministic iterable的eager PlanInput；若为PlanResult只记录direct producer；body captures不在Inputs。TemplateRefs：恰1个body template。Outputs：`ResultTypes`空。Sink：与body RegionKind匹配的statement/declaration sink。异构tuple、comptime sequence和其他编译期可枚举源的结构化展开。

### 效果

按迭代顺序累积body实际effects与pending outputs；受每轮及总fuel/生成量预算

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `5`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.6](./05-staging-comptime.md#56-stageexpand_for)。

<a id="instruction-stage-expand-while"></a>
## `stage.expand_while`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.expand_while, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

每轮重新执行condition并要求Known bool；true时elaborate一轮body，false终止

### 用法

Inputs：`Inputs`空；condition/body的外层环境只通过各自Template.Captures解析，每轮condition结果不得缓存为PlanInput。TemplateRefs：恰2个template，顺序为condition、body。Outputs：`ResultTypes`空。Sink：与body RegionKind匹配的statement/declaration sink。需要ComptimeWorld状态推进、无法预先枚举的结构化comptime while。

### 效果

按轮次累积condition/body实际effects；受循环、fuel、memory、effect与输出预算

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `6`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.7](./05-staging-comptime.md#57-stageexpand_while)。

<a id="instruction-stage-instantiate"></a>
## `stage.instantiate`

### 格式

Renderer：`generic_record`。Canonical renderer template：`#pN = plan_node {stage_opcode = stage.instantiate, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = {parent_canonical_work_key = none|some(sha256"HEX64"), enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application = none|some([{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = {owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256"HEX64", source_declaration_key = hex"HEX_BYTES", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256"HEX64"}, enclosing_instance = none|some({generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256"HEX64", instance_identity_digest = sha256"HEX64"}), decorator_application_key = sha256"HEX64"}, ...]), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256"HEX64"}, required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256"HEX64", origin = #oN}`

### 作用

形成并重算InstanceIdentity；相同identity合流；不做推导、偏应用、SFINAE或dependency/generated-open-generic template实例化

### 用法

Inputs：第一个semantic input为当前module唯一source-backed、GenericDeclarationProvenance present的开放GenericDecl handle，随后为全部规范化显式/default/pack实参。TemplateRefs：同module selected generic declaration注册的完整template vector，按TemplateRolePath顺序。Outputs：`ResultTypes`恰1项，为闭合instance handle/type的准确meta type并匹配全部PlanResult use。Sink：与实例请求位置匹配的value或declaration sink。当前module source-backed显式泛型实例化与已唯一选中候选的闭合实例请求；dependency只可导入provider预生成closed instance。

### 效果

实例work item实际effects由被elaborate typed Core决定；只在完整验证后原子发布

### 阶段

Plan（`ElaborationPlan`；不属于 Core operation）。

### Traits

—（Plan node；Core `OpcodeTraits` 不适用）。

### 来源

[Schema Registry §10.3](./10-schema-registry.md#103-plan-enum-与-record)，StageOpcodeTag `7`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §5.8](./05-staging-comptime.md#58-stageinstantiate)。
