# Importing a class does not grant access to its private fields.
expect_bytecode_source_error("app" "INK-S0057")
