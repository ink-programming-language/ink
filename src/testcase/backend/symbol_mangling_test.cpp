#include "ink/backend/llvm/llvm_backend.h"
#include "ink/execution/artifact/bytecode_archive.h"
#include "ink/execution/artifact/bytecode_builder.h"
#include "ink/execution/artifact/bytecode_linker.h"
#include "ink/ir/function/function.h"
#include "ink/ir/ir_builder.h"
#include "ink/ir/linkage.h"
#include "ink/ir/module/module_serialization.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <gtest/gtest.h>

#include <set>

namespace ink::backend::test
{
  namespace
  {
    struct Program
    {
        explicit Program(std::string_view Source, std::string Revision = "1.0.0", std::vector<std::pair<std::string, std::string>> Variants = {})
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          const semantic::Analyzer::ModuleInput Input{"math", &Parsed, {"org.example", {"demo"}, std::move(Revision), std::move(Variants)}};
          Module = semantic::Analyzer{}.analyzeModules(Context, std::span<const semantic::Analyzer::ModuleInput>(&Input, 1), "math");
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        semantic::SemanticContext Context;
        ir::Module *Module = nullptr;
    };

    void collect(const ir::BasicBlock &Block, std::vector<const ir::Function *> &Functions)
    {
      for (const auto &Value : Block.values())
      {
        if (ir::Function::classof(Value.get()))
        {
          const auto *Function = static_cast<const ir::Function *>(Value.get());
          Functions.push_back(Function);
          for (const auto &Body : Function->blocks())
          {
            collect(*Body, Functions);
          }
        }
      }
    }

    std::set<std::string> symbols(const ir::Module &Module)
    {
      std::vector<const ir::Function *> Functions;
      collect(Module.entryBlock(), Functions);
      std::set<std::string> Result;
      for (const auto *Function : Functions)
      {
        const auto Mangled = ir::functionSymbol(*Function);
        EXPECT_TRUE(Mangled) << Mangled.Error;
        EXPECT_TRUE(Result.insert(Mangled.Name).second) << Mangled.Name;
      }
      return Result;
    }

    execution::BytecodeArtifactResult bytecode(Program &Source, bool Import = false)
    {
      execution::SemanticValueBridge Bridge(Source.Context.irContext());
      std::vector<const ir::Function *> Functions;
      collect(Source.Module->entryBlock(), Functions);
      std::vector<execution::BytecodeFunctionInput> Inputs;
      for (const auto *Function : Functions)
      {
        execution::BytecodeFunctionInput Input;
        Input.Function = Function;
        Input.Kind = Import ? execution::BytecodeSymbolKind::Import : execution::BytecodeSymbolKind::Definition;
        Input.Identity.Module = "math";
        Inputs.push_back(std::move(Input));
      }
      return execution::buildBytecodeObject(Import ? "consumer" : "math", Bridge, Inputs);
    }
  } // namespace

  // Top-level order, comments and body edits do not affect identities; the resolved package revision does.
  TEST(SymbolIntegrationTest, StableAcrossOrderTriviaAndBodyChanges)
  {
    Program First("func a(): i32 { return 1; } func b(): i32 { return 2; }");
    Program Second("// relocated source\nfunc b(): i32 { return 100; } func a(): i32 { return 200; }");
    Program Revised("func a(): i32 { return 1; } func b(): i32 { return 2; }", "2.0.0");
    ASSERT_NE(First.Module, nullptr);
    ASSERT_NE(Second.Module, nullptr);
    ASSERT_NE(Revised.Module, nullptr);
    EXPECT_EQ(symbols(*First.Module), symbols(*Second.Module));
    EXPECT_NE(symbols(*First.Module), symbols(*Revised.Module));
  }

  // Local functions retain their enclosing overload and lexical block even when their own names/signatures coincide.
  TEST(SymbolIntegrationTest, SeparatesLexicalScopesAndParentOverloads)
  {
    Program Source("func f(x: i32): i32 { { func local(): i32 { return 1; } } { func local(): i32 { return 2; } } return x; } func f(x: bool): i32 { func local(): i32 { return 3; } return 0; }");
    ASSERT_NE(Source.Module, nullptr);
    EXPECT_EQ(symbols(*Source.Module).size(), 5U);
    auto Built = bytecode(Source);
    ASSERT_TRUE(Built) << Built.Message;
    EXPECT_EQ(Built.Artifact->Symbols.size(), 5U);
  }

