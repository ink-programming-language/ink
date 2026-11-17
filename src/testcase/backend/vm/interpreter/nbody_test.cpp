#include "../../../../../benchmark/nbody/nbody_reference.h"

#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace ink::vm::test
{
    // The complete five-body VM program matches both energies and all 35 reference state values, including reset across repeated executions.
    TEST(VMNBodyTest, MatchesScalarReferenceAndResetsBodiesBetweenExecutions)
    {
        const auto SourcePath = std::filesystem::path(__FILE__).parent_path() / "../../../../../benchmark/nbody/nbody.asm";
        std::ifstream Input(SourcePath, std::ios::binary);
        ASSERT_TRUE(Input.is_open());
        const std::string Text{std::istreambuf_iterator<char>(Input), std::istreambuf_iterator<char>()};
        ASSERT_FALSE(Input.bad());
        auto Module = readMachineModuleText(Text);
        ASSERT_TRUE(Module.has_value());
        core::CollectingDiagnosticConsumer Diagnostics;
        Interpreter VM;
        VM.getDiagnostics().addConsumer(Diagnostics);
        auto& Names = core::NamePool::instance();
        const auto EntryName = Names.intern("main");
        const auto StepsName = Names.intern("nbody_steps");
        const auto InitialName = Names.intern("nbody_initial_energy");
        const auto FinalName = Names.intern("nbody_final_energy");
        const auto BodiesName = Names.intern("nbody_bodies");
        const std::array ExportNames{EntryName, StepsName, InitialName, FinalName, BodiesName};
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(**Module, EntryName, ExportNames);
        ASSERT_TRUE(Diagnostics.diagnostics().empty());
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.resolveSymbolAddress(EntryName);
        auto* StepsAddress = VM.resolveSymbolAddress(StepsName);
        const auto* InitialAddress = VM.resolveSymbolAddress(InitialName);
        const auto* FinalAddress = VM.resolveSymbolAddress(FinalName);
        auto* BodiesAddress = VM.resolveSymbolAddress(BodiesName);
        ASSERT_NE(Entry, nullptr);
        ASSERT_NE(StepsAddress, nullptr);
        ASSERT_NE(InitialAddress, nullptr);
        ASSERT_NE(FinalAddress, nullptr);
        ASSERT_NE(BodiesAddress, nullptr);
        constexpr std::array<std::uint64_t, 6> StepCounts = {
            0,
            1,
            10,
            1000,
            0,
            1000,
        };
        for (const auto Steps : StepCounts)
        {
            SCOPED_TRACE(Steps);
            std::memcpy(StepsAddress, &Steps, sizeof(Steps));
            std::memset(BodiesAddress, 0xA5, sizeof(benchmark::nbody::Bodies));
            const auto Expected = benchmark::nbody::reference(Steps);

            ASSERT_TRUE(VM.execute(Entry));
            ASSERT_TRUE(Diagnostics.diagnostics().empty());

            double InitialEnergy;
            double FinalEnergy;
            benchmark::nbody::BodyWords ActualBodies;
            std::memcpy(&InitialEnergy, InitialAddress, sizeof(InitialEnergy));
            std::memcpy(&FinalEnergy, FinalAddress, sizeof(FinalEnergy));
            std::memcpy(ActualBodies.data(), BodiesAddress, sizeof(ActualBodies));
            EXPECT_NEAR(InitialEnergy, -0.16907516382852447, 1.0e-14);
            EXPECT_TRUE(benchmark::nbody::nearlyEqual(InitialEnergy, Expected.InitialEnergy));
            EXPECT_TRUE(benchmark::nbody::nearlyEqual(FinalEnergy, Expected.FinalEnergy));
            const auto ExpectedBodies = benchmark::nbody::bodyWords(Expected.BodyValues);
            for (std::size_t Index = 0; Index < ActualBodies.size(); ++Index)
            {
                SCOPED_TRACE(Index);
                EXPECT_TRUE(benchmark::nbody::nearlyEqual(ActualBodies[Index], ExpectedBodies[Index])) << "actual=" << ActualBodies[Index] << ", expected=" << ExpectedBodies[Index];
            }
        }
        VM.getDiagnostics().removeConsumer(Diagnostics);
    }
} // namespace ink::vm::test
