#!/usr/bin/env python3
"""Generate the standalone Ink IR instruction reference from the schema registry."""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


EXPECTED_SHAPE_COUNT = 22
EXPECTED_PAYLOAD_COUNT = 34
EXPECTED_CORE_OPCODE_COUNT = 137
EXPECTED_STAGE_OPCODES = (
    "stage.force_value",
    "stage.force_block",
    "stage.select_if",
    "stage.select_match",
    "stage.expand_for",
    "stage.expand_while",
    "stage.instantiate",
)

BRANCH_OPCODES = frozenset(("cf.br", "cf.cond_br"))
CALL_OPCODES = frozenset(("call.direct", "call.indirect", "call.virtual", "call.interface", "call.invoke", "async.call", "async.invoke", "async.continuation_invoke", "decorator.continuation_invoke"))
LOOKUP_OPCODES = frozenset(("reflect.lookup_type", "reflect.lookup_interface", "reflect.lookup_function", "reflect.lookup_member"))
AWAIT_OPCODES = frozenset(("async.await", "async.await_copy"))
RESULT_DESTINATION_OPCODES = frozenset(("cast.interface_make", "cast.interface_up", "cast.try_class", "cast.try_interface", "call.indirect", "call.virtual", "call.interface", "async.call", "async.invoke", "async.continuation_invoke", "reflect.call", "abi.call", "abi.invoke", "async.await_copy"))
FLEXIBLE_DESTINATION_OPCODES = frozenset(("call.direct", "call.invoke", "decorator.continuation_invoke"))

