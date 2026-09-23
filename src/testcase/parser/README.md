# Parser 语法、恢复与恶意输入测试

所有用例都经过实际的 `tokenizer::tokenize -> parser::parse` 链路。`ParserTest::read` 统一验证 AST 的必需子节点及范围、诊断的来源和字节范围，以及恢复记录的插入点、实际 token 和跳过范围。测试不使用 C++ 异常捕获；断言失败、进程崩溃和超时均视为失败。

| 文件 | 覆盖内容 |
| --- | --- |
| `grammar_corpus_test.cpp` / `corpus/grammar_cases.inc` | 2026-09-21 BNF 审查的全部 539 个逐分支样例和 2,400 个固定种子组合样例，共 2,939 个独立参数化测试 |
| `ast_serialization_test.cpp` | 全部 2,939 个语法样例与 66 种节点的二进制往返、Token/payload/恢复信息、独立生命周期、深树、损坏输入和资源限制、ICE 诊断及首次失败去重；格式及接口见 [AST 序列化说明](../../../docs/Ink-AST-Serialization.md) |
| `recovery_test.cpp` | 原有设计示例、恢复边界、长链、固定种子随机 token 流 |
| `recovery_contracts_test.cpp` | 精确的插入与删除位置、成对定界符同步、后续声明保留、表达式和模式的 missing/error 节点、参数列表、条件与 match 分支、else/switch 归属、for-in 推测回滚、声明族及 EOF、诊断风暴、嵌套/工作/分配预算、取消与复用 |
| `unicode_security_test.cpp` | 非法 UTF-8、完整枚举非法连续字节、标量边界与截断、过长编码伪装 ASCII 语法、NUL/BOM、UTF-16/32 输入、双向控制与不可见字符、Unicode 空白和标点混淆、NFC 与组合字符、非法转义、原始字节偏移、字节变异与固定种子随机输入 |
| `adversarial_process_test.cmake` | 通过 `ink_parse` 的文件和 stdin 入口分别运行 21 个样例，严格检查退出码、诊断、AST 及文件/stdin 输出一致性 |

Unicode 预期遵守 `docs/Ink-Lexical-Rules.md`：非法 UTF-8、直接 NUL 和文件开头 BOM 在任何区域都拒绝；字符串和注释内允许的控制符、非字符等内容不能被误当成语法。标识符按 NFC 规范化，不进行 NFKC 或同形字符折叠。Unicode 错误覆盖 13 个源码区域，包含普通/原始/三引号字面量、字符字面量、两种注释和多字节换行前缀。

变异测试先确认种子程序有效，再逐 token 删除、截断和插入定界符；另对 Unicode 程序逐字节删除、截断和替换。随机字节及 Unicode 样例使用固定种子 `0x554E4943`，失败日志保留样例序号、字节位置或区域，便于复现。这些有限语料用于持续回归，不代表已穷举所有输入。

恢复树范围的回归样例包括 `match(x) { A | => 1 };`，以及达到深度限制的 `if(x) if(x) ...`：即使缺失节点位于空白之后，其插入点也必须包含在祖先的源码范围内。

在已配置的构建目录运行：

```sh
cmake --build cmake-build-debug --target ink_tests
ctest --test-dir cmake-build-debug --output-on-failure
```

`ink_tests` 是统一的 GoogleTest 可执行文件，包含 tokenizer、parser 和 AST 的全部单元测试。在 IDE 中运行 `ink_tests` 即可执行所有这些用例；只运行 parser/AST 用例时，使用 GoogleTest 过滤器 `--gtest_filter="Parser*.*:AST*.*:*/Parser*.*"`。CTest 另外注册三个 CLI 进程测试，`run_all_tests` 会构建 `ink_tests` 及它依赖的两个 CLI 工具，再执行完整 CTest 回归。

只运行新增恶意输入测试：

```sh
ctest --test-dir cmake-build-debug -R "ParserTest.Recovery|ParserUnicodeTest|ParserAdversarialProcessTest" --output-on-failure
```

通过 CTest 运行时，每个 parser 单元测试最多 30 秒。进程测试中的每次 CLI 调用最多 10 秒，整个进程测试最多 120 秒。直接运行 GoogleTest 可用 `--gtest_filter` 复现，但不会获得 CTest 的外部超时保护。

## BNF 固定语料

`corpus/grammar_cases.inc` 固定保存这次审查的全部 2,939 个样例，保留原始顺序和编号，包括不同推导得到的重复源码及合法的空模块。每个样例单独注册到 GoogleTest 和 CTest；失败信息包含规则名、分支节点与选择，或随机种子、样例编号和生成预算，以及完整输入。

每例都断言解析成功、`ParseStatus::Completed`、诊断为空、恢复记录为空，并经过 `ParserTest::read` 的 AST 必需子节点、源码范围和元数据校验。固定语料不依赖构建目录中的审查 JSON 文件，运行测试时也不需要 Python 或重新生成随机输入。

`generate_grammar_cases.py` 直接读取 `docs/Ink-grammar-Rules.bnf`，以 `module` 为入口生成分支样例，并使用种子 `20260921` 在 8 种上下文中生成组合样例；不会通过 parser 的接受结果筛选或删除样例。当前分支语料触达全部 121 条产生式，并覆盖产生式选择、可选项的有无、重复项的 0/1/2 次展开。语料文件记录规范化为 LF 后的文法 SHA-256。

从仓库根目录检查语料是否与当前 BNF 及生成器一致：

```sh
python src/testcase/parser/generate_grammar_cases.py --check
```

有意修改 BNF 或生成规则后，运行 `python src/testcase/parser/generate_grammar_cases.py` 更新语料，检查生成 diff，再重新构建和运行测试。正常 C++ 构建直接使用已保存的语料，不增加 Python 构建依赖。

只运行这 2,939 个语法样例，或单独复现一个组合样例：

```sh
ctest --test-dir cmake-build-debug -R "^Grammar(Branches|Combinations)/ParserGrammarCorpusTest\." --output-on-failure --parallel 4
ctest --test-dir cmake-build-debug -R "^GrammarCombinations/ParserGrammarCorpusTest\.AcceptsValidGrammar/Case0001_expr$" --output-on-failure
```

这些样例验证语法接受性和 AST 结构契约；不代替针对运算符结合性、特定 AST 字段、非法语法和错误恢复的手写测试。
