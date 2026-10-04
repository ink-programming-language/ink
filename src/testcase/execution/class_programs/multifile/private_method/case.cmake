# Method access checks use the defining class even across independently compiled modules.
expect_bytecode_source_error("app" "INK-S0057")