CORE_OPCODE_PURPOSES: dict[str, str] = {
    "const.int": "从ConstantTable物化一个类型与位宽准确匹配的整数SSA常量",
    "const.bool": "从ConstantTable物化一个bool SSA常量",
    "const.float": "从ConstantTable物化一个格式与位模式准确匹配的浮点SSA常量",
    "const.null": "物化指定raw pointer类型的null SSA常量",
    "const.unit": "物化Core唯一的零载荷unit SSA值",
    "const.symbol_addr": "物化带显式加数的已解析数据符号raw地址",
    "const.function": "物化绑定已解析函数符号与准确函数类型的函数指针常量",
    "cf.br": "把控制无条件转移到唯一successor并传递其edge arguments",
    "cf.cond_br": "按bool条件在true与false successor之间选择控制流",
    "cf.switch": "按整数或规范enum判别值在有序case与required default之间分派控制流",
    "cf.select": "按bool条件在两个同型SSA值之间执行无分支选择",
    "cf.return": "结束当前函数并返回与逻辑结果通道匹配的零个或一个值",
    "cf.unreachable": "声明当前控制点在所有合法动态执行中不可到达",
    "arith.add": "对同型定宽整数或ptrsize执行模2^N加法",
    "arith.sub": "对同型定宽整数或ptrsize执行模2^N减法",
    "arith.mul": "对同型定宽整数或ptrsize执行模2^N乘法",
    "arith.neg": "计算定宽整数或ptrsize的模2^N加法逆元",
    "arith.and": "对同型定宽整数或ptrsize执行逐位与",
    "arith.or": "对同型定宽整数或ptrsize执行逐位或",
    "arith.xor": "对同型定宽整数或ptrsize执行逐位异或",
    "arith.not": "对定宽整数或ptrsize执行逐位取反",
    "arith.cmp": "按已注册整数predicate比较两个同型整数、ptrsize或bool值并产生bool",
    "pdb.sdiv": "按PDB边界规则执行有符号整数除法并产生向零截断的商",
    "pdb.udiv": "按PDB边界规则执行无符号整数除法并产生商",
    "pdb.srem": "按PDB边界规则执行有符号整数除法并产生余数",
    "pdb.urem": "按PDB边界规则执行无符号整数除法并产生余数",
    "pdb.shl": "按PDB边界规则执行定宽整数左移",
    "pdb.lshr": "按PDB边界规则执行零扩展逻辑右移",
    "pdb.ashr": "按PDB边界规则执行符号扩展算术右移",
    "pdb.fptosi": "按PDB浮点边界规则把浮点值转换为有符号整数",
    "pdb.fptoui": "按PDB浮点边界规则把浮点值转换为无符号整数",
    "fp.add": "按TargetContext与fast-math标志执行浮点加法",
    "fp.sub": "按TargetContext与fast-math标志执行浮点减法",
    "fp.mul": "按TargetContext与fast-math标志执行浮点乘法",
    "fp.div": "按TargetContext与fast-math标志执行浮点除法",
    "fp.neg": "翻转浮点值的符号语义以执行浮点取负",
    "fp.fma": "按TargetContext与fast-math标志执行单次舍入的浮点乘加",
    "fp.cmp": "按已注册浮点predicate与TargetContext比较两个同型浮点值并产生bool",
    "fp.assume_finite": "把已验证的浮点有限性事实显式建立为后续优化可依赖的证明",
    "cast.int": "在中央允许的整数宽度与符号解释之间执行准确整数转换",
    "cast.int_to_float": "把整数或ptrsize数值转换为目标浮点格式",
    "cast.float": "在两个已注册浮点格式之间执行精度转换",
    "cast.bit": "在中央允许的两个同位宽标量类型之间重解释位模式",
    "cast.ptr": "在兼容address space内改变raw pointer的静态pointee类型",
    "cast.ptr_access": "收窄raw pointer的静态访问权限且保持地址不变",
    "cast.ref_access": "收窄reference的静态访问权限且保持对象身份不变",
    "cast.class_up": "沿已验证class继承边把对象view上转为指定base class",
    "cast.interface_make": "为对象构造目标interface reference并提交required destination",
    "cast.interface_up": "把interface reference上转为已验证ancestor interface并提交destination",
    "cast.try_class": "对class或interface view执行可失败的运行时class转换",
    "cast.try_interface": "对class或interface view执行可失败的运行时interface转换并写入Optional destination",
    "enum.value": "物化TargetABI已分类为单标量的无载荷enum variant值",
    "enum.init_variant": "在active enum初始化事务中选择并建立指定活动variant",
    "enum.discriminant": "读取enum的语义分支序号而不暴露物理tag或niche编码",
    "enum.is_variant": "判断enum当前活动分支是否为指定variant",
    "slice.init": "从首元素place与长度初始化非空安全slice的data、length及borrow generation",
    "slice.data": "读取安全slice的物理data raw pointer并显式丢弃checked borrow generation",
    "slice.length": "读取安全slice的ptrsize元素数量",
    "slice.index": "在bounds与generation验证后取得安全slice中指定元素的place",
    "slice.subslice": "在bounds与generation验证后派生安全slice的连续子区间",
    "slice.init_empty": "把active destination初始化为canonical null加零长度的空slice",
    "mem.alloca": "在当前函数frame中分配未初始化的typed stack storage并产生init place",
    "mem.global_place": "取得已完成对应lifecycle激活的global或thread-local对象place",
    "mem.load": "从Alive且对齐的typed place读取一个允许按值加载的值",
    "mem.store": "把一个同型值写入Alive且对齐的可写typed place",
    "mem.load_unaligned": "按显式alignment从允许非对齐访问的typed place读取值",
    "mem.store_unaligned": "按显式alignment向允许非对齐访问的typed place写入值",
    "place.deref": "把满足pointee与lifetime前置条件的raw pointer解释为typed place capability",
    "place.from_ref": "从reference借用语义派生访问权限不增强的typed place",
    "place.as_alive": "把路径上已证明Alive的init place重绑定为ro或rw place的新capability generation",
    "place.as_uninitialized": "把路径上已销毁且可重建的owner place重绑定为fresh init capability",
    "place.field": "投影到class或struct中由symbol指定的字段子对象place",
    "place.base": "投影到class对象中由symbol指定的base子对象place",
    "place.tuple_element": "按静态index投影到tuple的元素子对象place",
    "place.array_element": "按动态ptrsize index与bounds证明投影到array元素place",
    "place.enum_payload": "在匹配活动variant证明下投影到enum payload子对象place",
    "place.addr": "在NoEscape与lifetime约束下取得typed place对应的raw地址",
    "place.borrow": "从Alive typed place建立权限不增强且捕获当前generation的非空reference",
    "raw.load": "按显式alignment从合法raw地址读取允许raw访问的标量位模式",
    "raw.store": "按显式alignment向合法raw地址写入允许raw访问的标量位模式",
    "raw.memcpy": "在已证明不重叠的raw byte范围间复制准确byte count",
    "raw.memmove": "在允许重叠的raw byte范围间移动准确byte count",
    "raw.memset": "用单字节pattern填充合法raw byte范围",
    "ptr.offset": "按目标sizeof(T)缩放元素偏移并计算同型raw pointer",
    "ptr.byte_offset": "按原始byte offset计算同address-space raw pointer",
    "ptr.cmp": "按允许的地址predicate比较两个兼容address-space raw pointer",
    "obj.init.begin": "为未初始化root或合法child storage建立fresh对象初始化事务",
    "obj.init": "在active事务中用单标量或unit值初始化非sealed leaf对象",
    "obj.init.copy": "在active事务中按语义组成部分复制初始化Copyable address-only对象",
    "obj.init.commit": "验证完整初始化并把事务root提交为fresh Alive对象generation",
    "obj.assign.copy": "在两个Alive address-only对象之间执行普通Copyable赋值",
    "obj.destroy": "按静态完整类型执行nothrow析构链并结束对象lifetime generation",
    "obj.destroy_dynamic": "按匹配vtable与版本选择完整动态析构闭包并结束对象lifetime",
    "call.direct": "调用由SymbolId直接确定且签名、entry identity与ABI匹配的nothrow Ink目标",
    "call.indirect": "通过准确函数指针调用签名与ABI匹配的nothrow Ink目标",
    "call.virtual": "经已验证class slot与版本表分派nothrow virtual Ink调用",
    "call.interface": "经已验证interface method映射分派nothrow Ink调用",
    "call.invoke": "调用可能产生Ink unwind的目标并显式分流normal与unwind successor",
    "eh.entry": "在landing block入口物化该unwind边唯一传入的active exception token",
    "eh.match": "按有序typed handler与可选catch_all匹配active exception并保留unmatched传播边",
    "eh.payload": "在匹配证明下把active exception payload读取为指定类型view",
    "eh.end_catch": "结束当前catch对exception token的线性所有权并转移到normal successor",
    "eh.throw": "用指定异常类型与constructor从参数构造并抛出新的exception token",
    "eh.throw_copy": "复制可抛出异常对象并把副本封装为新的exception token",
    "eh.throw_from": "在保留cause链的同时构造并抛出新的exception token",
    "eh.rethrow": "把当前active exception token原样重新抛向外层unwind边界",
    "eh.resume": "从cleanup路径继续传播尚未被catch消费的exception token",
    "rt.trap": "按TrapKind触发可诊断且不返回的runtime trap语义",
    "rt.fatal": "按TrapKind触发进程级不可恢复且不返回的runtime fatal语义",
    "rt.version.pin": "从当前borrowed version owner建立独立线性owned version pin",
    "rt.version.unpin": "消费并释放一条owned version pin",
    "rt.version.transfer_pin": "把owned version pin的所有权转移到指定runtime owner种类",
    "rt.version.current_owner": "物化当前activation已有version owner的不可逃逸borrowed view",
    "async.call": "以同步nothrow construction构造sealed Task destination而不执行其异步body",
    "async.invoke": "以normal或unwind结果构造sealed Task destination并显式处理同步construction failure",
    "async.await": "驱动并等待Task完成，在normal边产生可按值结果或在unwind边产生exception",
    "async.await_copy": "驱动并等待Task完成，把address-only Copyable结果写入required destination",
    "async.task.drive_once": "把created或suspended Task至多推进一个合作式执行步骤",
    "async.task.publish_success": "在线性Task状态机中发布已完成的void、scalar或已提交address-only结果",
    "async.task.publish_failure": "在线性Task状态机中发布owned exception作为失败结果",
    "async.task.destroy": "按TaskDestroySummary关闭并销毁sealed Task frame与其剩余状态",
    "async.cancel.request": "原子设置Task的cooperative cancellation request状态",
    "async.cancel.is_requested": "读取Task当前是否已收到cooperative cancellation request",
    "async.continuation_invoke": "在同一Task与frame内调用下一层async decorator continuation并分流normal/unwind",
    "reflect.lookup_type": "按模块与字符串名称查询并在found边产生owned type snapshot",
    "reflect.lookup_interface": "按模块与字符串名称查询并在found边产生owned interface snapshot",
    "reflect.lookup_function": "按模块、字符串名称与准确函数类型查询owned function snapshot",
    "reflect.lookup_member": "在owner snapshot中按member kind与名称查询owner-bounded borrowed member view",
    "reflect.snapshot.clone": "从现有owned reflection snapshot建立同版本的独立owned clone",
    "reflect.snapshot.release": "消费并释放一条owned reflection snapshot handle",
    "reflect.call": "通过pinned function snapshot与已验证adapter执行nothrow reflection调用",
    "decorator.region": "承载一个staged decorator body region及其准确入口、出口与层级签名",
    "decorator.continuation_invoke": "激活下一层sync decorator continuation并分流normal与unwind",
    "decorator.continuation_yield": "从decorator body向包围应用返回指定result passing mode的值或空通道",
    "abi.call": "经已注册C ABI bridge调用已证明不会产生Ink unwind的foreign目标",
    "abi.invoke": "经已注册failure bridge调用foreign目标并把失败规范化到Ink unwind边",
    "ct.register_module_item": "在active decorator application中有序发射一个typed module registration记录",
}

