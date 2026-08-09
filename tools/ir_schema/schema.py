from dataclasses import dataclass


@dataclass(frozen=True)
class Opcode:
    name: str
    mnemonic: str
    payload: str
    min_operands: int
    max_operands: int
    min_results: int
    max_results: int
    successors: int
    terminator: bool
    effects: tuple[str, ...]
    stages: tuple[str, ...] = ("Staged", "Closed")


@dataclass(frozen=True)
class PlanOpcode:
    name: str
    mnemonic: str
    min_inputs: int
    max_inputs: int
    result_count: int
    stages: tuple[str, ...] = ("Staged",)


VARIADIC = 255


OPCODES = (
    Opcode("ConstInt", "const.int", "Constant", 0, 0, 1, 1, 0, False, ("Pure",)),
    Opcode("ConstBool", "const.bool", "Constant", 0, 0, 1, 1, 0, False, ("Pure",)),
    Opcode("IntAdd", "arith.add", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("IntSub", "arith.sub", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("IntMul", "arith.mul", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("IntNeg", "arith.neg", "None", 1, 1, 1, 1, 0, False, ("Pure",)),
    Opcode("IntSignedDiv", "pdb.sdiv", "None", 2, 2, 1, 1, 0, False, ("TargetDependent", "PdbBoundary", "MayTrap")),
    Opcode("IntUnsignedDiv", "pdb.udiv", "None", 2, 2, 1, 1, 0, False, ("TargetDependent", "PdbBoundary", "MayTrap")),
    Opcode("IntSignedRem", "pdb.srem", "None", 2, 2, 1, 1, 0, False, ("TargetDependent", "PdbBoundary", "MayTrap")),
    Opcode("IntUnsignedRem", "pdb.urem", "None", 2, 2, 1, 1, 0, False, ("TargetDependent", "PdbBoundary", "MayTrap")),
    Opcode("IntAnd", "arith.and", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("IntOr", "arith.or", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("IntXor", "arith.xor", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("IntCompare", "arith.cmp", "Compare", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("BoolNot", "bool.not", "None", 1, 1, 1, 1, 0, False, ("Pure",)),
    Opcode("BoolAnd", "bool.and", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("BoolOr", "bool.or", "None", 2, 2, 1, 1, 0, False, ("Pure",)),
    Opcode("CastInt", "cast.int", "Type", 1, 1, 1, 1, 0, False, ("Pure",)),
    Opcode("Alloca", "mem.alloca", "Type", 0, 0, 1, 1, 0, False, ("Allocate",)),
    Opcode("Load", "mem.load", "None", 1, 1, 1, 1, 0, False, ("ReadMemory",)),
    Opcode("Store", "mem.store", "None", 2, 2, 0, 0, 0, False, ("WriteMemory",)),
    Opcode("DirectCall", "call.direct", "DirectCall", 0, VARIADIC, 0, 1, 0, False, ("Call",)),
    Opcode("Branch", "cf.br", "None", 0, 0, 0, 0, 1, True, ("Control",)),
    Opcode("CondBranch", "cf.cond_br", "None", 1, 1, 0, 0, 2, True, ("Control",)),
    Opcode("Return", "cf.return", "None", 0, 1, 0, 0, 0, True, ("Control",)),
    Opcode("Unreachable", "cf.unreachable", "None", 0, 0, 0, 0, 0, True, ("Control",)),
    Opcode("Trap", "rt.trap", "Trap", 0, 0, 0, 0, 0, True, ("Runtime", "MayTrap", "Control")),
)


PLAN_OPCODES = (
    PlanOpcode("ForceValue", "stage.force_value", 1, 1, 1),
)
