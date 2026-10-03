#ifndef INK_TESTCASE_EXECUTION_ARTIFACT_TEST_SUPPORT_H
#define INK_TESTCASE_EXECUTION_ARTIFACT_TEST_SUPPORT_H

#include "ink/execution/artifact/bytecode_archive.h"
#include "ink/execution/artifact/bytecode_builder.h"
#include "ink/execution/artifact/bytecode_linker.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/execution/engine/execution_machine.h"
#include "ink/ir/ir_builder.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace ink::execution::test
{
  struct ArtifactContext
  {
      ir::Function *function(std::string_view Name, const ir::Type &ReturnType, std::span<const ir::Type *const> Parameters = {}, ir::LanguageLinkage Linkage = ir::LanguageLinkage::Ink, ir::FunctionBinding Binding = ir::FunctionBinding::Local)
      {
        auto Function = Builder.createFunction(Context.namePool().intern(Name), *Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters), {}, {}, ir::CallingConvention::C, Linkage, Binding);
        auto *Result = Function.get();
        Functions.push_back(std::move(Function));
        return Result;
      }

      bool begin(ir::Function &Function)
      {
        auto *Body = Builder.createFunctionBody(Function);
        return Body && Builder.setInsertPoint(*Body);
      }

      const ir::IntegerConstant &integer(std::uint64_t Value)
      {
        return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Value));
      }

      BytecodeFunctionInput input(const ir::Function &Function, std::string Module, std::string Name, BytecodeSymbolKind Kind = BytecodeSymbolKind::Definition, BytecodeVisibility Visibility = BytecodeVisibility::Public, std::vector<BytecodeGenericArgument> Arguments = {})
      {
        return {&Function, {std::move(Module), std::move(Name), {}, std::move(Arguments)}, Kind, Visibility};
      }

      core::CompilationContext Compilation;
      ir::IRContext Context{Compilation};
      SemanticValueBridge Bridge{Context};
      ir::IRBuilder Builder{Context};
      const ir::IntegerType &Int32 = *Context.typePool().getType<ir::TypeKind::Integer>(32, true);
      const ir::Type &Bool = Context.typePool().getType<ir::TypeKind::Bool>();
      std::vector<std::unique_ptr<ir::Function>> Functions;
  };

  inline const BytecodeSymbol *findArtifactSymbol(const BytecodeArtifact &Artifact, std::string_view Module, std::string_view Name)
  {
    for (const BytecodeSymbol &Symbol : Artifact.Symbols)
    {
      if (Symbol.Identity.Module == Module && Symbol.Identity.Name == Name)
      {
        return &Symbol;
      }
    }
    return nullptr;
  }

  inline BytecodeArtifactResult constantObject(std::string Module, std::string Name, std::uint64_t Value, BytecodeVisibility Visibility = BytecodeVisibility::Public, ir::LanguageLinkage Linkage = ir::LanguageLinkage::Ink, ir::FunctionBinding Binding = ir::FunctionBinding::Local)
  {
    ArtifactContext Test;
    auto *Function = Test.function(Name, Test.Int32, {}, Linkage, Binding);
    if (!Function || !Test.begin(*Function) || !Test.Builder.createReturnInstruction(&Test.integer(Value)))
    {
      return {BytecodeStatus::InvalidInput, "could not construct constant test function", nullptr};
    }
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, Module, Name, BytecodeSymbolKind::Definition, Visibility)};
    return buildBytecodeObject(Module, Test.Bridge, Inputs);
  }

  inline BytecodeArtifactResult forwardingObject(std::string Module, std::string Name, std::string ImportedModule, std::string ImportedName, std::uint64_t Increment = 0)
  {
    ArtifactContext Test;
    auto *Import = Test.function(ImportedName, Test.Int32);
    auto *Function = Test.function(Name, Test.Int32);
    if (!Import || !Function || !Test.begin(*Function))
    {
      return {BytecodeStatus::InvalidInput, "could not construct forwarding test function", nullptr};
    }
    const auto *Call = Test.Builder.createCallInstruction(*Import);
    const auto *Result = Call ? Test.Builder.createAddInstruction(*Call, Test.integer(Increment)) : nullptr;
    if (!Result || !Test.Builder.createReturnInstruction(Result))
    {
      return {BytecodeStatus::InvalidInput, "could not construct forwarding test body", nullptr};
    }
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, Module, Name), Test.input(*Import, ImportedModule, ImportedName, BytecodeSymbolKind::Import)};
    return buildBytecodeObject(Module, Test.Bridge, Inputs);
  }

  inline RuntimeValueResult executeArtifact(BytecodeArtifact &Artifact)
  {
    core::CompilationContext Compilation;
    ir::IRContext EmptyContext(Compilation);
    ExecutionEngine Engine(EmptyContext);
    ExecutionLinker Linker(std::move(Artifact.Image));
    ExecutionMachine Machine(Engine, Linker);
    return Machine.execute(Artifact.Entry, {});
  }

  class ArtifactFiles final
  {
    public:
      ArtifactFiles()
      {
        std::error_code Error;
        const auto Parent = std::filesystem::temp_directory_path(Error);
        if (Error)
        {
          return;
        }
        static std::atomic<std::uint64_t> Next{0};
        const auto Stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (std::size_t Attempt = 0; Attempt < 32; ++Attempt)
        {
          Directory = Parent / ("ink-artifact-" + std::to_string(Stamp) + "-" + std::to_string(Next.fetch_add(1)));
          if (std::filesystem::create_directory(Directory, Error))
          {
            Ready = true;
            return;
          }
          if (Error)
          {
            return;
          }
        }
      }

      ~ArtifactFiles()
      {
        if (Ready)
        {
          std::error_code Error;
          for (const auto &Path : Paths)
          {
            std::filesystem::remove(Path, Error);
          }
          std::filesystem::remove(Directory, Error);
        }
      }

      bool ready() const noexcept
      {
        return Ready;
      }

      std::filesystem::path file(std::string_view Name)
      {
        auto Path = Directory / Name;
        Paths.push_back(Path);
        return Path;
      }

    private:
      bool Ready = false;
      std::filesystem::path Directory;
      std::vector<std::filesystem::path> Paths;
  };
} // namespace ink::execution::test

#endif
