#!/usr/bin/env python3
"""Generate InkVM LLVM TableGen register and opcode metadata from VM .def files.

Single source of truth:
  src/include/ink/backend/vm/instruction/register.def
  src/include/ink/backend/vm/instruction/opcode.def
  src/include/ink/backend/vm/instruction/compare_branch.def
  src/include/ink/backend/vm/instruction/integer_compare.def
  src/include/ink/backend/vm/instruction/register_compare_branch.def
  src/include/ink/backend/vm/instruction/float_compare.def

Usage:
  python cmake/generate_inkvm_tablegen.py --vm-instruction-dir src/include/ink/backend/vm/instruction --register-td build/InkVMRegisterInfoGen.td --opcode-td build/InkVMOpcodeMetaGen.td
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import sys


WORD_BYTES = 8
MAX_OPERANDS = 3
VALID_OPERAND_TYPES = frozenset({"RegisterOperand", "AddressOperand", "ImmediateOperand", "BasicBlockOperand"})
VALID_BIT_WIDTHS = frozenset({0, 8, 16, 32, 64})
EXPECTED_OPCODE_INCLUDES = (
    "ink/backend/vm/instruction/compare_branch.def",
    "ink/backend/vm/instruction/integer_compare.def",
    "ink/backend/vm/instruction/register_compare_branch.def",
    "ink/backend/vm/instruction/float_compare.def",
)
IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
GPR_KIND_RE = re.compile(r"^R(\d+)$")
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"\s*$')


class GeneratorError(RuntimeError):
    """Raised when a VM .def file violates the expected ISA definition schema."""


@dataclass(frozen=True)
class RegisterSpec:
    kind: str
    encoding: int
    spelling: str


@dataclass(frozen=True)
class FormSpec:
    name: str
    operand_count: int
    register_count: int
    address_count: int
    immediate_count: int
    block_count: int
    byte_size: int


@dataclass(frozen=True)
class OpcodeSpec:
    kind: str
    encoding: int
    spelling: str
    form: str
    bit_width: int
    byte_size: int
    operand_count: int


def strip_comments(text: str, source_name: str) -> str:
    output: list[str] = []
    index = 0
    length = len(text)
    while index < length:
        char = text[index]
        if char == '"':
            output.append(char)
            index += 1
            while index < length:
                inner = text[index]
                output.append(inner)
                index += 1
                if inner == "\\" and index < length:
                    output.append(text[index])
                    index += 1
                elif inner == '"':
                    break
            else:
                raise GeneratorError(f"{source_name}: unterminated string literal")
        elif char == "/" and index + 1 < length and text[index + 1] == "/":
            index += 2
            while index < length and text[index] != "\n":
                index += 1
        elif char == "/" and index + 1 < length and text[index + 1] == "*":
            index += 2
            while index + 1 < length and not (text[index] == "*" and text[index + 1] == "/"):
                if text[index] == "\n":
                    output.append("\n")
                index += 1
            if index + 1 >= length:
                raise GeneratorError(f"{source_name}: unterminated block comment")
            output.append(" ")
            index += 2
        else:
            output.append(char)
            index += 1
    return "".join(output)


def remove_preprocessor_lines(text: str) -> tuple[str, list[str]]:
    cleaned_lines: list[str] = []
    includes: list[str] = []
    continuing_directive = False
    for line in text.splitlines():
        stripped = line.lstrip()
        if continuing_directive or stripped.startswith("#"):
            include_match = INCLUDE_RE.match(line)
            if include_match is not None:
                includes.append(include_match.group(1))
            cleaned_lines.append("")
            continuing_directive = line.rstrip().endswith("\\")
        else:
            cleaned_lines.append(line)
    return "\n".join(cleaned_lines), includes


def split_macro_arguments(argument_text: str, source_name: str, line_number: int) -> list[str]:
    if not argument_text.strip():
        return []
    arguments: list[str] = []
    current: list[str] = []
    depth = 0
    index = 0
    length = len(argument_text)
    while index < length:
        char = argument_text[index]
        if char == '"':
            current.append(char)
            index += 1
            while index < length:
                inner = argument_text[index]
                current.append(inner)
                index += 1
                if inner == "\\" and index < length:
                    current.append(argument_text[index])
                    index += 1
                elif inner == '"':
                    break
            else:
                raise GeneratorError(f"{source_name}:{line_number}: unterminated string literal in macro argument")
        elif char == "(":
            depth += 1
            current.append(char)
            index += 1
        elif char == ")":
            depth -= 1
            if depth < 0:
                raise GeneratorError(f"{source_name}:{line_number}: unbalanced ')' in macro argument")
            current.append(char)
            index += 1
        elif char == "," and depth == 0:
            arguments.append("".join(current).strip())
            current.clear()
            index += 1
        else:
            current.append(char)
            index += 1
    if depth != 0:
        raise GeneratorError(f"{source_name}:{line_number}: unbalanced '(' in macro argument")
    arguments.append("".join(current).strip())
    return arguments


def parse_macro_invocations(path: Path, expected_macros: frozenset[str]) -> tuple[list[tuple[str, list[str], int]], list[str]]:
    raw_text = path.read_text(encoding="utf-8")
    source_name = path.name
    stripped = strip_comments(raw_text, source_name)
    body, includes = remove_preprocessor_lines(stripped)

    invocations: list[tuple[str, list[str], int]] = []
    index = 0
    length = len(body)
    line_number = 1

    while index < length:
        while index < length and body[index].isspace():
            if body[index] == "\n":
                line_number += 1
            index += 1
        if index >= length:
            break

        start_line = line_number
        name_start = index
        while index < length and (body[index].isalnum() or body[index] == "_"):
            index += 1
        macro_name = body[name_start:index]
        if not macro_name:
            raise GeneratorError(f"{source_name}:{start_line}: unexpected character {body[index]!r}")
        if macro_name not in expected_macros:
            raise GeneratorError(f"{source_name}:{start_line}: unexpected macro invocation '{macro_name}'")

        while index < length and body[index].isspace():
            if body[index] == "\n":
                line_number += 1
            index += 1
        if index >= length or body[index] != "(":
            raise GeneratorError(f"{source_name}:{start_line}: expected '(' after '{macro_name}'")

        index += 1
        arg_start = index
        depth = 1
        while index < length and depth > 0:
            char = body[index]
            if char == "\n":
                line_number += 1
                index += 1
            elif char == '"':
                index += 1
                while index < length:
                    inner = body[index]
                    if inner == "\n":
                        line_number += 1
                    index += 1
                    if inner == "\\" and index < length:
                        if body[index] == "\n":
                            line_number += 1
                        index += 1
                    elif inner == '"':
                        break
                else:
                    raise GeneratorError(f"{source_name}:{start_line}: unterminated string literal")
            elif char == "(":
                depth += 1
                index += 1
            elif char == ")":
                depth -= 1
                index += 1
            else:
                index += 1
        if depth != 0:
            raise GeneratorError(f"{source_name}:{start_line}: unterminated invocation of '{macro_name}'")

        argument_text = body[arg_start:index - 1]
        arguments = split_macro_arguments(argument_text, source_name, start_line)
        invocations.append((macro_name, arguments, start_line))

    return invocations, includes


def parse_identifier(value: str, field_name: str, source_name: str, line_number: int) -> str:
    if not IDENTIFIER_RE.match(value):
        raise GeneratorError(f"{source_name}:{line_number}: invalid {field_name} identifier '{value}'")
    return value


def parse_int(value: str, field_name: str, minimum: int, maximum: int, source_name: str, line_number: int) -> int:
    try:
        parsed = int(value, 0)
    except ValueError as exc:
        raise GeneratorError(f"{source_name}:{line_number}: invalid {field_name} integer '{value}'") from exc
    if parsed < minimum or parsed > maximum:
        raise GeneratorError(f"{source_name}:{line_number}: {field_name} value {parsed} out of range [{minimum}, {maximum}]")
    return parsed


def parse_quoted_string(value: str, field_name: str, source_name: str, line_number: int) -> str:
    if len(value) < 2 or value[0] != '"' or value[-1] != '"':
        raise GeneratorError(f"{source_name}:{line_number}: expected quoted {field_name}, got '{value}'")
    inner = value[1:-1]
    if "\\" in inner or '"' in inner or not inner:
        raise GeneratorError(f"{source_name}:{line_number}: unsupported {field_name} literal '{value}'")
    return inner


def parse_registers(register_def: Path) -> list[RegisterSpec]:
    invocations, includes = parse_macro_invocations(register_def, frozenset({"REGISTER"}))
    if includes:
        raise GeneratorError(f"{register_def.name}: unexpected #include directives: {includes}")

    registers: list[RegisterSpec] = []
    seen_kinds: set[str] = set()
    seen_encodings: set[int] = set()
    seen_spellings: set[str] = set()

    for _, args, line_number in invocations:
        if len(args) != 3:
            raise GeneratorError(f"{register_def.name}:{line_number}: REGISTER expects 3 arguments, got {len(args)}")
        kind = parse_identifier(args[0], "register kind", register_def.name, line_number)
        encoding = parse_int(args[1], "register encoding", 0x00, 0xFF, register_def.name, line_number)
        spelling = parse_quoted_string(args[2], "register spelling", register_def.name, line_number)

        if kind in seen_kinds:
            raise GeneratorError(f"{register_def.name}:{line_number}: duplicate register kind '{kind}'")
        if encoding in seen_encodings:
            raise GeneratorError(f"{register_def.name}:{line_number}: duplicate register encoding 0x{encoding:02X}")
        if spelling in seen_spellings:
            raise GeneratorError(f"{register_def.name}:{line_number}: duplicate register spelling '{spelling}'")

        seen_kinds.add(kind)
        seen_encodings.add(encoding)
        seen_spellings.add(spelling)
        registers.append(RegisterSpec(kind=kind, encoding=encoding, spelling=spelling))

    if not registers:
        raise GeneratorError(f"{register_def.name}: no REGISTER entries found")
    return registers


def parse_opcodes(vm_instruction_dir: Path) -> list[OpcodeSpec]:
    opcode_def = vm_instruction_dir / "opcode.def"
    invocations, includes = parse_macro_invocations(opcode_def, frozenset({"OPCODE_FORM", "OPCODE"}))

    if tuple(includes) != EXPECTED_OPCODE_INCLUDES:
        raise GeneratorError(f"{opcode_def.name}: expected #include list {list(EXPECTED_OPCODE_INCLUDES)}, got {includes}")

    forms: dict[str, FormSpec] = {}
    opcodes: list[OpcodeSpec] = []
    seen_kinds: set[str] = set()
    seen_encodings: set[int] = set()

    def register_opcode(kind: str, encoding: int, spelling: str, form_name: str, bit_width: int, source_name: str, line_number: int) -> None:
        if form_name not in forms:
            raise GeneratorError(f"{source_name}:{line_number}: unknown instruction form '{form_name}'")
        if bit_width not in VALID_BIT_WIDTHS:
            raise GeneratorError(f"{source_name}:{line_number}: unsupported bit width {bit_width}")
        if kind in seen_kinds:
            raise GeneratorError(f"{source_name}:{line_number}: duplicate opcode kind '{kind}'")
        if encoding in seen_encodings:
            raise GeneratorError(f"{source_name}:{line_number}: duplicate opcode encoding 0x{encoding:04X}")
        form = forms[form_name]
        seen_kinds.add(kind)
        seen_encodings.add(encoding)
        opcodes.append(OpcodeSpec(kind=kind, encoding=encoding, spelling=spelling, form=form_name, bit_width=bit_width, byte_size=form.byte_size, operand_count=form.operand_count))

    for macro_name, args, line_number in invocations:
        if macro_name == "OPCODE_FORM":
            if not args:
                raise GeneratorError(f"{opcode_def.name}:{line_number}: OPCODE_FORM requires a form name")
            form_name = parse_identifier(args[0], "form name", opcode_def.name, line_number)
            if form_name in forms:
                raise GeneratorError(f"{opcode_def.name}:{line_number}: duplicate OPCODE_FORM '{form_name}'")
            operand_types = args[1:]
            if len(operand_types) > MAX_OPERANDS:
                raise GeneratorError(f"{opcode_def.name}:{line_number}: form '{form_name}' exceeds {MAX_OPERANDS} operands")
            for operand_type in operand_types:
                if operand_type not in VALID_OPERAND_TYPES:
                    raise GeneratorError(f"{opcode_def.name}:{line_number}: unknown operand type '{operand_type}' in form '{form_name}'")
            forms[form_name] = FormSpec(
                name=form_name,
                operand_count=len(operand_types),
                register_count=sum(1 for item in operand_types if item == "RegisterOperand"),
                address_count=sum(1 for item in operand_types if item == "AddressOperand"),
                immediate_count=sum(1 for item in operand_types if item == "ImmediateOperand"),
                block_count=sum(1 for item in operand_types if item == "BasicBlockOperand"),
                byte_size=(len(operand_types) + 1) * WORD_BYTES,
            )
        else:
            if len(args) != 10:
                raise GeneratorError(f"{opcode_def.name}:{line_number}: OPCODE expects 10 arguments, got {len(args)}")
            kind = parse_identifier(args[0], "opcode kind", opcode_def.name, line_number)
            encoding = parse_int(args[1], "opcode encoding", 0x0001, 0xFFFF, opcode_def.name, line_number)
            spelling = parse_quoted_string(args[2], "opcode spelling", opcode_def.name, line_number)
            form_name = parse_identifier(args[3], "opcode form", opcode_def.name, line_number)
            bit_width = parse_int(args[4], "bit width", 0, 64, opcode_def.name, line_number)
            register_count = parse_int(args[5], "register count", 0, MAX_OPERANDS, opcode_def.name, line_number)
            address_count = parse_int(args[6], "address count", 0, MAX_OPERANDS, opcode_def.name, line_number)
            immediate_count = parse_int(args[7], "immediate count", 0, MAX_OPERANDS, opcode_def.name, line_number)
            if form_name not in forms:
                raise GeneratorError(f"{opcode_def.name}:{line_number}: unknown instruction form '{form_name}'")
            form = forms[form_name]
            if (register_count, address_count, immediate_count) != (form.register_count, form.address_count, form.immediate_count):
                raise GeneratorError(f"{opcode_def.name}:{line_number}: operand counts for '{kind}' do not match form '{form_name}'")
            register_opcode(kind, encoding, spelling, form_name, bit_width, opcode_def.name, line_number)

    compare_branch_def = vm_instruction_dir / "compare_branch.def"
    cb_invocations, cb_includes = parse_macro_invocations(compare_branch_def, frozenset({"COMPARE_BRANCH"}))
    if cb_includes:
        raise GeneratorError(f"{compare_branch_def.name}: unexpected #include directives: {cb_includes}")
    for _, args, line_number in cb_invocations:
        if len(args) != 7:
            raise GeneratorError(f"{compare_branch_def.name}:{line_number}: COMPARE_BRANCH expects 7 arguments, got {len(args)}")
        kind = parse_identifier(args[0], "opcode kind", compare_branch_def.name, line_number)
        encoding = parse_int(args[1], "opcode encoding", 0x0001, 0xFFFF, compare_branch_def.name, line_number)
        spelling = parse_quoted_string(args[2], "opcode spelling", compare_branch_def.name, line_number)
        form_name = parse_identifier(args[3], "opcode form", compare_branch_def.name, line_number)
        bit_width = parse_int(args[4], "bit width", 8, 64, compare_branch_def.name, line_number)
        if form_name not in {"Mmb", "Mib"}:
            raise GeneratorError(f"{compare_branch_def.name}:{line_number}: unexpected compare-branch form '{form_name}'")
        register_opcode(kind, encoding, spelling, form_name, bit_width, compare_branch_def.name, line_number)

    integer_compare_def = vm_instruction_dir / "integer_compare.def"
    ic_invocations, ic_includes = parse_macro_invocations(integer_compare_def, frozenset({"INTEGER_COMPARE"}))
    if ic_includes:
        raise GeneratorError(f"{integer_compare_def.name}: unexpected #include directives: {ic_includes}")
    for _, args, line_number in ic_invocations:
        if len(args) != 6:
            raise GeneratorError(f"{integer_compare_def.name}:{line_number}: INTEGER_COMPARE expects 6 arguments, got {len(args)}")
        kind = parse_identifier(args[0], "opcode kind", integer_compare_def.name, line_number)
        encoding = parse_int(args[1], "opcode encoding", 0x0001, 0xFFFF, integer_compare_def.name, line_number)
        spelling = parse_quoted_string(args[2], "opcode spelling", integer_compare_def.name, line_number)
        form_name = parse_identifier(args[3], "opcode form", integer_compare_def.name, line_number)
        if form_name not in {"Rrr", "Rri"}:
            raise GeneratorError(f"{integer_compare_def.name}:{line_number}: unexpected integer-compare form '{form_name}'")
        register_opcode(kind, encoding, spelling, form_name, 64, integer_compare_def.name, line_number)

    register_compare_branch_def = vm_instruction_dir / "register_compare_branch.def"
    rcb_invocations, rcb_includes = parse_macro_invocations(register_compare_branch_def, frozenset({"REGISTER_COMPARE_BRANCH"}))
    if rcb_includes:
        raise GeneratorError(f"{register_compare_branch_def.name}: unexpected #include directives: {rcb_includes}")
    for _, args, line_number in rcb_invocations:
        if len(args) != 6:
            raise GeneratorError(f"{register_compare_branch_def.name}:{line_number}: REGISTER_COMPARE_BRANCH expects 6 arguments, got {len(args)}")
        kind = parse_identifier(args[0], "opcode kind", register_compare_branch_def.name, line_number)
        encoding = parse_int(args[1], "opcode encoding", 0x0001, 0xFFFF, register_compare_branch_def.name, line_number)
        spelling = parse_quoted_string(args[2], "opcode spelling", register_compare_branch_def.name, line_number)
        form_name = parse_identifier(args[3], "opcode form", register_compare_branch_def.name, line_number)
        if form_name not in {"Rrb", "Rib"}:
            raise GeneratorError(f"{register_compare_branch_def.name}:{line_number}: unexpected register-compare-branch form '{form_name}'")
        register_opcode(kind, encoding, spelling, form_name, 64, register_compare_branch_def.name, line_number)

    float_compare_def = vm_instruction_dir / "float_compare.def"
    fc_invocations, fc_includes = parse_macro_invocations(float_compare_def, frozenset({"FLOAT_COMPARE"}))
    if fc_includes:
        raise GeneratorError(f"{float_compare_def.name}: unexpected #include directives: {fc_includes}")
    for _, args, line_number in fc_invocations:
        if len(args) != 4:
            raise GeneratorError(f"{float_compare_def.name}:{line_number}: FLOAT_COMPARE expects 4 arguments, got {len(args)}")
        kind = parse_identifier(args[0], "opcode kind", float_compare_def.name, line_number)
        encoding = parse_int(args[1], "opcode encoding", 0x0001, 0xFFFF, float_compare_def.name, line_number)
        spelling = parse_quoted_string(args[2], "opcode spelling", float_compare_def.name, line_number)
        register_opcode(kind, encoding, spelling, "Rrr", 64, float_compare_def.name, line_number)

    opcodes.sort(key=lambda item: item.encoding)
    if not opcodes:
        raise GeneratorError(f"{opcode_def.name}: no OPCODE entries found")
    return opcodes


def render_register_td(registers: list[RegisterSpec]) -> str:
    gpr_indices: list[int] = []
    for reg in registers:
        match = GPR_KIND_RE.match(reg.kind)
        if match is not None:
            gpr_indices.append(int(match.group(1)))

    if not gpr_indices or gpr_indices != list(range(len(gpr_indices))):
        raise GeneratorError(f"register.def: expected contiguous general-purpose registers R0..Rn, found indices {gpr_indices}")

    lines = [
        "// Generated by cmake/generate_inkvm_tablegen.py; do not edit.",
        "// Source: src/include/ink/backend/vm/instruction/register.def",
        "",
        "class InkVMReg<bits<8> Encoding, string Name> : Register<Name> {",
        '  let Namespace = "InkVM";',
        "  let HWEncoding{7-0} = Encoding;",
        "}",
        "",
    ]
    for reg in registers:
        lines.append(f'def {reg.kind} : InkVMReg<0x{reg.encoding:02X}, "{reg.spelling}">, DwarfRegNum<[{reg.encoding}]>;')
    lines.append("")
    lines.append(f"defvar InkVMGPRMaxIndex = {gpr_indices[-1]};")
    return "\n".join(lines) + "\n"


def render_opcode_td(opcodes: list[OpcodeSpec]) -> str:
    lines = [
        "// Generated by cmake/generate_inkvm_tablegen.py; do not edit.",
        "// Sources: src/include/ink/backend/vm/instruction/{opcode,compare_branch,integer_compare,register_compare_branch,float_compare}.def",
        "",
        "class InkVMOpcodeMeta<bits<16> encoding, string mnemonic, int size, int operandCount, int bitWidth> {",
        "  bits<16> Encoding = encoding;",
        "  string Mnemonic = mnemonic;",
        "  int Size = size;",
        "  int OperandCount = operandCount;",
        "  int BitWidth = bitWidth;",
        "}",
        "",
    ]
    for op in opcodes:
        lines.append(f'def InkVMOp_{op.kind} : InkVMOpcodeMeta<0x{op.encoding:04X}, "{op.spelling}", {op.byte_size}, {op.operand_count}, {op.bit_width}>;')
    return "\n".join(lines) + "\n"


def write_output(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vm-instruction-dir", type=Path, required=True, help="Path to src/include/ink/backend/vm/instruction")
    parser.add_argument("--register-td", type=Path, required=True, help="Output path for InkVMRegisterInfoGen.td")
    parser.add_argument("--opcode-td", type=Path, required=True, help="Output path for InkVMOpcodeMetaGen.td")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        registers = parse_registers(arguments.vm_instruction_dir / "register.def")
        opcodes = parse_opcodes(arguments.vm_instruction_dir)
        write_output(arguments.register_td, render_register_td(registers))
        write_output(arguments.opcode_td, render_opcode_td(opcodes))
        return 0
    except (GeneratorError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