INSTANCE_IDENTITY_TEXT_TEMPLATE = "{generic_declaration_symbol = @symbol, arguments = [TAGGED_CLOSED_GENERIC_ARGUMENT, ...], target_key = sha256\"HEX64\", instance_identity_digest = sha256\"HEX64\"}"
SOURCE_BACKED_ANCHOR_TEXT_TEMPLATE = "{owner_symbol = @symbol, region_structural_path = [REGION_PATH_STEP, ...], source_file_content_digest = sha256\"HEX64\", source_declaration_key = hex\"HEX_BYTES\", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], anchor_role_tag = ANCHOR_ROLE, source_backed_anchor_key = sha256\"HEX64\"}"
DECORATOR_APPLICATION_FRAME_TEXT_TEMPLATE = "{producer_symbol = @symbol, decorator_declaration_symbol = @decorator, application_anchor = " + SOURCE_BACKED_ANCHOR_TEXT_TEMPLATE + ", enclosing_instance = none|some(" + INSTANCE_IDENTITY_TEXT_TEMPLATE + "), decorator_application_key = sha256\"HEX64\"}"
DECORATOR_APPLICATION_IDENTITY_TEXT_TEMPLATE = "[" + DECORATOR_APPLICATION_FRAME_TEXT_TEMPLATE + ", ...]"
PARENT_ELABORATION_CONTEXT_TEXT_TEMPLATE = "{parent_canonical_work_key = none|some(sha256\"HEX64\"), enclosing_instance = none|some(" + INSTANCE_IDENTITY_TEXT_TEMPLATE + "), decorator_application = none|some(" + DECORATOR_APPLICATION_IDENTITY_TEXT_TEMPLATE + "), dynamic_control_path = DYNAMIC_CONTROL_PATH, parent_elaboration_context_digest = sha256\"HEX64\"}"
SOURCE_BACKED_CALLSITE_TEXT_TEMPLATE = "{producer_symbol = @symbol, source_file_content_digest = sha256\"HEX64\", source_declaration_key = hex\"HEX_BYTES\", normalized_structural_node_path = [SOURCE_STRUCTURAL_PATH_STEP, ...], callee_identity = core_opcode(ct.register_module_item), source_backed_callsite_key = sha256\"HEX64\"}"
PLAN_NODE_TEXT_TEMPLATE = "#pN = plan_node {stage_opcode = STAGE_OPCODE, inputs = [...], template_refs = [...], result_types = [...], sink = ..., parent_elaboration_context = " + PARENT_ELABORATION_CONTEXT_TEXT_TEMPLATE + ", required_capabilities = [...], dependency_plan_nodes = [...], canonical_work_key = sha256\"HEX64\", origin = #oN}"


class RegistryError(Exception):
    """Raised when the normative registry cannot be parsed exactly."""


@dataclass(frozen=True)
class SchemaEntry:
    tag: int
    name: str
    description: str
    text_form: str | None
    line_number: int


@dataclass(frozen=True)
class OpcodeEntry:
    tag: int
    hexadecimal_tag: str
    name: str
    schema: str
    rule: str
    effects: str
    stage: str
    traits: str
    source_heading: str
    source_line: int


@dataclass(frozen=True)
class StageOpcodeEntry:
    tag: int
    name: str
    source_heading: str
    source_line: int


@dataclass(frozen=True)
class StageSchemaEntry:
    tag: int
    name: str
    inputs: str
    templates: str
    outputs: str
    sink: str
    rule: str
    effects: str
    usage: str
    staging_section: str
    source_line: int


@dataclass(frozen=True)
class Registry:
    shapes: dict[int, SchemaEntry]
    payloads: dict[int, SchemaEntry]
    opcodes: tuple[OpcodeEntry, ...]
    stage_opcodes: tuple[StageOpcodeEntry, ...]
    stage_schemas: dict[int, StageSchemaEntry]


