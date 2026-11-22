#include "ink/backend/llvm_inkvm/inkvm_target.h"
#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include <array>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

namespace ink::backend::llvm_inkvm
{
    namespace
    {
        std::unique_ptr<llvm::Module> parseIR(llvm::LLVMContext& Context, std::string_view IRText, std::string_view ModuleName)
        {
            llvm::SMDiagnostic ParseError;
            auto ParsedModule = llvm::parseAssemblyString(llvm::StringRef(IRText.data(), IRText.size()), ParseError, Context);
            if (ParsedModule != nullptr)
            {
                ParsedModule->setModuleIdentifier(llvm::StringRef(ModuleName.data(), ModuleName.size()));
            }
            return ParsedModule;
        }
    } // namespace

    // Verifies that an empty LLVM IR function lowers through SelectionDAG ISel and AsmPrinter into clean Ink VM assembly text.
    TEST(InkVMEmptyFunctionCodegenTest, EmitsCleanInkVMAssemblyForEmptyMain)
    {
        llvm::LLVMContext Context;
        auto Module = parseIR(Context, "define void @main() {\nentry:\n  ret void\n}\n", "empty_main");
        ASSERT_NE(Module, nullptr);

        std::string Assembly;
        std::string ErrorMessage;
        ASSERT_TRUE(emitInkVMAssembly(*Module, Assembly, ErrorMessage)) << ErrorMessage;
        EXPECT_TRUE(ErrorMessage.empty());

        EXPECT_NE(Assembly.find(".module \"empty_main\""), std::string::npos) << Assembly;
        EXPECT_EQ(Assembly.find(".section .data"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".section .text"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".function @main external"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("entry:"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("    ret\n"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".endfunction"), std::string::npos) << Assembly;

        EXPECT_EQ(Assembly.find(".globl"), std::string::npos) << Assembly;
        EXPECT_EQ(Assembly.find(".type"), std::string::npos) << Assembly;
        EXPECT_EQ(Assembly.find(".size"), std::string::npos) << Assembly;
        EXPECT_EQ(Assembly.find(".file"), std::string::npos) << Assembly;
    }

    // Verifies the full direct pipeline: LLVM IR -> InkVM backend MachineModule -> ExecutableEmitter -> Interpreter execution.
    TEST(InkVMEmptyFunctionCodegenTest, ExecutesCompiledEmptyMainInInkVMInterpreter)
    {
        llvm::LLVMContext Context;
        auto IRModule = parseIR(Context, "define void @main() {\nentry:\n  ret void\n}\n", "empty_exec");
        ASSERT_NE(IRModule, nullptr);

        auto CompiledModule = emitInkVMMachineModule(*IRModule);
        ASSERT_TRUE(CompiledModule.has_value()) << CompiledModule.error();

        core::CollectingDiagnosticConsumer DiagnosticsConsumer;
        vm::Interpreter Interpreter;
        Interpreter.getDiagnostics().addConsumer(DiagnosticsConsumer);

        const core::Name MainName = core::NamePool::instance().intern("main");
        const std::array ExportNames{MainName};
        vm::ExecutableEmitter Emitter(Interpreter.getDiagnostics());
        vm::ExecutableFile Executable = Emitter.emit(**CompiledModule, MainName, ExportNames);

        const auto Handle = Interpreter.loadLibrary(std::move(Executable));
        ASSERT_NE(Handle, vm::Interpreter::InvalidLibraryHandle);

        const void* EntryAddress = Interpreter.resolveSymbolAddress(MainName);
        ASSERT_NE(EntryAddress, nullptr);
        EXPECT_TRUE(Interpreter.execute(EntryAddress));
        EXPECT_TRUE(DiagnosticsConsumer.takeDiagnostics().empty());
    }

    // Verifies that internal/external function linkages and external declarations map cleanly to Ink VM assembly directives.
    TEST(InkVMEmptyFunctionCodegenTest, EmitsLocalLinkageAndExternalDeclarations)
    {
        constexpr std::string_view IRText = R"(declare void @host_ping()
define internal void @helper() {
  ret void
}
define void @main() {
entry:
  ret void
}
)";

        llvm::LLVMContext Context;
        auto IRModule = parseIR(Context, IRText, "multi_fn");
        ASSERT_NE(IRModule, nullptr);

        std::string Assembly;
        std::string ErrorMessage;
        ASSERT_TRUE(emitInkVMAssembly(*IRModule, Assembly, ErrorMessage)) << ErrorMessage;

        EXPECT_NE(Assembly.find(".declare function @host_ping"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".function @helper\n"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".function @main external"), std::string::npos) << Assembly;

        auto ParsedVMModule = vm::readMachineModuleText(Assembly);
        ASSERT_TRUE(ParsedVMModule.has_value()) << Assembly;
        EXPECT_EQ(std::ranges::distance((*ParsedVMModule)->functions()), 2);
        EXPECT_EQ((*ParsedVMModule)->symbolDeclarations().size(), 1u);
    }
} // namespace ink::backend::llvm_inkvm
