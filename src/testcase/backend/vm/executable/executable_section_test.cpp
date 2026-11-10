#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable/executable_file_validator.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  namespace
  {
    ExecutableFile makeSectionedExecutable()
    {
      ExecutableFile Executable;
      Executable.getImage().resize(24);
      Executable.getSections() = {
          {ExecutableSectionKind::Code, 0, 8},
          {ExecutableSectionKind::ReadOnlyData, 8, 8},
          {ExecutableSectionKind::WritableData, 16, 8},
      };
      return Executable;
    }

    void expectSectionICE(const ExecutableFile &Executable, const char *Pattern)
    {
      const core::DiagnosticEngine Diagnostics;
      EXPECT_DEATH(ExecutableFileValidator::validate(Executable, Diagnostics), Pattern);
    }
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getByteOrder()), ByteOrder &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getByteOrder()), const ByteOrder &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getSections()), std::vector<ExecutableSection> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getSections()), const std::vector<ExecutableSection> &>);

  // Explicit sections partition the complete image, and both byte orders allow entries anywhere inside code.
  TEST(VMExecutableSectionTest, AcceptsCompleteSectionsAndBothByteOrders)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable = makeSectionedExecutable();
    EXPECT_EQ(Executable.getByteOrder(), ByteOrder::Little);
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getByteOrder() = ByteOrder::Big;
    Executable.getEntryOffset() = 7;
    ExecutableFileValidator::validate(Executable, Diagnostics);
    EXPECT_EQ(Executable.getSections()[1].Kind, ExecutableSectionKind::ReadOnlyData);
    EXPECT_EQ(Executable.getSections()[2].Kind, ExecutableSectionKind::WritableData);
  }

  // Copying a sectioned executable keeps byte order and section storage independent of later source changes.
  TEST(VMExecutableSectionTest, CopiesByteOrderAndSectionsIndependently)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Original = makeSectionedExecutable();
    Original.getByteOrder() = ByteOrder::Big;
    const ExecutableFile Copy = Original;
    Original.getByteOrder() = ByteOrder::Little;
    Original.getSections()[0].ByteSize = 1;
    Original.getSections().clear();
    EXPECT_EQ(Copy.getByteOrder(), ByteOrder::Big);
    ASSERT_EQ(Copy.getSections().size(), 3U);
    EXPECT_EQ(Copy.getSections()[0].ByteSize, 8U);
    ExecutableFileValidator::validate(Copy, Diagnostics);
  }

  // An opaque image without section metadata remains accepted, preserving the original structural model.
  TEST(VMExecutableSectionTest, AcceptsImagesWithoutSectionMetadata)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable;
    Executable.getImage().resize(3);
    Executable.getEntryOffset() = 2;
    EXPECT_TRUE(Executable.getSections().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Unknown byte-order values are rejected even when no optional section metadata is present.
  TEST(VMExecutableSectionDeathTest, RejectsUnknownByteOrder)
  {
    ExecutableFile Executable;
    Executable.getImage().push_back(0);
    Executable.getByteOrder() = static_cast<ByteOrder>(255);
    expectSectionICE(Executable, "INK-V0002.*image.*byte order is invalid");
  }

  // Section descriptors reject unknown kinds and empty ranges before testing their coverage.
  TEST(VMExecutableSectionDeathTest, RejectsUnknownKindsAndEmptySections)
  {
    ExecutableFile Executable = makeSectionedExecutable();
    Executable.getSections()[1].Kind = static_cast<ExecutableSectionKind>(255);
    expectSectionICE(Executable, "INK-V0002.*section #1.*kind is invalid");
    Executable.getSections()[1].Kind = ExecutableSectionKind::ReadOnlyData;
    Executable.getSections()[1].ByteSize = 0;
    expectSectionICE(Executable, "INK-V0002.*section #1.*section is empty");
  }

  // Section ranges must start at zero and remain in physical order without gaps or overlaps.
  TEST(VMExecutableSectionDeathTest, RejectsGapsOverlapsAndUnorderedSections)
  {
    ExecutableFile Executable = makeSectionedExecutable();
    Executable.getSections()[0].ImageOffset = 1;
    expectSectionICE(Executable, "INK-V0002.*section #0.*ordered, contiguous, and nonoverlapping");
    Executable.getSections()[0].ImageOffset = 0;
    Executable.getSections()[1].ImageOffset = 9;
    expectSectionICE(Executable, "INK-V0002.*section #1.*ordered, contiguous, and nonoverlapping");
    Executable.getSections()[1].ImageOffset = 7;
    expectSectionICE(Executable, "INK-V0002.*section #1.*ordered, contiguous, and nonoverlapping");
    Executable.getSections()[1].ImageOffset = 16;
    expectSectionICE(Executable, "INK-V0002.*section #1.*ordered, contiguous, and nonoverlapping");
  }

  // Section lengths may neither extend past the image nor overflow uint64, and every image byte must be covered.
  TEST(VMExecutableSectionDeathTest, RejectsOutOfBoundsAndIncompleteCoverage)
  {
    ExecutableFile Executable = makeSectionedExecutable();
    Executable.getSections()[2].ByteSize = 9;
    expectSectionICE(Executable, "INK-V0002.*section #2.*section is outside the image");
    Executable.getSections()[2].ByteSize = std::numeric_limits<std::uint64_t>::max();
    expectSectionICE(Executable, "INK-V0002.*section #2.*section is outside the image");
    Executable.getSections()[2].ByteSize = 7;
    expectSectionICE(Executable, "INK-V0002.*section table.*sections do not cover the complete image");
  }

  // Entries at the first data byte, the final data byte, or in images containing only data are rejected.
  TEST(VMExecutableSectionDeathTest, RequiresAnEntryInsideCode)
  {
    ExecutableFile Executable = makeSectionedExecutable();
    Executable.getEntryOffset() = 8;
    expectSectionICE(Executable, "INK-V0002.*image.*entry offset is outside code sections");
    Executable.getEntryOffset() = 23;
    expectSectionICE(Executable, "INK-V0002.*image.*entry offset is outside code sections");
    Executable.getEntryOffset() = 0;
    Executable.getSections()[0].Kind = ExecutableSectionKind::WritableData;
    expectSectionICE(Executable, "INK-V0002.*image.*entry offset is outside code sections");
  }
} // namespace ink::vm::test