def strip_code(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value.startswith("`") and value.endswith("`"):
        return value[1:-1]
    return value


def split_markdown_row(line: str) -> list[str]:
    """Split one Markdown table row while preserving escaped pipe characters."""
    stripped = line.strip()
    if not stripped.startswith("|") or not stripped.endswith("|"):
        raise RegistryError(f"not a Markdown table row: {line}")

    cells: list[str] = []
    current: list[str] = []
    escaped = False
    for character in stripped[1:-1]:
        if escaped:
            if character == "|":
                current.append("|")
            else:
                current.extend(("\\", character))
            escaped = False
            continue
        if character == "\\":
            escaped = True
            continue
        if character == "|":
            cells.append("".join(current).strip())
            current = []
            continue
        current.append(character)
    if escaped:
        current.append("\\")
    cells.append("".join(current).strip())
    return cells


def heading_slug(heading: str) -> str:
    """Build a stable local anchor compatible with the headings in these drafts."""
    text = heading.lstrip("#").strip().replace("`", "").lower()
    characters: list[str] = []
    previous_hyphen = False
    for character in text:
        if character.isspace():
            if characters and not previous_hyphen:
                characters.append("-")
                previous_hyphen = True
            continue
        if character.isalnum() or character in ("-", "_"):
            characters.append(character)
            previous_hyphen = character == "-"
    return "".join(characters).strip("-")


def instruction_anchor(name: str) -> str:
    normalized = re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")
    return f"instruction-{normalized}"


def add_unique_by_tag(entries: dict[int, SchemaEntry], entry: SchemaEntry, category: str) -> None:
    if entry.tag in entries:
        previous = entries[entry.tag]
        raise RegistryError(f"duplicate {category} tag {entry.tag} at lines {previous.line_number} and {entry.line_number}")
    for previous in entries.values():
        if previous.name == entry.name:
            raise RegistryError(f"duplicate {category} name {entry.name!r} at lines {previous.line_number} and {entry.line_number}")
    entries[entry.tag] = entry


def parse_schema_row(cells: list[str], line_number: int, category: str) -> SchemaEntry:
    expected_cell_count = 4 if category == "PayloadSchema" else 3
    if len(cells) != expected_cell_count or not cells[0].isdigit():
        raise RegistryError(f"malformed {category} row at line {line_number}")
    name = strip_code(cells[1])
    if not name or name == cells[1]:
        raise RegistryError(f"{category} name must be code-formatted at line {line_number}")
    text_form = cells[3] if category == "PayloadSchema" else None
    if text_form is not None and not text_form:
        raise RegistryError(f"{category} text form must not be empty at line {line_number}")
    return SchemaEntry(tag=int(cells[0]), name=name, description=cells[2], text_form=text_form, line_number=line_number)


def parse_opcode_row(cells: list[str], line_number: int, source_heading: str) -> OpcodeEntry:
    if len(cells) != 7:
        raise RegistryError(f"malformed opcode row with {len(cells)} cells at line {line_number}")
    tag_match = re.fullmatch(r"(\d+)\s+\(`0x([0-9A-Fa-f]{4})`\)", cells[0])
    if tag_match is None:
        raise RegistryError(f"malformed opcode tag at line {line_number}: {cells[0]!r}")
    name = strip_code(cells[1])
    if not name or name == cells[1]:
        raise RegistryError(f"opcode name must be code-formatted at line {line_number}")
    tag = int(tag_match.group(1))
    hexadecimal_tag = tag_match.group(2).upper()
    if tag != int(hexadecimal_tag, 16):
        raise RegistryError(f"decimal and hexadecimal opcode tags disagree at line {line_number}")
    return OpcodeEntry(tag=tag, hexadecimal_tag=hexadecimal_tag, name=name, schema=cells[2], rule=cells[3], effects=cells[4], stage=cells[5], traits=cells[6], source_heading=source_heading, source_line=line_number)


def parse_stage_schema_row(cells: list[str], line_number: int) -> StageSchemaEntry:
    if len(cells) != 10 or not cells[0].isdigit():
        raise RegistryError(f"malformed StageOpcode schema row at line {line_number}")
    name = strip_code(cells[1])
    if not name or name == cells[1]:
        raise RegistryError(f"StageOpcode schema name must be code-formatted at line {line_number}")
    if not re.fullmatch(r"5\.[2-8]", cells[9]):
        raise RegistryError(f"invalid staging section at line {line_number}: {cells[9]!r}")
    return StageSchemaEntry(tag=int(cells[0]), name=name, inputs=cells[2], templates=cells[3], outputs=cells[4], sink=cells[5], rule=cells[6], effects=cells[7], usage=cells[8], staging_section=cells[9], source_line=line_number)


def parse_registry(path: Path) -> Registry:
    lines = path.read_text(encoding="utf-8").splitlines()
    shapes: dict[int, SchemaEntry] = {}
    payloads: dict[int, SchemaEntry] = {}
    opcodes: list[OpcodeEntry] = []
    stage_opcodes: list[StageOpcodeEntry] = []
    stage_schemas: dict[int, StageSchemaEntry] = {}
    current_h2 = ""
    current_h3 = ""
    current_h4 = ""
    active_enum = ""

    for line_number, line in enumerate(lines, start=1):
        if line.startswith("## "):
            current_h2 = line
            current_h3 = ""
            current_h4 = ""
            active_enum = ""
            continue
        if line.startswith("### "):
            current_h3 = line
            current_h4 = ""
            active_enum = ""
            continue
        if line.startswith("#### "):
            current_h4 = line
            active_enum = ""
            continue
        stripped_line = line.strip()
        if not stripped_line.startswith("|"):
            continue
        if not stripped_line.endswith("|"):
            if "`0x" in stripped_line or ((current_h3.startswith("### 12.2 ") or current_h3.startswith("### 12.3 ")) and re.match(r"^\|\s*\d", stripped_line)):
                raise RegistryError(f"unterminated normative table row at line {line_number}")
            continue

        cells = split_markdown_row(line)
        if current_h3.startswith("### 12.2 ") and cells and cells[0].isdigit():
            add_unique_by_tag(shapes, parse_schema_row(cells, line_number, "ShapeSchema"), "ShapeSchema")
            continue
        if current_h3.startswith("### 12.3 ") and cells and cells[0].isdigit():
            add_unique_by_tag(payloads, parse_schema_row(cells, line_number, "PayloadSchema"), "PayloadSchema")
            continue
        if current_h3.startswith("### 10.3 ") and len(cells) == 3:
            enum_name = strip_code(cells[0])
            if enum_name and enum_name not in ("Enum", "---"):
                active_enum = enum_name
            if active_enum == "StageOpcodeTag" and cells[1].isdigit():
                name = strip_code(cells[2])
                if not name or name == cells[2]:
                    raise RegistryError(f"StageOpcodeTag name must be code-formatted at line {line_number}")
                stage_opcodes.append(StageOpcodeEntry(tag=int(cells[1]), name=name, source_heading=current_h3, source_line=line_number))
            continue
        if current_h4.startswith("#### 10.3.1 ") and cells and cells[0].isdigit():
            stage_schema = parse_stage_schema_row(cells, line_number)
            if stage_schema.tag in stage_schemas:
                raise RegistryError(f"duplicate StageOpcode schema tag {stage_schema.tag} at lines {stage_schemas[stage_schema.tag].source_line} and {line_number}")
            stage_schemas[stage_schema.tag] = stage_schema
            continue
        if (current_h2.startswith("## 13.") or current_h2.startswith("## 14.")) and "`0x" in cells[0]:
            opcodes.append(parse_opcode_row(cells, line_number, current_h3))

    registry = Registry(shapes=shapes, payloads=payloads, opcodes=tuple(opcodes), stage_opcodes=tuple(stage_opcodes), stage_schemas=stage_schemas)
    validate_registry(registry)
    return registry


def validate_unique(entries: tuple[OpcodeEntry, ...] | tuple[StageOpcodeEntry, ...], category: str) -> None:
    tags: dict[int, int] = {}
    names: dict[str, int] = {}
    for entry in entries:
        if entry.tag in tags:
            raise RegistryError(f"duplicate {category} tag {entry.tag} at lines {tags[entry.tag]} and {entry.source_line}")
        if entry.name in names:
            raise RegistryError(f"duplicate {category} name {entry.name!r} at lines {names[entry.name]} and {entry.source_line}")
        tags[entry.tag] = entry.source_line
        names[entry.name] = entry.source_line


def validate_registry(registry: Registry) -> None:
    if len(registry.shapes) != EXPECTED_SHAPE_COUNT:
        raise RegistryError(f"expected {EXPECTED_SHAPE_COUNT} ShapeSchema rows, found {len(registry.shapes)}")
    if len(registry.payloads) != EXPECTED_PAYLOAD_COUNT:
        raise RegistryError(f"expected {EXPECTED_PAYLOAD_COUNT} PayloadSchema rows, found {len(registry.payloads)}")
    if len(registry.opcodes) != EXPECTED_CORE_OPCODE_COUNT:
        raise RegistryError(f"expected {EXPECTED_CORE_OPCODE_COUNT} Core opcode rows, found {len(registry.opcodes)}")
    if len(registry.stage_opcodes) != len(EXPECTED_STAGE_OPCODES):
        raise RegistryError(f"expected {len(EXPECTED_STAGE_OPCODES)} StageOpcodeTag rows, found {len(registry.stage_opcodes)}")
    if len(registry.stage_schemas) != len(EXPECTED_STAGE_OPCODES):
        raise RegistryError(f"expected {len(EXPECTED_STAGE_OPCODES)} StageOpcode schema rows, found {len(registry.stage_schemas)}")

    validate_unique(registry.opcodes, "Core opcode")
    validate_unique(registry.stage_opcodes, "StageOpcodeTag")

    opcode_names = {entry.name for entry in registry.opcodes}
    purpose_names = set(CORE_OPCODE_PURPOSES)
    if opcode_names != purpose_names:
        missing = sorted(opcode_names - purpose_names)
        extra = sorted(purpose_names - opcode_names)
        raise RegistryError(f"Core opcode purpose coverage mismatch: missing={missing!r}, extra={extra!r}")
    if any(not purpose.strip() or "同上" in purpose for purpose in CORE_OPCODE_PURPOSES.values()):
        raise RegistryError("Core opcode purposes must be non-empty and must not use ditto wording")
    purpose_owners: dict[str, list[str]] = {}
    for opcode_name, purpose in CORE_OPCODE_PURPOSES.items():
        purpose_owners.setdefault(purpose, []).append(opcode_name)
    duplicate_purposes = {purpose: names for purpose, names in purpose_owners.items() if len(names) > 1}
    if duplicate_purposes:
        raise RegistryError(f"Core opcode purposes must be opcode-specific: {duplicate_purposes!r}")
    stage_names = tuple(entry.name for entry in sorted(registry.stage_opcodes, key=lambda entry: entry.tag))
    if stage_names != EXPECTED_STAGE_OPCODES:
        raise RegistryError(f"StageOpcodeTag is incomplete or reordered: expected {EXPECTED_STAGE_OPCODES!r}, found {stage_names!r}")
    for opcode in registry.stage_opcodes:
        schema = registry.stage_schemas.get(opcode.tag)
        if schema is None or schema.name != opcode.name:
            raise RegistryError(f"StageOpcodeTag {opcode.tag} {opcode.name!r} has no matching central schema row")

    for opcode in registry.opcodes:
        references = re.findall(r"S(\d+)/P(\d+)", opcode.schema)
        if not references:
            raise RegistryError(f"opcode {opcode.name!r} at line {opcode.source_line} has no Sx/Py schema reference")
        for shape_tag_text, payload_tag_text in references:
            shape_tag = int(shape_tag_text)
            payload_tag = int(payload_tag_text)
            if shape_tag not in registry.shapes:
                raise RegistryError(f"opcode {opcode.name!r} references missing ShapeSchemaTag {shape_tag}")
            if payload_tag not in registry.payloads:
                raise RegistryError(f"opcode {opcode.name!r} references missing PayloadSchemaTag {payload_tag}")

    anchors = [instruction_anchor(entry.name) for entry in registry.opcodes]
    anchors.extend(instruction_anchor(entry.name) for entry in registry.stage_opcodes)
    if len(anchors) != len(set(anchors)):
        raise RegistryError("instruction names produce duplicate Markdown anchors")
    validate_special_renderers(registry)


def section_number(heading: str) -> str:
    match = re.match(r"###\s+(\d+\.\d+)", heading)
    if match is None:
        raise RegistryError(f"cannot extract section number from heading {heading!r}")
    return match.group(1)


def schema_references(opcode: OpcodeEntry) -> tuple[tuple[int, int], ...]:
    references: list[tuple[int, int]] = []
    seen: set[tuple[int, int]] = set()
    for shape_tag_text, payload_tag_text in re.findall(r"S(\d+)/P(\d+)", opcode.schema):
        reference = (int(shape_tag_text), int(payload_tag_text))
        if reference not in seen:
            seen.add(reference)
            references.append(reference)
    return tuple(references)


def count_placeholder(prefix: str, count: str) -> str:
    if count == "0":
        return ""
    if count == "1":
        return f"%{prefix}0"
    if count.isdigit():
        return ", ".join(f"%{prefix}{index}" for index in range(int(count)))
    if count == "*":
        return "OPERANDS"
    if count == "0|1":
        return f"[%{prefix}0]"
    raise RegistryError(f"unsupported structural count {count!r}")


def structural_counts(shape: SchemaEntry) -> tuple[str, str, str, str, str]:
    match = re.search(r"R([0-9*|]+) O([0-9*|]+) G([0-9*|]+) S([0-9*|]+) D([0-9*|]+)", shape.description)
    if match is None:
        raise RegistryError(f"cannot parse structural shape S{shape.tag}: {shape.description!r}")
    return match.groups()


def renderer_kind(opcode: OpcodeEntry) -> str:
    if opcode.name in BRANCH_OPCODES:
        return "branch"
    if opcode.name == "cf.switch":
        return "switch"
    if opcode.name == "eh.match":
        return "eh_match"
    if opcode.name == "eh.end_catch":
        return "eh_end_catch"
    if opcode.name in CALL_OPCODES:
        return "call"
    if opcode.name == "reflect.call":
        return "reflect_call"
    if opcode.name in ("abi.call", "abi.invoke"):
        return "abi_call"
    if opcode.name in LOOKUP_OPCODES:
        return "lookup"
    if opcode.name in AWAIT_OPCODES:
        return "await"
    if opcode.name == "decorator.region":
        return "region"
    return "generic_operation"


def payload_template(opcode: OpcodeEntry, payload: SchemaEntry) -> str:
    if payload.tag in (1, 8, 14, 15, 25, 26, 27, 30, 32):
        return ""
    if payload.tag == 7:
        return "[{fast = [FAST_MATH_FLAGS]}]"
    if payload.tag == 9:
        if opcode.name in ("place.base", "cast.class_up"):
            return "{base = @symbol}"
        if opcode.name == "place.field":
            return "{field = @symbol}"
        if opcode.name in ("enum.value", "enum.init_variant", "enum.is_variant", "place.enum_payload"):
            return "{variant = @symbol}"
        raise RegistryError(f"opcode {opcode.name!r} has no SelectorSymbol text key")
    if payload.tag == 34:
        if opcode.name != "ct.register_module_item":
            raise RegistryError(f"opcode {opcode.name!r} cannot use ModuleRegistration payload")
        return "{value_form = VALUE_FORM, source_backed_callsite = " + SOURCE_BACKED_CALLSITE_TEXT_TEMPLATE + "}"
    if payload.text_form is not None:
        code_fragments = re.findall(r"`([^`]+)`", payload.text_form)
        if code_fragments:
            return code_fragments[0]
    raise RegistryError(f"payload P{payload.tag} {payload.name!r} has no mechanical text template")


def result_prefix(result_count: str) -> str:
    if result_count == "0":
        return ""
    if result_count == "1":
        return "%result0 = "
    if result_count == "0|1":
        return "[%result0 = ]"
    raise RegistryError(f"unsupported result count {result_count!r}")


def destination_template(destination_count: str, role: str = "ROLE") -> str:
    clause = f"to %destination {{destination_role = {role}}}"
    if destination_count == "0":
        return ""
    if destination_count == "1":
        return " " + clause
    if destination_count == "0|1":
        return " [" + clause + "]"
    raise RegistryError(f"unsupported destination count {destination_count!r}")


def destination_role(opcode: OpcodeEntry) -> str:
    if opcode.name in RESULT_DESTINATION_OPCODES:
        return "result"
    return "ROLE"


def operation_suffix() -> str:
    return " : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)"


def argument_list() -> str:
    return "([%arg0, ...])"


def grammar_variants(forms: tuple[tuple[str, str], ...]) -> str:
    return "\n".join(f"{label} ::= {form}" for label, form in forms)


def generic_operation_template(opcode: OpcodeEntry, shape: SchemaEntry, payload: SchemaEntry) -> str:
    result_count, operand_count, region_count, successor_count, destination_count = structural_counts(shape)
    if region_count != "0" or successor_count != "0":
        raise RegistryError(f"generic renderer cannot encode regions/successors for {opcode.name!r}")

    pieces = [result_prefix(result_count) + opcode.name]
    optional_single_operands = {"cf.return": "[%value]", "rt.fatal": "[%active]", "decorator.continuation_yield": "[%value]"}
    operands = optional_single_operands.get(opcode.name, count_placeholder("operand", operand_count))
    if operands:
        pieces.append(f" {operands}")
    payload_text = payload_template(opcode, payload)
    if payload_text:
        pieces.append(" " + payload_text)
    pieces.append(destination_template(destination_count, destination_role(opcode)))
    pieces.append(operation_suffix())
    return "".join(pieces)


def call_template(opcode: OpcodeEntry, shape: SchemaEntry) -> str:
    _, _, _, _, destination_count = structural_counts(shape)
    call_payload = "{function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N}"
    arguments = argument_list()
    if opcode.name == "call.direct":
        body = "call.direct @symbol" + arguments + " " + call_payload
        return grammar_variants((("VOID_OR_NEVER", body + operation_suffix()), ("UNIT_OR_SCALAR", "%result0 = " + body + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + operation_suffix()), ("CONSTRUCTOR", body + destination_template("1", "initializing_receiver") + operation_suffix())))
    elif opcode.name == "call.indirect":
        body = "call.indirect %callee" + arguments + " " + call_payload
    elif opcode.name == "call.virtual":
        body = "call.virtual %receiver {slot = @symbol}" + arguments + " " + call_payload
    elif opcode.name == "call.interface":
        body = "call.interface %receiver {method = @symbol}" + arguments + " " + call_payload
    elif opcode.name == "call.invoke":
        body = "call.invoke callee_kind = CALLEE_KIND CALLEE" + arguments + " " + call_payload
        constructor_body = "call.invoke callee_kind = direct @symbol" + arguments + " " + call_payload
        return grammar_variants((("VOID_OR_NEVER", body + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()), ("UNIT_OR_SCALAR", body + " normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE" + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()), ("CONSTRUCTOR", constructor_body + destination_template("1", "initializing_receiver") + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix())))
    elif opcode.name in ("async.call", "async.invoke"):
        body = opcode.name + " callee_kind = CALLEE_KIND CALLEE" + arguments + " " + call_payload + destination_template(destination_count, "result")
        if opcode.name == "async.invoke":
            body += " normal NORMAL_EDGE unwind UNWIND_EDGE"
        return body + operation_suffix()
    elif opcode.name in ("async.continuation_invoke", "decorator.continuation_invoke"):
        body = opcode.name + " {next_layer = N, function_type = !tN}" + arguments
        forms = [("VOID_OR_NEVER", body + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()), ("UNIT_OR_SCALAR", body + " normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE" + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix())]
        if opcode.name == "decorator.continuation_invoke":
            forms.append(("FORWARDED_CONSTRUCTOR", body + destination_template("1", "initializing_receiver") + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()))
        return grammar_variants(tuple(forms))
    else:
        raise RegistryError(f"unhandled call renderer for {opcode.name!r}")
    return grammar_variants((("VOID_OR_NEVER", body + operation_suffix()), ("UNIT_OR_SCALAR", "%result0 = " + body + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + operation_suffix())))


def renderer_template(opcode: OpcodeEntry, shape: SchemaEntry, payload: SchemaEntry) -> str:
    kind = renderer_kind(opcode)
    if kind == "generic_operation":
        return generic_operation_template(opcode, shape, payload)
    if kind == "branch":
        if opcode.name == "cf.br":
            return "cf.br EDGE" + operation_suffix()
        return "cf.cond_br %condition, TRUE_EDGE, FALSE_EDGE" + operation_suffix()
    if kind == "switch":
        return "cf.switch %key [case #cN -> CASE_EDGE, ...] default DEFAULT_EDGE" + operation_suffix()
    if kind == "eh_match":
        return "eh.match %active [!tN -> ^bbN(%active), ..., catch_all -> ^bbC(%active)] unmatched ^bbU(%active)" + operation_suffix()
    if kind == "eh_end_catch":
        return "eh.end_catch %active -> NORMAL_EDGE" + operation_suffix()
    if kind == "call":
        return call_template(opcode, shape)
    if kind == "reflect_call":
        body = "reflect.call %snapshot receiver(none|%receiver) args" + argument_list() + " {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true}"
        return grammar_variants((("VOID_OR_NEVER", body + operation_suffix()), ("UNIT_OR_SCALAR", "%result0 = " + body + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + operation_suffix())))
    if kind == "abi_call":
        body = opcode.name + " C_CALLEE" + argument_list() + " {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N}"
        if opcode.name == "abi.invoke":
            return grammar_variants((("VOID_OR_NEVER", body + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()), ("UNIT_OR_SCALAR", body + " normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE" + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix())))
        return grammar_variants((("VOID_OR_NEVER", body + operation_suffix()), ("UNIT_OR_SCALAR", "%result0 = " + body + operation_suffix()), ("ADDRESS_RESULT", body + destination_template("1", "result") + operation_suffix())))
    if kind == "lookup":
        payload_text = payload_template(opcode, payload)
        payload_clause = " " + payload_text if payload_text else ""
        operands = "%owner, %name" if opcode.name == "reflect.lookup_member" else "%qualified_name"
        return opcode.name + " " + operands + payload_clause + " found FOUND_EDGE missing MISSING_EDGE" + operation_suffix()
    if kind == "await":
        if opcode.name == "async.await":
            return "async.await %task normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()
        return "async.await_copy %task" + destination_template("1", "result") + " normal NORMAL_EDGE unwind UNWIND_EDGE" + operation_suffix()
    if kind == "region":
        payload_text = payload_template(opcode, payload)
        return opcode.name + " OPERANDS " + payload_text + operation_suffix() + " region(decorator_body, origin(#oN)) {\n  BLOCKS\n}"
    raise RegistryError(f"unhandled renderer kind {kind!r} for {opcode.name!r}")


SPECIAL_RENDERER_GOLDENS = {
    "cf.return": "cf.return [%value] : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "call.direct": "VOID_OR_NEVER ::= call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nUNIT_OR_SCALAR ::= %result0 = call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nADDRESS_RESULT ::= call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nCONSTRUCTOR ::= call.direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} to %destination {destination_role = initializing_receiver} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "call.invoke": "VOID_OR_NEVER ::= call.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nUNIT_OR_SCALAR ::= call.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nADDRESS_RESULT ::= call.invoke callee_kind = CALLEE_KIND CALLEE([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nCONSTRUCTOR ::= call.invoke callee_kind = direct @symbol([%arg0, ...]) {function_type = !tN, entry_identity = ENTRY_IDENTITY, calling_convention = ink, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} to %destination {destination_role = initializing_receiver} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "cast.interface_make": "cast.interface_make OPERANDS {interface = @symbol} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "async.continuation_invoke": "VOID_OR_NEVER ::= async.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nUNIT_OR_SCALAR ::= async.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nADDRESS_RESULT ::= async.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "decorator.continuation_invoke": "VOID_OR_NEVER ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nUNIT_OR_SCALAR ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) normal NORMAL_EDGE_WITH_RESULT0 unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nADDRESS_RESULT ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) to %destination {destination_role = result} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nFORWARDED_CONSTRUCTOR ::= decorator.continuation_invoke {next_layer = N, function_type = !tN}([%arg0, ...]) to %destination {destination_role = initializing_receiver} normal NORMAL_EDGE unwind UNWIND_EDGE : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "reflect.call": "VOID_OR_NEVER ::= reflect.call %snapshot receiver(none|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nUNIT_OR_SCALAR ::= %result0 = reflect.call %snapshot receiver(none|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nADDRESS_RESULT ::= reflect.call %snapshot receiver(none|%receiver) args([%arg0, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "abi.call": "VOID_OR_NEVER ::= abi.call C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nUNIT_OR_SCALAR ::= %result0 = abi.call C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)\nADDRESS_RESULT ::= abi.call C_CALLEE([%arg0, ...]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = sha256\"HEX64\", explicit_argument_count = N} to %destination {destination_role = result} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN)",
    "decorator.region": "decorator.region OPERANDS {decorator_kind = name, layer = N, function_type = !tN} : (OPERAND_TYPES) -> CHANNEL_TYPES loc(#oN) region(decorator_body, origin(#oN)) {\n  BLOCKS\n}",
}


