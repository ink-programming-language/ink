#include "ink/execution/hybrid/archive.h"
#include "ink/execution/hybrid/runtime.h"
#include "ink/execution/artifact/bytecode_builder.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>
#include <thread>

namespace ink::execution::test
{
  // Immutable string bytes returned across the native boundary outlive the patch image.
  TEST(HybridRuntimeTest, RetainsStringResultAfterPatchRemoval)
  {
    std::thread Worker([]()
    {
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      ir::IRBuilder Builder(Context);
      auto *Module = Builder.createModule(Context.namePool().intern("strings"));
      const auto *U8 = Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto *String = Context.typePool().getType<ir::TypeKind::Slice>(*U8, ir::AccessKind::ReadOnly);
      auto Function = Builder.createFunction(Context.namePool().intern("text"), *Context.typePool().getType<ir::TypeKind::Function>(*String));
      auto *Source = Function.get();
      auto *Body = Builder.createFunctionBody(*Source);
      Builder.appendValue(Module->entryBlock(), std::move(Function));
      Builder.setInsertPoint(*Body);
      Builder.createReturnInstruction(Context.constantPool().getStringConstant(*String, "retained"));
      SemanticValueBridge Bridge(Context);
      BytecodeFunctionInput Input;
      Input.Function = Source;
      Input.Kind = BytecodeSymbolKind::Import;
      auto Base = buildBytecodeObject("strings", Bridge, std::span(&Input, 1));
      ASSERT_TRUE(Base) << Base.Message;
      Input.Kind = BytecodeSymbolKind::Definition;
      auto Patch = buildBytecodeObject("strings", Bridge, std::span(&Input, 1));
      ASSERT_TRUE(Patch) << Patch.Message;
      const auto Build = hybridBuildId("string-test");
      const auto Manifest = serializeHybridArtifact(HybridArtifactKind::Base, Build, *Base.Artifact);
      const auto Package = serializeHybridArtifact(HybridArtifactKind::Patch, Build, *Patch.Artifact);
      ASSERT_TRUE(Manifest);
      ASSERT_TRUE(Package);
      const InkHybridFunction Native{Bridge.lowerFunction(*Source), 1, [](void *, const void *const *)
      {
      }};
      const InkHybridModule Definition{1, Manifest.Bytes.data(), Manifest.Bytes.size(), &Native, 1};
      ASSERT_EQ(ink_hybrid_register(&Definition), InkHybridSuccess) << ink_hybrid_last_error();
      ASSERT_EQ(ink_hybrid_apply_bytes(Package.Bytes.data(), Package.Bytes.size()), InkHybridSuccess) << ink_hybrid_last_error();
      struct StringStorage
      {
          const char *Data = nullptr;
          std::size_t Size = 0;
      } Result;
      void *Selected = ink_hybrid_enter(&Definition, Native.Function);
      ASSERT_NE(Selected, nullptr);
      EXPECT_EQ(ink_hybrid_apply_bytes(Package.Bytes.data(), Package.Bytes.size()), InkHybridBusy);
      const auto Status = ink_hybrid_invoke(Selected, &Result, nullptr);
      ink_hybrid_leave();
      ASSERT_EQ(Status, InkHybridSuccess) << ink_hybrid_last_error();
      ASSERT_EQ(ink_hybrid_clear(), InkHybridSuccess);
      EXPECT_EQ(std::string_view(Result.Data, Result.Size), "retained");
    });
    Worker.join();
  }

  // Package decoding rejects truncated headers and foreign versions before invoking bytecode.
  TEST(HybridRuntimeTest, RejectsMalformedArtifacts)
  {
    EXPECT_FALSE(deserializeHybridArtifact(""));
    EXPECT_FALSE(deserializeHybridArtifact("INKHYB01P"));
    EXPECT_FALSE(deserializeHybridArtifact(std::string("INKHYB02P") + std::string(64, 'a')));
    EXPECT_FALSE(deserializeHybridArtifact(std::string("INKHYB01P") + std::string(64, 'z')));
  }
} // namespace ink::execution::test