  // LLVM and bytecode emit exactly the same identities for class methods, defaults and free functions.
  TEST(SymbolIntegrationTest, SharesSymbolsAcrossLLVMBytecodeAndReflection)
  {
    Program Source("class Box { field Value: i32 = 7; func get(): i32 { return this.Value; }  func __init__(): void {  }  func __init__(InitialValue: i32): void { this.Value = InitialValue; } }; func main(): i32 { var x = Box(); return x.get(); }");
    ASSERT_NE(Source.Module, nullptr);
    auto Built = bytecode(Source);
    ASSERT_TRUE(Built) << Built.Message;
    ::llvm::LLVMContext Context;
    const auto Lowered = llvm::lowerToLLVMIR(Context, *Source.Module, nullptr);
    ASSERT_TRUE(Lowered.succeeded()) << Lowered.error();
    bool SawInitializer = false;
    for (const auto &Symbol : Built.Artifact->Symbols)
    {
      EXPECT_NE(Lowered.module()->getFunction(Symbol.Identity.LinkName), nullptr) << Symbol.Identity.LinkName;
      SawInitializer = SawInitializer || Symbol.Identity.LinkName.starts_with("_INK2I");
    }
    EXPECT_TRUE(SawInitializer);
    EXPECT_NE(Lowered.module()->getFunction(ir::reflectionSymbol(*Source.Module).Name), nullptr);
    EXPECT_NE(Lowered.module()->getModuleFlag("ink.abi.version"), nullptr);
    EXPECT_NE(Lowered.module()->getNamedMetadata("ink.abi.definitions"), nullptr);
    const auto Saved = execution::serializeBytecodeArtifact(*Built.Artifact);
    ASSERT_TRUE(Saved) << Saved.Message;
    const auto Loaded = execution::deserializeBytecodeArtifact(Saved.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    EXPECT_EQ(Loaded.Artifact->Symbols.front().Identity, Built.Artifact->Symbols.front().Identity);
  }

  // Field default identity follows the field name, independent of its physical index or default expression.
  TEST(SymbolIntegrationTest, InitializerUsesFieldName)
  {
    Program First("class Box { field Value: i32 = 7; }; func main(): i32 { return 0; }");
    Program Second("class Box { field Before: bool = true; field Value: i32 = 8; }; func main(): i32 { return 0; }");
    ASSERT_NE(First.Module, nullptr);
    ASSERT_NE(Second.Module, nullptr);
    const auto A = symbols(*First.Module);
    const auto B = symbols(*Second.Module);
    for (const auto &Symbol : A)
    {
      EXPECT_TRUE(B.contains(Symbol));
    }
  }

  // Package revisions coexist in a linked artifact and an import cannot accidentally bind to another revision.
  TEST(SymbolIntegrationTest, BytecodeSeparatesPackageRevisions)
  {
    Program First("func answer(): i32 { return 1; }");
    Program Second("func answer(): i32 { return 2; }", "2.0.0");
    ASSERT_NE(First.Module, nullptr);
    ASSERT_NE(Second.Module, nullptr);
    auto A = bytecode(First);
    auto B = bytecode(Second);
    auto Imported = bytecode(Second, true);
    ASSERT_TRUE(A) << A.Message;
    ASSERT_TRUE(B) << B.Message;
    ASSERT_TRUE(Imported) << Imported.Message;
    const execution::BytecodeArtifact *Together[] = {A.Artifact.get(), B.Artifact.get()};
    EXPECT_TRUE(execution::linkBytecodeArtifacts(Together));
    const execution::BytecodeArtifact *Wrong[] = {A.Artifact.get(), Imported.Artifact.get()};
    EXPECT_EQ(execution::linkBytecodeArtifacts(Wrong).Status, execution::BytecodeStatus::MissingSymbol);
    const execution::BytecodeArtifact *Right[] = {B.Artifact.get(), Imported.Artifact.get()};
    EXPECT_TRUE(execution::linkBytecodeArtifacts(Right));
  }

  // Text and binary IR recovery preserve package metadata and lexical scopes, so recompilation keeps every link name.
  TEST(SymbolIntegrationTest, IRArchivesPreserveIdentities)
  {
    Program Source("func outer(): i32 { func local(): i32 { return 3; } return local(); }", "1.0.0", {{"feature", "on"}, {"cfg", "checked"}});
    ASSERT_NE(Source.Module, nullptr);
    ASSERT_EQ(Source.Module->linkageIdentity().Package.Variant.front().first, "cfg");
    auto ChangedIdentity = Source.Module->linkageIdentity();
    ChangedIdentity.Package.Revision = "2.0.0";
    EXPECT_FALSE(ir::IRBuilder(Source.Context.irContext()).setModuleIdentity(*Source.Module, ChangedIdentity));
    const auto Expected = symbols(*Source.Module);
    const parser::ParseResult *Syntax[] = {&Source.Parsed};
    for (bool Binary : {false, true})
    {
      const auto Saved = Binary ? ir::serializeModuleBinary(*Source.Module, {}, Syntax) : ir::serializeModuleText(*Source.Module, {}, Syntax);
      ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
      core::CompilationContext Compilation;
      ir::IRContext Restored(Compilation);
      const auto Loaded = Binary ? ir::deserializeModuleBinary(Restored, Saved.Bytes) : ir::deserializeModuleText(Restored, Saved.Bytes);
      ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
      EXPECT_EQ(Loaded.ModuleValue->linkageIdentity(), Source.Module->linkageIdentity());
      EXPECT_EQ(symbols(*Loaded.ModuleValue), Expected);
      auto OldVersion = Saved.Bytes;
      if (Binary)
      {
        OldVersion[4] = 6;
      }
      else
      {
        OldVersion[7] = '6';
      }
      const auto Old = Binary ? ir::deserializeModuleBinary(Restored, OldVersion) : ir::deserializeModuleText(Restored, OldVersion);
      EXPECT_EQ(Old.Status, core::ArchiveStatus::UnsupportedVersion);
      auto InvalidIdentity = Saved.Bytes;
      const auto Prefix = InvalidIdentity.find("_INK2J");
      ASSERT_NE(Prefix, std::string::npos);
      InvalidIdentity[Prefix + 4] = '1';
      const auto Invalid = Binary ? ir::deserializeModuleBinary(Restored, InvalidIdentity) : ir::deserializeModuleText(Restored, InvalidIdentity);
      EXPECT_FALSE(Invalid.succeeded());
      EXPECT_EQ(Restored.modules().size(), 1U);
    }
  }

  // AOT rejects conflicting layouts even when declarations carry the same valid package-qualified class identity.
  TEST(SymbolIntegrationTest, RejectsConflictingNativeClassContracts)
  {
    Program Source("func main(): i32 { return 0; }");
    ASSERT_NE(Source.Module, nullptr);
    auto &Context = Source.Context.irContext();
    ir::IRBuilder Builder(Context);
    const auto Identity = abi::mangle(abi::record('T', {abi::record('c', {ir::declarationRecord(Source.Module->linkageIdentity(), {}, 'c', "Cell"), {'X', {}}})}));
    ASSERT_TRUE(Identity);
    for (std::uint32_t Width : {8U, 32U})
    {
      const auto *Class = Builder.createClassType(Context.namePool().intern("Cell"));
      const ir::ClassField Fields[] = {{Context.namePool().intern("Value"), Context.typePool().getType<ir::TypeKind::Integer>(Width, true)}};
      ASSERT_TRUE(Builder.defineClassType(*Class, Fields, Identity.Name));
      ASSERT_TRUE(Builder.registerClassType(*Source.Module, *Class));
    }
    ::llvm::LLVMContext LLVM;
    const auto Lowered = llvm::lowerToLLVMIR(LLVM, *Source.Module, nullptr);
    EXPECT_FALSE(Lowered.succeeded());
    EXPECT_NE(Lowered.error().find("Conflicting AOT definitions"), std::string::npos) << Lowered.error();
  }

  // A valid-looking forged symbol with a different result type is rejected against its bytecode descriptor.
  TEST(SymbolIntegrationTest, RejectsForgedLogicalSignature)
  {
    Program Source("func answer(): i32 { return 1; }");
    ASSERT_NE(Source.Module, nullptr);
    auto Built = bytecode(Source);
    ASSERT_TRUE(Built) << Built.Message;
    auto &Symbol = Built.Artifact->Symbols.front();
    const auto Parsed = abi::demangle(Symbol.Identity.LinkName);
    ASSERT_TRUE(Parsed);
    auto F = *abi::childRecords(*Parsed.Identity);
    auto S = *abi::childRecords(F.back());
    S.back() = abi::record('Q', {{'b', {}}});
    F.back() = abi::record('S', S);
    const auto Forged = abi::mangle(abi::record('F', F));
    ASSERT_TRUE(Forged);
    Symbol.Identity.LinkName = Forged.Name;
    EXPECT_EQ(execution::validateBytecodeArtifact(*Built.Artifact).Status, execution::BytecodeStatus::SignatureMismatch);
    EXPECT_FALSE(execution::serializeBytecodeArtifact(*Built.Artifact));
  }
} // namespace ink::backend::test
