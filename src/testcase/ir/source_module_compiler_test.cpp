#include "ink/ir/compilation/source_module_compiler.h"

#include "ink/core/context.h"
#include "ink/ir/model/module.h"

#include "../diagnostic_test_support.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace ink::ir
{
  namespace
  {
    std::atomic_size_t NextTemporaryDirectoryId{0};

    class TemporaryDirectory
    {
      public:
        TemporaryDirectory()
        {
          std::error_code Error;
          const std::filesystem::path Base = std::filesystem::temp_directory_path(Error);
          if (Error)
          {
            return;
          }
          const auto Timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
          Path = Base / ("ink-source-module-compiler-test-" + std::to_string(Timestamp) + "-" + std::to_string(NextTemporaryDirectoryId.fetch_add(1)));
          Ready = std::filesystem::create_directory(Path, Error) && !Error;
        }

        ~TemporaryDirectory()
        {
          if (Ready)
          {
            std::error_code Error;
            std::filesystem::remove_all(Path, Error);
          }
        }

        bool ready() const noexcept
        {
          return Ready;
        }

        const std::filesystem::path &path() const noexcept
        {
          return Path;
        }

      private:
        std::filesystem::path Path;
        bool Ready = false;
    };

    bool writeTextFile(const std::filesystem::path &Path, std::string_view Text)
    {
      std::error_code Error;
      std::filesystem::create_directories(Path.parent_path(), Error);
      if (Error)
      {
        return false;
      }
      std::ofstream Output(Path, std::ios::binary);
      if (!Output)
      {
        return false;
      }
      Output.write(Text.data(), static_cast<std::streamsize>(Text.size()));
      return static_cast<bool>(Output);
    }

    // Verifies that a canonical module name is mapped to nested path segments and deserialized in the session IR context.
    TEST(SourceModuleCompilerTest, LoadsModuleFromSearchPath)
    {
      TemporaryDirectory Directory;
      ASSERT_TRUE(Directory.ready());
      ASSERT_TRUE(writeTextFile(Directory.path() / "package" / "dependency.ir", R"(inkir 1
module package.dependency

define i32 @answer() {
entry:
  ret i32 42
}
)"));
      SourceModuleCompilerOptions Options;
      Options.ModuleSearchPaths.push_back(Directory.path());
      SourceModuleCompiler Compiler(std::move(Options));
      core::CompilationContext Compilation;
      CompilationSession Session(Compilation, Compiler);

      const ModuleCompilationResult Result = Session.getOrCompileModule("package.dependency");

      ASSERT_EQ(Result.Status, ModuleCompilationStatus::Found);
      ASSERT_NE(Result.ModuleValue, nullptr);
      ASSERT_TRUE(Result.ModuleValue->Name.has_value());
      EXPECT_EQ(*Result.ModuleValue->Name, "package.dependency");
      EXPECT_EQ(&Result.ModuleValue->context(), &Session.irContext());
    }

    // Verifies that an absent module returns NotFound without manufacturing a source diagnostic.
    TEST(SourceModuleCompilerTest, ReportsMissingModuleAsNotFound)
    {
      TemporaryDirectory Directory;
      ASSERT_TRUE(Directory.ready());
      SourceModuleCompilerOptions Options;
      Options.ModuleSearchPaths.push_back(Directory.path());
      SourceModuleCompiler Compiler(std::move(Options));
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      CompilationSession Session(Compilation, Compiler);

      const ModuleCompilationResult Result = Session.getOrCompileModule("package.missing");

      EXPECT_EQ(Result.Status, ModuleCompilationStatus::NotFound);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }

    // Verifies that malformed InkIR found through a search path reports the deserializer diagnostic through the compilation context.
    TEST(SourceModuleCompilerTest, PreservesMalformedModuleDiagnostics)
    {
      TemporaryDirectory Directory;
      ASSERT_TRUE(Directory.ready());
      ASSERT_TRUE(writeTextFile(Directory.path() / "package" / "broken.ir", "not valid InkIR"));
      SourceModuleCompilerOptions Options;
      Options.ModuleSearchPaths.push_back(Directory.path());
      SourceModuleCompiler Compiler(std::move(Options));
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      CompilationSession Session(Compilation, Compiler);

      const ModuleCompilationResult Result = Session.getOrCompileModule("package.broken");

      EXPECT_EQ(Result.Status, ModuleCompilationStatus::Failed);
      EXPECT_FALSE(Diagnostics.diagnostics().empty());
    }

    // Verifies that a caller-provided entry image participates in the same compilation session as searched dependencies.
    TEST(SourceModuleCompilerTest, ReturnsPrecompiledEntryModule)
    {
      SourceModuleCompiler Compiler({});
      core::CompilationContext Compilation;
      CompilationSession Session(Compilation, Compiler);
      auto Entry = std::make_shared<Module>(Session.irContext());
      Entry->Name = "application.entry";
      ASSERT_TRUE(Entry->Name.has_value());
      ASSERT_TRUE(Compiler.addPrecompiledModule(*Entry->Name, Entry));

      const ModuleCompilationResult Result = Session.getOrCompileModule("application.entry");

      ASSERT_EQ(Result.Status, ModuleCompilationStatus::Found);
      EXPECT_EQ(Result.ModuleValue, Entry);
    }
  } // namespace
} // namespace ink::ir