def validate_special_renderers(registry: Registry) -> None:
    opcodes = {opcode.name: opcode for opcode in registry.opcodes}
    for name, expected in SPECIAL_RENDERER_GOLDENS.items():
        opcode = opcodes.get(name)
        if opcode is None:
            raise RegistryError(f"special renderer golden references missing opcode {name!r}")
        references = schema_references(opcode)
        if len(references) != 1:
            raise RegistryError(f"special renderer golden for {name!r} requires one schema variant")
        shape_tag, payload_tag = references[0]
        actual = renderer_template(opcode, registry.shapes[shape_tag], registry.payloads[payload_tag])
        if actual != expected:
            raise RegistryError(f"special renderer drift for {name!r}: expected {expected!r}, found {actual!r}")

    for name in RESULT_DESTINATION_OPCODES:
        opcode = opcodes[name]
        for shape_tag, payload_tag in schema_references(opcode):
            actual = renderer_template(opcode, registry.shapes[shape_tag], registry.payloads[payload_tag])
            if "destination_role = ROLE" in actual:
                raise RegistryError(f"result destination renderer for {name!r} retained a free role")

    for name in FLEXIBLE_DESTINATION_OPCODES:
        opcode = opcodes[name]
        for shape_tag, payload_tag in schema_references(opcode):
            actual = renderer_template(opcode, registry.shapes[shape_tag], registry.payloads[payload_tag])
            if "destination_role = result" not in actual or "destination_role = initializing_receiver" not in actual:
                raise RegistryError(f"flexible destination renderer for {name!r} does not enumerate both legal roles")

    for name in CALL_OPCODES | frozenset(("reflect.call", "abi.call", "abi.invoke")):
        opcode = opcodes[name]
        for shape_tag, payload_tag in schema_references(opcode):
            actual = renderer_template(opcode, registry.shapes[shape_tag], registry.payloads[payload_tag])
            if "(%arg0, ...)" in actual or "args(%arg0, ...)" in actual:
                raise RegistryError(f"call-like renderer for {name!r} cannot represent zero explicit arguments")

    for name in ("call.direct", "call.indirect", "call.virtual", "call.interface", "reflect.call", "abi.call"):
        opcode = opcodes[name]
        shape_tag, payload_tag = schema_references(opcode)[0]
        actual = renderer_template(opcode, registry.shapes[shape_tag], registry.payloads[payload_tag])
        if "VOID_OR_NEVER ::= " not in actual or "UNIT_OR_SCALAR ::= " not in actual or "ADDRESS_RESULT ::= " not in actual or "[%result0 = ]" in actual or ("ADDRESS_RESULT ::= " in actual and " [to %destination" in actual):
            raise RegistryError(f"result/destination variants for {name!r} are not explicit and mutually exclusive")


