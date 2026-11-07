#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/text/module_text.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    std::string validationDescription(const core::Diagnostic &Diagnostic)
    {
      for (const core::DiagnosticArgument &Argument : Diagnostic.Arguments)
      {
        if (Argument.Name == core::DiagnosticArgumentName::Description)
        {
          if (const auto *Description = std::get_if<std::string>(&Argument.Value))
          {
            return *Description;
          }
        }
      }
      return {};
    }

    void expectGraphError(std::string_view Text, std::string_view Expected)
    {
      auto Parsed = readModuleText(Text);
      ASSERT_FALSE(Parsed.has_value());
      EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::IRTextInvalid);
      EXPECT_EQ(Parsed.error().classification(), core::DiagnosticClass::User);
      const std::string Description = validationDescription(Parsed.error());
      EXPECT_NE(Description.find(Expected), std::string::npos) << Description;
    }
  } // namespace

  // Self inheritance and two-record inheritance cycles are recoverable input errors even when storage would be reference-shaped.
  TEST(IRTextValidationTest, RejectsDirectAndMutualInheritanceCycles)
  {
    expectGraphError(R"(module "" { %t0 = type struct extends (%t0) {} })", "inheritance contains a cycle");
    expectGraphError(R"(module "" { %t0 = type class extends (%t1) {} %t1 = type class extends (%t0) {} })", "inheritance contains a cycle");
  }

  // Struct instance fields cannot form direct, mutual, or array-mediated recursive by-value layouts.
  TEST(IRTextValidationTest, RejectsRecursiveByValueRecordStorage)
  {
    expectGraphError(R"(module "" { %t0 = type struct { field public instance "Self": %t0; } })", "recursive by-value layout");
    expectGraphError(R"(module "" { %t0 = type struct { field public instance "Next": %t1; } %t1 = type struct { field public instance "Previous": %t0; } })", "recursive by-value layout");
    expectGraphError(R"(module "" { %t0 = type struct { field public instance "Items": [1 x %t0]; } })", "recursive by-value layout");
  }

  // A class field is a reference and a static struct field is outside instance layout, so both may refer to their own nominal type.
  TEST(IRTextValidationTest, AcceptsReferenceAndStaticSelfReferences)
  {
    constexpr std::string_view Text = R"(module "" { %t0 = type class { field public instance "Self": %t0; } %t1 = type struct { field public static "Shared": %t1; } })";
    auto Parsed = readModuleText(Text);
    ASSERT_TRUE(Parsed.has_value()) << validationDescription(Parsed.error());
    EXPECT_EQ((*Parsed)->module().objectTypeCount(), 1U);
    EXPECT_EQ((*Parsed)->module().structTypeCount(), 1U);
  }

  // Two CFG edges from one predecessor require two phi entries carrying the same value identity.
  TEST(IRTextValidationTest, RejectsConflictingValuesOnRepeatedPhiPredecessors)
  {
    constexpr std::string_view Text = R"(module ""
{
  define private none i32 @f0 name ""(positional bool %a0 name "") abi ink cc default fixed
  {
    %bb0 name "":
      cond_branch %a0, %bb1, %bb1;
    %bb1 name "":
      %v0 = phi [int<i32>(1), %bb0], [int<i32>(2), %bb0]: i32;
      return %v0;
  }
})";
    expectGraphError(Text, "different values for repeated predecessor edges");
  }

  // A repeated destination edge must not be deduplicated when comparing phi incoming counts against the CFG.
  TEST(IRTextValidationTest, RequiresOnePhiEntryPerRepeatedCfgEdge)
  {
    constexpr std::string_view Text = R"(module ""
{
  define private none i32 @f0 name ""(positional bool %a0 name "") abi ink cc default fixed
  {
    %bb0 name "":
      cond_branch %a0, %bb1, %bb1;
    %bb1 name "":
      %v0 = phi [int<i32>(1), %bb0]: i32;
      return %v0;
  }
})";
    expectGraphError(Text, "phi incoming edges do not match CFG predecessors");
  }

  // Equal repeated phi inputs remain distinct edge entries while pooled constants preserve their shared value identity.
  TEST(IRTextValidationTest, AcceptsEqualValuesOnRepeatedPhiPredecessors)
  {
    constexpr std::string_view Text = R"(module ""
{
  define private none i32 @f0 name ""(positional bool %a0 name "") abi ink cc default fixed
  {
    %bb0 name "":
      cond_branch %a0, %bb1, %bb1;
    %bb1 name "":
      %v0 = phi [int<i32>(1), %bb0], [int<i32>(1), %bb0]: i32;
      return %v0;
  }
})";
    auto Parsed = readModuleText(Text);
    ASSERT_TRUE(Parsed.has_value()) << validationDescription(Parsed.error());
  }

  // A phi edge use is checked at its predecessor terminator, so a value defined only in the destination cannot flow backward onto that edge.
  TEST(IRTextValidationTest, RejectsPhiValuesThatDoNotDominateTheirIncomingEdges)
  {
    constexpr std::string_view Text = R"(module ""
{
  define private none i32 @f0 name ""() abi ink cc default fixed
  {
    %bb0 name "":
      branch %bb1;
    %bb1 name "":
      %v0 = phi [%v1, %bb0]: i32;
      %v1 = add int<i32>(1), int<i32>(2): i32;
      return %v0;
  }
})";
    expectGraphError(Text, "definition does not dominate its use");
  }

  // Type-valued instruction references nested in pointer metadata cannot borrow SSA definitions from a different function.
  TEST(IRTextValidationTest, RejectsForeignInstructionsHiddenInTypeMetadata)
  {
    constexpr std::string_view Text = R"(module ""
{
  define private none void @f0 name ""(positional ptr<type> %a0 name "") abi ink cc default fixed
  {
    %bb0 name "":
      %v0 = load %a0: type;
      return;
  }
  define private none void @f1 name ""() abi ink cc default fixed
  {
    %bb1 name "":
      %v1 = alloca ptr<%v0>: ptr<ptr<%v0>>;
      return;
  }
})";
    expectGraphError(Text, "operand instruction is not owned by this function");
  }

  // A type-producing definition in one branch does not dominate a sibling branch that embeds it in allocation metadata.
  TEST(IRTextValidationTest, ChecksDominanceInsideNestedTypeExpressions)
  {
    constexpr std::string_view Text = R"(module ""
{
  define private none void @f0 name ""(positional bool %a0 name "", positional ptr<type> %a1 name "") abi ink cc default fixed
  {
    %bb0 name "":
      cond_branch %a0, %bb1, %bb2;
    %bb1 name "":
      %v0 = load %a1: type;
      return;
    %bb2 name "":
      %v1 = alloca [2 x ptr<%v0>]: ptr<[2 x ptr<%v0>]>;
      return;
  }
})";
    expectGraphError(Text, "definition does not dominate its use");
  }

  // Flat record declarations can encode a much deeper type graph than any individual expression, and must hit a recoverable graph-depth limit.
  TEST(IRTextValidationTest, BoundsDepthAcrossFlatNominalReferenceChains)
  {
    std::string Text = "module \"\" {\n";
    constexpr std::size_t Count = 160;
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      Text += "%t" + std::to_string(Index) + " = type struct { ";
      if (Index + 1 < Count)
      {
        Text += "field public instance \"Next\": ptr<%t" + std::to_string(Index + 1) + ">; ";
      }
      Text += "}\n";
    }
    Text += "}";
    expectGraphError(Text, "type graph exceeds supported validation depth");
  }

  // A shallow static index of a long by-value chain must still be bounded during layout traversal after type nodes have been cached.
  TEST(IRTextValidationTest, BoundsDepthOfPreviouslyVisitedStorageGraphs)
  {
    std::string Text = "module \"\" {\n%root = type struct {\n";
    constexpr std::size_t Count = 160;
    for (std::size_t Index = Count; Index != 0; --Index)
    {
      Text += "field public static \"Cached\": %t" + std::to_string(Index - 1) + ";\n";
    }
    Text += "field public instance \"Root\": %t0;\n}\n";
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      Text += "%t" + std::to_string(Index) + " = type struct { ";
      if (Index + 1 < Count)
      {
        Text += "field public instance \"Next\": %t" + std::to_string(Index + 1) + "; ";
      }
      Text += "}\n";
    }
    Text += "}";
    expectGraphError(Text, "storage graph exceeds supported validation depth");
  }

  // Hundreds of records reached only through successive method bodies are processed as a worklist instead of accumulating recursive extraction frames.
  TEST(IRTextValidationTest, RoundTripsLongMethodBodyReachabilityChains)
  {
    TypePool Types;
    constexpr std::size_t Count = 512;
    std::vector<StructType *> Records;
    Records.reserve(Count);
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      Records.push_back(&Types.createStructType());
    }
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    InstructionBuilder Builder(Types);
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      Function &MethodFunction = Records[Index]->addMethod(Signature).function();
      Builder.setInsertPoint(MethodFunction.createBasicBlock());
      if (Index + 1 < Count)
      {
        static_cast<void>(Builder.createAlloca(*Records[Index + 1]));
      }
      static_cast<void>(Builder.createReturn());
    }
    Module Original;
    Original.createGlobalVariable(Types.getPointerType(*Records.front()));
    auto Written = writeModuleText(Original);
    ASSERT_TRUE(Written.has_value()) << validationDescription(Written.error());
    auto Parsed = readModuleText(*Written);
    ASSERT_TRUE(Parsed.has_value()) << validationDescription(Parsed.error());
    EXPECT_EQ((*Parsed)->module().structTypeCount(), 0U);
    ASSERT_EQ((*Parsed)->module().globalVariableCount(), 1U);
    const auto *Current = static_cast<const StructType *>(&(*Parsed)->module().globalVariables().begin()->valueType());
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      ASSERT_EQ(Current->methods().size(), 1U);
      const Function &MethodFunction = Current->methods()[0].function();
      ASSERT_EQ(MethodFunction.basicBlockCount(), 1U);
      const BasicBlock &Block = *MethodFunction.basicBlocks().begin();
      if (Index + 1 < Count)
      {
        ASSERT_EQ(Block.instructionCount(), 2U);
        const auto &Allocation = static_cast<const AllocaInstruction &>(*Block.instructions().begin());
        ASSERT_TRUE(StructType::classof(&Allocation.getAllocatedType()));
        Current = static_cast<const StructType *>(&Allocation.getAllocatedType());
      }
      else
      {
        EXPECT_EQ(Block.instructionCount(), 1U);
      }
    }
    auto Rewritten = writeModuleText((*Parsed)->module());
    ASSERT_TRUE(Rewritten.has_value()) << validationDescription(Rewritten.error());
    EXPECT_EQ(*Rewritten, *Written);
  }
} // namespace ink::ir::test
