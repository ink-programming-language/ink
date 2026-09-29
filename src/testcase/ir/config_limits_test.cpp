#include "../core/environment_test_support.h"

#include "ink/ir/ir_builder.h"
#include "ink/ir/module/module_serialization.h"

namespace ink::ir::test
{
  // Each module archive limit snapshots its own environment override and permits later caller assignment.
  TEST(IRConfigLimitsTest, ModuleArchiveDefaultsFollowEnvironment)
  {
    struct ConfigCase
    {
        const char *EnvironmentName;
        std::size_t ModuleArchiveLimits::*Field;
        std::size_t DefaultValue;
    };
    const ConfigCase Cases[] = {
        {"INK_MODULE_ARCHIVE_MAX_BYTES", &ModuleArchiveLimits::MaxArchiveBytes, 64 * 1024 * 1024},
        {"INK_MODULE_ARCHIVE_MAX_OBJECTS", &ModuleArchiveLimits::MaxObjects, 1024 * 1024},
        {"INK_MODULE_ARCHIVE_MAX_FIELDS", &ModuleArchiveLimits::MaxFields, 1024 * 1024},
        {"INK_MODULE_ARCHIVE_MAX_STRING_BYTES", &ModuleArchiveLimits::MaxStringBytes, 16 * 1024 * 1024},
        {"INK_MODULE_ARCHIVE_MAX_ALLOCATION_BYTES", &ModuleArchiveLimits::MaxAllocationBytes, 256 * 1024 * 1024},
        {"INK_MODULE_ARCHIVE_MAX_NESTING_DEPTH", &ModuleArchiveLimits::MaxNestingDepth, 256},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.EnvironmentName);
      core::test::ScopedEnvironmentVariable Environment(Entry.EnvironmentName);
      ASSERT_TRUE(Environment.set(nullptr));
      EXPECT_EQ(ModuleArchiveLimits{}.*Entry.Field, Entry.DefaultValue);
      ASSERT_TRUE(Environment.set("17"));
      ModuleArchiveLimits Snapshot;
      EXPECT_EQ(Snapshot.*Entry.Field, 17U);
      ASSERT_TRUE(Environment.set("29"));
      EXPECT_EQ(ModuleArchiveLimits{}.*Entry.Field, 29U);
      EXPECT_EQ(Snapshot.*Entry.Field, 17U);
      Snapshot.*Entry.Field = 41;
      ASSERT_TRUE(Environment.set("invalid"));
      EXPECT_EQ(ModuleArchiveLimits{}.*Entry.Field, Entry.DefaultValue);
      EXPECT_EQ(Snapshot.*Entry.Field, 41U);
      ASSERT_TRUE(Environment.set("0"));
      EXPECT_EQ(ModuleArchiveLimits{}.*Entry.Field, 0U);
    }
  }

  // Nested AST limits continue using AST configuration independently of the enclosing module budget.
  TEST(IRConfigLimitsTest, EmbeddedASTKeepsItsOwnConfiguration)
  {
    core::test::ScopedEnvironmentVariable ModuleEnvironment("INK_MODULE_ARCHIVE_MAX_BYTES");
    core::test::ScopedEnvironmentVariable ASTEnvironment("INK_AST_ARCHIVE_MAX_BYTES");
    ASSERT_TRUE(ModuleEnvironment.set("123"));
    ASSERT_TRUE(ASTEnvironment.set("456"));
    ModuleArchiveLimits Snapshot;
    EXPECT_EQ(Snapshot.MaxArchiveBytes, 123U);
    EXPECT_EQ(Snapshot.AST.MaxArchiveBytes, 456U);
    ASSERT_TRUE(ASTEnvironment.set("789"));
    EXPECT_EQ(Snapshot.AST.MaxArchiveBytes, 456U);
    EXPECT_EQ(ModuleArchiveLimits{}.AST.MaxArchiveBytes, 789U);
  }

  // All four archive entry points enforce the configured byte budget and accept an explicit override.
  TEST(IRConfigLimitsTest, ArchiveEntryPointsHonorConfiguredBudgetAndExplicitOverride)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_MODULE_ARCHIVE_MAX_BYTES");
    ASSERT_TRUE(Environment.set(nullptr));
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Root = Builder.createModule(Context.namePool().intern("Configured"));
    ASSERT_NE(Root, nullptr);
    const auto Text = serializeModuleText(*Root);
    const auto Binary = serializeModuleBinary(*Root);
    ASSERT_TRUE(Text.succeeded());
    ASSERT_TRUE(Binary.succeeded());
    ASSERT_TRUE(Environment.set("0"));
    EXPECT_EQ(serializeModuleText(*Root).Status, ModuleArchiveStatus::LimitExceeded);
    EXPECT_EQ(serializeModuleBinary(*Root).Status, ModuleArchiveStatus::LimitExceeded);
    EXPECT_EQ(deserializeModuleText(Context, Text.Bytes).Status, ModuleArchiveStatus::LimitExceeded);
    EXPECT_EQ(deserializeModuleBinary(Context, Binary.Bytes).Status, ModuleArchiveStatus::LimitExceeded);
    EXPECT_EQ(Context.modules().size(), 1U);
    ModuleArchiveLimits Limits;
    Limits.MaxArchiveBytes = Text.Bytes.size();
    const auto ExplicitText = serializeModuleText(*Root, Limits);
    ASSERT_TRUE(ExplicitText.succeeded()) << ExplicitText.Message;
    EXPECT_EQ(ExplicitText.Bytes, Text.Bytes);
    EXPECT_TRUE(deserializeModuleText(Context, Text.Bytes, Limits).succeeded());
    Limits.MaxArchiveBytes = Binary.Bytes.size();
    const auto ExplicitBinary = serializeModuleBinary(*Root, Limits);
    ASSERT_TRUE(ExplicitBinary.succeeded()) << ExplicitBinary.Message;
    EXPECT_EQ(ExplicitBinary.Bytes, Binary.Bytes);
    EXPECT_TRUE(deserializeModuleBinary(Context, Binary.Bytes, Limits).succeeded());
  }
} // namespace ink::ir::test