def render_core_format(opcode: OpcodeEntry, registry: Registry) -> list[str]:
    lines: list[str] = []
    references = schema_references(opcode)
    for index, (shape_tag, payload_tag) in enumerate(references, start=1):
        shape = registry.shapes[shape_tag]
        payload = registry.payloads[payload_tag]
        label = "Canonical renderer template" if len(references) == 1 else f"Canonical renderer template（变体 {index}）"
        template = renderer_template(opcode, shape, payload)
        lines.append(f"Renderer：`{renderer_kind(opcode)}`。{label}：")
        lines.append("")
        if "\n" in template:
            lines.extend(("```text", template, "```"))
        else:
            lines.append(f"`{template}`")
        lines += ["", f"结构约束：S{shape.tag} `{shape.name}` — {shape.description}。", "", f"Payload：P{payload.tag} `{payload.name}` — {payload.description}。Canonical text：{payload.text_form}。", ""]
    lines.append("大写标识符、`%...`和`BLOCKS`是按该条schema替换的元变量；`LABEL ::= form`逐行列出互斥格式，明确写作`[%arg0, ...]`、`[%value]`、`[%active]`或`[{fast = ...}]`的段可以整体为空，`OPERANDS`按该条type/CFG rule展开为可为空的逗号分隔operand序列。`switch` case list、`eh.match` handler list及其他list/vector周围的`[`与`]`是canonical literal delimiter，不可省略。其余关键字、标点、payload字段顺序、successor顺序、共同type signature与`loc(#oN)`均为canonical renderer固定内容，不得省略或重排。")
    return lines


