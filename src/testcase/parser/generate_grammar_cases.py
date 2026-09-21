#!/usr/bin/env python3
"""Reproduce the fixed grammar audit corpus without running the Ink parser."""

import argparse
from collections import deque
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import random
import re
import sys


SEED = 20260921
COMBINATION_COUNT = 2400
TERMINALS = {
    "IDENTIFIER": "x",
    "INTEGER_LITERAL": "1",
    "FLOAT_LITERAL": "1.5",
    "CHAR_LITERAL": "'c'",
    "STRING_LITERAL": '"s"',
    "EOF": "",
}
CONTEXTS = (
    ("stmt", "", ""),
    ("expr", "", ";"),
    ("expr", "var v = ", ";"),
    ("type_expr", "var v: ", ";"),
    ("pattern", "match(x) { ", " => 0 };"),
    ("binding_pattern", "var ", " = xs;"),
    ("decl", "", ""),
    ("for_header", "for (", ") {}"),
)
TOKEN = re.compile(r'"(?:[^"\\]|\\.)*"|::=|[A-Za-z_][A-Za-z_0-9]*|[{}\[\]();|]')


@dataclass
class Node:
    kind: str
    value: object
    number: int
    rule: str


@dataclass
class Case:
    name: str
    rule: str
    derivation: str
    source: str


class Grammar:
    def __init__(self, text):
        self.tokens = []
        end = 0
        uncommented = re.sub(r"//[^\n]*", "", text)
        for match in TOKEN.finditer(uncommented):
            if uncommented[end:match.start()].strip():
                raise ValueError(f"Unrecognized grammar text at offset {end}")
            self.tokens.append(match.group())
            end = match.end()
        if uncommented[end:].strip():
            raise ValueError(f"Unrecognized grammar text at offset {end}")
        self.cursor = 0
        self.node_count = 0
        self.rules = {}
        self.current_rule = ""
        while self.cursor < len(self.tokens):
            self.current_rule = self.consume()
            if self.current_rule in self.rules:
                raise ValueError(f"Duplicate rule: {self.current_rule}")
            self.consume("::=")
            self.rules[self.current_rule] = self.parse_alternatives(";")
            self.consume(";")
        if "module" not in self.rules:
            raise ValueError("The grammar must define module")
        self.costs = {name: float("inf") for name in self.rules}
        while True:
            updated = {name: self.cost(rule) for name, rule in self.rules.items()}
            if updated == self.costs:
                break
            self.costs = updated
        if any(value == float("inf") for value in self.costs.values()):
            raise ValueError("Every grammar rule must have a finite derivation")

    def consume(self, expected=None):
        if self.cursor >= len(self.tokens):
            raise ValueError(f"Unexpected grammar EOF while reading {self.current_rule}")
        value = self.tokens[self.cursor]
        if expected is not None and value != expected:
            raise ValueError(f"Expected {expected}, found {value} in {self.current_rule}")
        self.cursor += 1
        return value

    def node(self, kind, value):
        self.node_count += 1
        return Node(kind, value, self.node_count, self.current_rule)

    def parse_alternatives(self, end):
        choices = []
        parts = []
        while self.cursor < len(self.tokens) and self.tokens[self.cursor] != end:
            token = self.consume()
            if token == "|":
                choices.append(self.node("seq", parts))
                parts = []
            elif token in ("(", "[", "{"):
                close = {"(": ")", "[": "]", "{": "}"}[token]
                inner = self.parse_alternatives(close)
                self.consume(close)
                parts.append(inner if token == "(" else self.node("opt" if token == "[" else "rep", inner))
            elif token.startswith('"'):
                parts.append(self.node("lit", json.loads(token)))
            elif re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", token):
                parts.append(self.node("ref", token))
            else:
                raise ValueError(f"Unexpected token {token} in {self.current_rule}")
        choices.append(self.node("seq", parts))
        return self.node("alt", choices)

    def cost(self, node):
        if node.kind == "lit":
            return 1
        if node.kind == "ref":
            if node.value in TERMINALS:
                return 1
            if node.value not in self.rules:
                raise ValueError(f"Undefined rule: {node.value}")
            return self.costs[node.value]
        if node.kind == "seq":
            return sum(self.cost(part) for part in node.value)
        if node.kind == "alt":
            return min(self.cost(part) for part in node.value)
        return 0

    def children(self, node):
        if node.kind == "ref":
            return [self.rules[node.value]] if node.value in self.rules else []
        if node.kind in ("seq", "alt"):
            return node.value
        if node.kind in ("opt", "rep"):
            return [node.value]
        return []


