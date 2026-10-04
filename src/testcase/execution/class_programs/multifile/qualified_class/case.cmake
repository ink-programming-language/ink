# Module-qualified class construction and by-value calls remain valid after source files are removed.
expect_bytecode_result("app#main" 42)
