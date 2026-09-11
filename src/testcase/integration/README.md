# CodeContests integration fixtures

This directory contains 2,533 problems selected from
`D:/github/Seres/src/testcase/Integration/CodeContests`. The source corpus is a
subset of the Google DeepMind CodeContests `train` split at revision
`802411c3010cb00d1b05bad57ca77365a3c699d6`.

Each problem directory contains the selected `input.txt` and `output.txt` test
pair and an Ink port in `solution.ink`. Only problems whose selected accepted
reference solution is C++ are included; the 467 Python-reference problems from
the source selection are intentionally omitted.

These ports are lexical and syntactic integration fixtures. They retain the
source control flow and use placeholder names such as `dynamic` and `cpp_*`
for types, input, output, containers, and library operations. The test suite
does not type-check or execute these ports, and a successful parse does not
establish equivalent runtime behavior. The retained input/output pairs are
intended for executable integration tests once those mappings are implemented.

All 2,533 `solution.ink` files use `docs/grammar.bnf` and `docs/lexer.bnf`.
The grammar migration adds explicit binding types and function return types,
uses `cpp_uninitialized()` for former declarations without initializers, adds
types to `for` bindings, changes expression-level `if/else` to `?:`, changes
enum entries to `enum_field`, and changes `this` to `self`. Names that became
keywords receive a `cpp_` prefix. The six former destructor declarations are
ordinary `cpp_destruct_*` functions that retain their bodies for syntax
coverage; this does not introduce destructor semantics into the language.

The upstream dataset card declares CC BY 4.0 for the dataset and documents the
licenses and provenance of the incorporated contest sources. Retain this
attribution and review the upstream terms before redistributing these fixtures:
<https://github.com/google-deepmind/code_contests>.

`manifest.jsonl` preserves the original per-problem dataset row, source,
difficulty, selected reference language, and test-pair provenance.
