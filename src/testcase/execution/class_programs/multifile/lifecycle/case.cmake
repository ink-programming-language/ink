# Construction and implicit cleanup import both lifecycle methods across saved object boundaries.
expect_bytecode_result("app#main" 42)