class Generator:
    def __init__(self, grammar):
        self.grammar = grammar
        self.random = random.Random(SEED)
        self.fuel = 0

    def expand(self, node):
        kind, value = node.kind, node.value
        if kind == "lit":
            return [value]
        if kind == "ref":
            if value in TERMINALS:
                return [TERMINALS[value]] if TERMINALS[value] else []
            return self.expand(self.grammar.rules[value])
        if kind == "seq":
            return [token for part in value for token in self.expand(part)]
        if kind == "alt":
            if len(value) == 1:
                return self.expand(value[0])
            chosen = self.random.randrange(len(value)) if self.fuel > 0 else min(range(len(value)), key=lambda index: self.grammar.cost(value[index]))
            self.fuel -= 1
            return self.expand(value[chosen])
        count = 0 if self.fuel <= 0 else (int(self.random.random() < 0.4) if kind == "opt" else self.random.choices([0, 1, 2], [0.65, 0.30, 0.05])[0])
        self.fuel -= 1
        return [token for _ in range(count) for token in self.expand(value)]

    def witness(self, node, path, selection):
        if not path:
            self.fuel = 0
            if node.kind == "alt":
                return self.expand(node.value[selection])
            return [token for _ in range(selection) for token in self.expand(node.value)]
        if node.kind == "seq":
            result = []
            for index, part in enumerate(node.value):
                self.fuel = 0
                result.extend(self.witness(part, path[1:], selection) if index == path[0] else self.expand(part))
            return result
        return self.witness(self.grammar.children(node)[path[0]], path[1:], selection)

    def branches(self):
        cases = []
        root = self.grammar.rules["module"]
        queue = deque([(root, [])])
        seen = set()
        while queue:
            node, path = queue.popleft()
            if node.number in seen:
                continue
            seen.add(node.number)
            if node.kind in ("alt", "opt", "rep"):
                selections = range(len(node.value)) if node.kind == "alt" else range(3 if node.kind == "rep" else 2)
                for selection in selections:
                    source = " ".join(self.witness(root, path, selection))
                    derivation = f"node={node.number} {node.kind}={selection}"
                    cases.append(Case(f"Case{len(cases):04d}_{node.rule}", node.rule, derivation, source))
            for index, child in enumerate(self.grammar.children(node)):
                queue.append((child, path + [index]))
        unreachable = [name for name, rule in self.grammar.rules.items() if rule.number not in seen]
        if unreachable:
            raise ValueError(f"Rules unreachable from module: {', '.join(unreachable)}")
        return cases

    def combinations(self):
        cases = []
        for index in range(COMBINATION_COUNT):
            start, prefix, suffix = CONTEXTS[index % len(CONTEXTS)]
            budget = self.random.choice([8, 20, 50, 100])
            self.fuel = budget
            source = prefix + " ".join(self.expand(self.grammar.rules[start])) + suffix
            cases.append(Case(f"Case{index:04d}_{start}", start, f"seed={SEED} case={index} fuel={budget} context={index % len(CONTEXTS)}", source))
        return cases


def render_cases(grammar_text, grammar, branches, combinations):
    digest = hashlib.sha256(grammar_text.encode("utf-8")).hexdigest()
    lines = [
        "// Generated by src/testcase/parser/generate_grammar_cases.py; do not edit by hand.",
        f"// Grammar SHA-256 (UTF-8, normalized LF): {digest}",
        f"// Seed: {SEED}; reachable rules: {len(grammar.rules)}; branches: {len(branches)}; combinations: {len(combinations)}.",
        "// Each entry preserves one original audit case, including duplicate source texts and empty modules.",
        "",
    ]
    for name, cases in (("GrammarBranchCases", branches), ("GrammarCombinationCases", combinations)):
        lines.append(f"    constexpr GrammarCase {name}[] = {{")
        for case in cases:
            if ')INK"' in case.source or "\n" in case.source or "\r" in case.source:
                raise ValueError(f"Source requires a different C++ raw-string delimiter: {case.name}")
            fields = ", ".join(json.dumps(value, ensure_ascii=True) for value in (case.name, case.rule, case.derivation))
            lines.append(f'        {{{fields}, R"INK({case.source})INK"}},')
        lines.extend(["    };", ""])
    return "\n".join(lines)


def main():
    directory = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--grammar", type=Path, default=directory.parents[2] / "docs/Ink-grammar-Rules.bnf")
    parser.add_argument("--output", type=Path, default=directory / "corpus/grammar_cases.inc")
    parser.add_argument("--check", action="store_true", help="Check the committed corpus without changing it.")
    arguments = parser.parse_args()
    grammar_text = arguments.grammar.read_text(encoding="utf-8-sig")
    grammar = Grammar(grammar_text)
    branches = Generator(grammar).branches()
    combinations = Generator(grammar).combinations()
    content = render_cases(grammar_text, grammar, branches, combinations)
    if arguments.check:
        if not arguments.output.is_file() or arguments.output.read_text(encoding="utf-8") != content:
            print(f"Grammar corpus is stale or missing: {arguments.output}. Run {Path(__file__).name} to regenerate it.", file=sys.stderr)
            return 1
    else:
        arguments.output.parent.mkdir(parents=True, exist_ok=True)
        arguments.output.write_bytes(content.encode("utf-8"))
    print(f"{'Verified' if arguments.check else 'Generated'} {len(branches)} branch cases and {len(combinations)} combination cases across {len(grammar.rules)} reachable rules: {arguments.output}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError) as error:
        print(f"Grammar corpus generation failed: {error}", file=sys.stderr)
        sys.exit(1)
