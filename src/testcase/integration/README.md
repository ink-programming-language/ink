# CodeContests integration fixtures

This directory contains 2,533 problems selected from
`D:/github/Seres/src/testcase/Integration/CodeContests`. The source corpus is a
subset of the Google DeepMind CodeContests `train` split at revision
`802411c3010cb00d1b05bad57ca77365a3c699d6`.

Each problem directory contains the selected `input.txt` and `output.txt` test
pair and an Ink port in `solution.ink`. Only problems whose selected accepted
reference solution is C++ are included; the 467 Python-reference problems from
the source selection are intentionally omitted.

Ink currently provides a tokenizer and parser, but no standard library,
compiler, or program runner. Consequently, the Ink ports preserve the source
algorithms and use ordinary call syntax for input, output, containers, and
library operations, but can currently be checked only for lexical and syntactic
validity. The retained test pairs are intended for executable integration tests
once those runtime facilities exist.

The upstream dataset card declares CC BY 4.0 for the dataset and documents the
licenses and provenance of the incorporated contest sources. Retain this
attribution and review the upstream terms before redistributing these fixtures:
<https://github.com/google-deepmind/code_contests>.

`manifest.jsonl` preserves the original per-problem dataset row, source,
difficulty, selected reference language, and test-pair provenance.