def family_guidance(opcode: OpcodeEntry) -> str:
    families = (
        ("const.", "从ConstantTable把已验证常量物化为准确标量SSA值时使用"),
        ("cf.", "构造显式CFG边、合流或函数控制终点时使用"),
        ("arith.", "对定宽整数或ptrsize执行已定义的算术/比较语义时使用"),
        ("ptr.", "在扁平目标地址语义下执行raw pointer位移或比较时使用"),
        ("fp.", "按TargetContext浮点模式执行浮点运算时使用"),
        ("cast.", "在中央转换闭集允许的两种准确类型之间转换时使用"),
        ("pdb.", "需要按PDB表处理目标相关边界行为时使用"),
        ("enum.", "建立、查询或读取enum判别与payload时使用"),
        ("slice.", "建立或使用generation-aware安全slice时使用"),
        ("mem.", "访问已验证typed allocation/global/object storage时使用"),
        ("place.", "派生或重绑定typed place capability时使用"),
        ("raw.", "显式采用raw byte/pointer语义且已承担其前置条件时使用"),
        ("obj.", "显式推进对象初始化、赋值、销毁或rollback状态机时使用"),
        ("call.", "调用Ink逻辑签名并保留callee/destination/异常契约时使用"),
        ("eh.", "在线性exception token与显式unwind CFG上操作时使用"),
        ("rt.", "请求已注册runtime终止、trap或其他语义处理时使用"),
        ("async.", "构造、驱动、等待、发布或销毁sealed Task状态时使用"),
        ("reflect.", "通过受版本固定的runtime reflection schema查询或调用时使用"),
        ("decorator.", "执行staged decorator region或其continuation时使用"),
        ("abi.", "穿过已注册C ABI bridge且保留外部effect上界时使用"),
        ("ct.", "在ComptimeWorld中产生有序语义输出时使用"),
    )
    for prefix, guidance in families:
        if opcode.name.startswith(prefix):
            return guidance
    return "需要该中央opcode的准确语义而没有更专用schema时使用"


def schema_usage(opcode: OpcodeEntry, registry: Registry) -> str:
    descriptions: list[str] = []
    for shape_tag, payload_tag in schema_references(opcode):
        shape = registry.shapes[shape_tag]
        payload = registry.payloads[payload_tag]
        descriptions.append(f"选择S{shape.tag}/P{payload.tag}变体时，必须同时满足`{shape.name}`的arity、`{payload.name}`的字段顺序和该opcode的type/CFG rule")
    guidance = [f"当程序语义需要{CORE_OPCODE_PURPOSES[opcode.name]}时使用`{opcode.name}`；{family_guidance(opcode)}；具体前置条件为“{opcode.rule}”", "；".join(descriptions)]
    if "Terminator" in opcode.traits:
        guidance.append("它必须是block最后一条operation，所有normal/unwind/found/missing等successor及edge sentinel都显式编码")
    if "DestinationChannel" in opcode.traits:
        guidance.append("destination是独立owner/init通道而非普通operand/result，必须具有匹配role、active transaction及normal/unwind后置条件")
    if "MaySuspend" in opcode.traits:
        guidance.append("跨暂停存活的place、generation、cleanup、pin和Task状态必须frame-resident并在resume/destroy两路闭合")
    if "HasRegions" in opcode.traits:
        guidance.append("nested region的role、entry/exit signature与terminator必须匹配owner schema，不能把region改写成自由attribute")
    if "PlaceProducing" in opcode.traits:
        guidance.append("结果place保留准确allocation/path/access/lifetime authority，不能由相同数值地址伪造")
    if "Lifetime" in opcode.traits:
        guidance.append("使用点必须满足当前object/transaction generation，optimizer不得跨生命周期边界复制或重排")
    if "OwnedHandle" in opcode.traits:
        guidance.append("产生或消费的runtime handle按每条动态路径线性拥有并最终恰好释放或转移")
    if "BorrowedHandle" in opcode.traits:
        guidance.append("借用handle不得逃逸其owner/version/generation覆盖范围")
    if "Speculatable" in opcode.traits:
        guidance.append("仅在operand已可用且类型/TargetContext事实不变时才可推测、CSE或重排")
    if "OrderedSemanticEmit" in opcode.traits:
        guidance.append("执行必须保留application内源码顺序，不可删除、复制、CSE或越过其他semantic emit")
    guidance.append("result、operand、destination、successor和region只能来自schema；effect与trait由registry重算，不能用自由attribute补造")
    return "。".join(guidance) + "。"


def render_core_section(opcode: OpcodeEntry, registry: Registry) -> list[str]:
    source_slug = heading_slug(opcode.source_heading)
    source_number = section_number(opcode.source_heading)
    lines = [
        f'<a id="{instruction_anchor(opcode.name)}"></a>',
        f"## `{opcode.name}`",
        "",
        "### 格式",
        "",
    ]
    lines.extend(render_core_format(opcode, registry))
    lines += [
        "",
        "### 作用",
        "",
        f"{CORE_OPCODE_PURPOSES[opcode.name]}。类型/CFG约束：{opcode.rule}。",
        "",
        "### 用法",
        "",
        schema_usage(opcode, registry),
        "",
        "### 效果",
        "",
        opcode.effects,
        "",
        "### 阶段",
        "",
        opcode.stage,
        "",
        "### Traits",
        "",
        opcode.traits,
        "",
        "### 来源",
        "",
        f"[Schema Registry §{source_number}](./10-schema-registry.md#{source_slug})，OpcodeTag `{opcode.tag}` (`0x{opcode.hexadecimal_tag}`)；[ShapeSchemaTag](./10-schema-registry.md#122-shapeschematag)；[PayloadSchemaTag](./10-schema-registry.md#123-payloadschematag)。",
        "",
    ]
    return lines


def render_stage_section(opcode: StageOpcodeEntry, registry: Registry) -> list[str]:
    detail = registry.stage_schemas[opcode.tag]
    registry_slug = heading_slug(opcode.source_heading)
    staging_heading = f"### {detail.staging_section} `{opcode.name}`"
    staging_slug = heading_slug(staging_heading)
    return [
        f'<a id="{instruction_anchor(opcode.name)}"></a>',
        f"## `{opcode.name}`",
        "",
        "### 格式",
        "",
        f"Renderer：`generic_record`。Canonical renderer template：`{PLAN_NODE_TEXT_TEMPLATE.replace('STAGE_OPCODE', opcode.name)}`",
        "",
        "### 作用",
        "",
        detail.rule,
        "",
        "### 用法",
        "",
        f"Inputs：{detail.inputs}。TemplateRefs：{detail.templates}。Outputs：{detail.outputs}。Sink：{detail.sink}。{detail.usage}。",
        "",
        "### 效果",
        "",
        detail.effects,
        "",
        "### 阶段",
        "",
        "Plan（`ElaborationPlan`；不属于 Core operation）。",
        "",
        "### Traits",
        "",
        "—（Plan node；Core `OpcodeTraits` 不适用）。",
        "",
        "### 来源",
        "",
        f"[Schema Registry §10.3](./10-schema-registry.md#{registry_slug})，StageOpcodeTag `{opcode.tag}`；[StageOpcode schema §10.3.1](./10-schema-registry.md#1031-stageopcode-schema)；[Staging §{detail.staging_section}](./05-staging-comptime.md#{staging_slug})。",
        "",
    ]


def generate_document(registry: Registry) -> str:
    core_opcodes = tuple(sorted(registry.opcodes, key=lambda entry: entry.tag))
    stage_opcodes = tuple(sorted(registry.stage_opcodes, key=lambda entry: entry.tag))
    all_entries: tuple[OpcodeEntry | StageOpcodeEntry, ...] = core_opcodes + stage_opcodes
    lines = ["# 指令", ""]
    lines.extend(f"- [`{entry.name}`](#{instruction_anchor(entry.name)})" for entry in all_entries)
    lines.append("")
    for opcode in core_opcodes:
        lines.extend(render_core_section(opcode, registry))
    for opcode in stage_opcodes:
        lines.extend(render_stage_section(opcode, registry))
    return "\n".join(lines).rstrip() + "\n"


def parse_arguments() -> argparse.Namespace:
    script_path = Path(__file__).resolve()
    ir_directory = script_path.parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--registry", type=Path, default=ir_directory / "10-schema-registry.md", help="path to 10-schema-registry.md")
    parser.add_argument("--output", type=Path, default=ir_directory / "11-instruction-reference.md", help="generated instruction reference path")
    parser.add_argument("--check", action="store_true", help="fail if the output is absent or stale")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        registry = parse_registry(arguments.registry)
        document = generate_document(registry)
        if arguments.check:
            if not arguments.output.exists():
                raise RegistryError(f"generated output does not exist: {arguments.output}")
            if arguments.output.read_text(encoding="utf-8") != document:
                raise RegistryError(f"generated output is stale: {arguments.output}")
            action = "verified"
        else:
            arguments.output.write_text(document, encoding="utf-8", newline="\n")
            action = "generated"
    except (OSError, RegistryError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    print(f"{action} {arguments.output}: {len(registry.opcodes)} Core opcodes, {len(registry.stage_opcodes)} plan opcodes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
