#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/text/module_text.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    std::string describeDiagnostic(const core::Diagnostic &Diagnostic)
    {
      for (const core::DiagnosticArgument &Argument : Diagnostic.Arguments)
      {
        if (Argument.Name == core::DiagnosticArgumentName::Description)
        {
          if (const std::string *Description = std::get_if<std::string>(&Argument.Value))
          {
            return *Description;
          }
        }
      }
      return Diagnostic.code();
    }

    std::unique_ptr<ParsedModule> roundTrip(const Module &ModuleValue, std::string *SavedText = nullptr)
    {
      auto Written = writeModuleText(ModuleValue);
      if (!Written.has_value())
      {
        ADD_FAILURE() << "Writing IR failed: " << describeDiagnostic(Written.error());
        return nullptr;
      }
      if (SavedText)
      {
        *SavedText = *Written;
      }
      auto Parsed = readModuleText(*Written);
      if (!Parsed.has_value())
      {
        ADD_FAILURE() << "Reading IR failed: " << describeDiagnostic(Parsed.error()) << "\n"
                      << *Written;
        return nullptr;
      }
      auto Rewritten = writeModuleText((*Parsed)->module());
      if (!Rewritten.has_value())
      {
        ADD_FAILURE() << "Rewriting parsed IR failed: " << describeDiagnostic(Rewritten.error());
        return nullptr;
      }
      EXPECT_EQ(*Rewritten, *Written);
      return std::move(*Parsed);
    }

    void expectReadError(std::string_view Text)
    {
      auto Parsed = readModuleText(Text);
      ASSERT_FALSE(Parsed.has_value());
      EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::IRTextInvalid);
      EXPECT_EQ(Parsed.error().classification(), core::DiagnosticClass::User);
      ASSERT_TRUE(Parsed.error().Span.isValid());
      EXPECT_LE(Parsed.error().Span.getEnd().getByteOffset(), Text.size());
      EXPECT_FALSE(describeDiagnostic(Parsed.error()).empty());
    }
  } // namespace

  // Empty unnamed modules survive owning deserialization without acquiring declarations or a synthetic source name.
  TEST(IRModuleTextTest, RoundTripsAnEmptyUnnamedModule)
  {
    const Module Original;
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    EXPECT_EQ(Parsed->module().name(), core::NAME_None);
    EXPECT_EQ(Parsed->module().structTypeCount(), 0U);
    EXPECT_EQ(Parsed->module().objectTypeCount(), 0U);
    EXPECT_EQ(Parsed->module().globalVariableCount(), 0U);
    EXPECT_EQ(Parsed->module().functionCount(), 0U);
  }

  // Length-delimited module, function, argument, and block names retain quotes, backslashes, Unicode bytes, and embedded NULs.
  TEST(IRModuleTextTest, PreservesEscapedNamesIndependentlyOfTextualIds)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    const std::string Spelling = std::string("quote\" slash\\\n") + '\0' + "\xe5\x90\x8d";
    const core::Name Name = Names.intern(Spelling);
    Module Original(Name);
    Function &FunctionValue = Original.createFunction(Types.getFunctionType(Types.getBoolType(), std::array{Parameter(Types.getBoolType())}), Name);
    FunctionValue.argument(0).setName(Name);
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(FunctionValue.createBasicBlock(Name));
    static_cast<void>(Builder.createReturn(FunctionValue.argument(0)));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    EXPECT_EQ(Names.text(Parsed->module().name()), Spelling);
    ASSERT_EQ(Parsed->module().functionCount(), 1U);
    const Function &Copy = *Parsed->module().functions().begin();
    EXPECT_EQ(Names.text(Copy.name()), Spelling);
    EXPECT_EQ(Names.text(Copy.argument(0).name()), Spelling);
    ASSERT_EQ(Copy.basicBlockCount(), 1U);
    EXPECT_EQ(Names.text(Copy.basicBlocks().begin()->name()), Spelling);
    const auto &Return = static_cast<const ReturnInstruction &>(*Copy.basicBlocks().begin()->terminator());
    EXPECT_EQ(Return.getReturnValue(), &Copy.argument(0));
  }

  // Recursive struct/class fields, bases, and nested method functions keep nominal identity, declaration order, and member attributes.
  TEST(IRModuleTextTest, PreservesNestedMembersAndRecursiveRecordIdentities)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    StructType &First = Types.createStructType();
    StructType &Second = Types.createStructType();
    ObjectType &Object = Types.createObjectType();
    Module Original(Names.intern("Records"));
    Original.addStructType(First);
    Original.addStructType(Second);
    Original.addObjectType(Object);
    First.addField(Names.intern("Next"), Types.getPointerType(Second));
    Second.addField(Names.intern("Previous"), Types.getPointerType(First));
    Object.addBase(First);
    Object.addField(Names.intern("Payload"), Second, MemberKind::Instance, SymbolVisibility::Private);
    Object.addField(Names.intern("Count"), Int32, MemberKind::Static, SymbolVisibility::Public);
    const FunctionType &Signature = Types.getFunctionType(Int32, std::array{Parameter(Int32, Parameter::Kind::Named)});
    Function &Implementation = Object.addMethod(Signature, Names.intern("Echo"), MemberKind::Instance, SymbolVisibility::Public, NativeLinkage::Export).function();
    Implementation.argument(0).setName(Names.intern("Value"));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Implementation.createBasicBlock(Names.intern("Entry")));
    static_cast<void>(Builder.createReturn(Implementation.argument(0)));
    static_cast<void>(Object.addMethod(Types.getFunctionType(Types.getVoidType()), Names.intern("Native"), MemberKind::Static, SymbolVisibility::Private, NativeLinkage::Import));
    std::string Text;
    auto Parsed = roundTrip(Original, &Text);
    ASSERT_NE(Parsed, nullptr);
    const Module &Copy = Parsed->module();
    ASSERT_EQ(Copy.structTypeCount(), 2U);
    ASSERT_EQ(Copy.objectTypeCount(), 1U);
    EXPECT_EQ(Copy.functionCount(), 0U);
    const StructType &FirstCopy = *Copy.structTypes().begin();
    const StructType &SecondCopy = *std::next(Copy.structTypes().begin());
    const ObjectType &ObjectCopy = *Copy.objectTypes().begin();
    EXPECT_NE(&FirstCopy, &SecondCopy);
    ASSERT_EQ(FirstCopy.fields().size(), 1U);
    ASSERT_EQ(SecondCopy.fields().size(), 1U);
    EXPECT_EQ(&static_cast<const PointerType &>(FirstCopy.fields()[0].type()).pointeeType(), &SecondCopy);
    EXPECT_EQ(&static_cast<const PointerType &>(SecondCopy.fields()[0].type()).pointeeType(), &FirstCopy);
    ASSERT_EQ(ObjectCopy.bases().size(), 1U);
    EXPECT_EQ(ObjectCopy.bases()[0], &FirstCopy);
    ASSERT_EQ(ObjectCopy.fields().size(), 2U);
    EXPECT_EQ(ObjectCopy.fields()[0].name(), Names.intern("Payload"));
    EXPECT_EQ(&ObjectCopy.fields()[0].type(), &SecondCopy);
    EXPECT_EQ(ObjectCopy.fields()[0].kind(), MemberKind::Instance);
    EXPECT_EQ(ObjectCopy.fields()[0].visibility(), SymbolVisibility::Private);
    EXPECT_EQ(ObjectCopy.fields()[1].name(), Names.intern("Count"));
    EXPECT_EQ(ObjectCopy.fields()[1].kind(), MemberKind::Static);
    EXPECT_EQ(ObjectCopy.fields()[1].visibility(), SymbolVisibility::Public);
    ASSERT_EQ(ObjectCopy.methods().size(), 2U);
    EXPECT_EQ(ObjectCopy.methods()[0].kind(), MemberKind::Instance);
    const Function &MethodCopy = ObjectCopy.methods()[0].function();
    EXPECT_EQ(MethodCopy.name(), Names.intern("Echo"));
    EXPECT_EQ(MethodCopy.parent(), nullptr);
    EXPECT_EQ(MethodCopy.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(MethodCopy.nativeLinkage(), NativeLinkage::Export);
    ASSERT_EQ(MethodCopy.argumentCount(), 1U);
    EXPECT_EQ(MethodCopy.argument(0).kind(), Parameter::Kind::Named);
    EXPECT_EQ(MethodCopy.argument(0).name(), Names.intern("Value"));
    ASSERT_EQ(MethodCopy.basicBlockCount(), 1U);
    EXPECT_EQ(MethodCopy.basicBlocks().begin()->parent(), &MethodCopy);
    EXPECT_EQ(static_cast<const ReturnInstruction *>(MethodCopy.basicBlocks().begin()->terminator())->getReturnValue(), &MethodCopy.argument(0));
    EXPECT_EQ(ObjectCopy.methods()[1].kind(), MemberKind::Static);
    EXPECT_EQ(ObjectCopy.methods()[1].function().nativeLinkage(), NativeLinkage::Import);
    EXPECT_EQ(ObjectCopy.methods()[1].function().basicBlockCount(), 0U);
    EXPECT_NE(Text.find("struct"), std::string::npos);
    EXPECT_NE(Text.find("class"), std::string::npos);
    EXPECT_NE(Text.find("field"), std::string::npos);
    EXPECT_NE(Text.find("method"), std::string::npos);
    verifyModule(Copy, core::DiagnosticEngine{});
  }

  // Canonical assembly keeps fields and a complete method body inside their struct while emitting class declarations separately.
  TEST(IRModuleTextTest, PrintsCanonicalNestedRecordAssembly)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Module Original(Names.intern("Demo"));
    StructType &Record = Types.createStructType();
    ObjectType &Object = Types.createObjectType();
    Original.addStructType(Record);
    Original.addObjectType(Object);
    Record.addField(Names.intern("X"), Int32);
    Object.addField(Names.intern("Value"), Int32, MemberKind::Instance, SymbolVisibility::Private);
    Function &MethodFunction = Record.addMethod(Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Int32)}), Names.intern("sum"), MemberKind::Static, SymbolVisibility::Public).function();
    MethodFunction.argument(0).setName(Names.intern("a"));
    MethodFunction.argument(1).setName(Names.intern("b"));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(MethodFunction.createBasicBlock(Names.intern("entry")));
    AddInstruction &Sum = Builder.createAdd(MethodFunction.argument(0), MethodFunction.argument(1));
    static_cast<void>(Builder.createReturn(Sum));
    const std::string Expected = R"(module "Demo"
{
  %t0 = type struct
  {
    field public instance "X": i32;
    method static define public none i32 @f0 name "sum"(positional i32 %a0 name "a", positional i32 %a1 name "b") abi ink cc default fixed
    {
      %bb0 name "entry":
        %v0 = add %a0, %a1: i32;
        return %v0;
    }
  }
  %t1 = type class
  {
    field private instance "Value": i32;
  }
}
)";
    auto Written = writeModuleText(Original);
    ASSERT_TRUE(Written.has_value());
    EXPECT_EQ(*Written, Expected);
    auto Parsed = readModuleText(Expected);
    ASSERT_TRUE(Parsed.has_value()) << describeDiagnostic(Parsed.error());
    auto Rewritten = writeModuleText((*Parsed)->module());
    ASSERT_TRUE(Rewritten.has_value());
    EXPECT_EQ(*Rewritten, Expected);
  }

  // Equal-looking nominal records remain distinct even when both have empty bodies and share the same derived pointer shape.
  TEST(IRModuleTextTest, RetainsDistinctEmptyNominalTypes)
  {
    TypePool Types;
    Module Original;
    StructType &First = Types.createStructType();
    StructType &Second = Types.createStructType();
    Original.addStructType(First);
    Original.addStructType(Second);
    Original.createGlobalVariable(Types.getPointerType(First));
    Original.createGlobalVariable(Types.getPointerType(Second));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const Module &Copy = Parsed->module();
    ASSERT_EQ(Copy.structTypeCount(), 2U);
    const StructType &FirstCopy = *Copy.structTypes().begin();
    const StructType &SecondCopy = *std::next(Copy.structTypes().begin());
    EXPECT_NE(&FirstCopy, &SecondCopy);
    ASSERT_EQ(Copy.globalVariableCount(), 2U);
    EXPECT_EQ(&Copy.globalVariables().begin()->valueType(), &FirstCopy);
    EXPECT_EQ(&std::next(Copy.globalVariables().begin())->valueType(), &SecondCopy);
  }

  // Reachable borrowed record definitions are serialized without silently adding them to the module's registration lists.
  TEST(IRModuleTextTest, PreservesUnregisteredReachableRecordDefinitions)
  {
    TypePool Types;
    StructType &Borrowed = Types.createStructType();
    Borrowed.addField(core::NamePool::instance().intern("Self"), Types.getPointerType(Borrowed));
    static_cast<void>(Borrowed.addMethod(Types.getFunctionType(Types.getVoidType())));
    Module Original;
    Original.createGlobalVariable(Types.getPointerType(Borrowed));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    EXPECT_EQ(Parsed->module().structTypeCount(), 0U);
    ASSERT_EQ(Parsed->module().globalVariableCount(), 1U);
    const Value &StoredType = Parsed->module().globalVariables().begin()->valueType();
    ASSERT_TRUE(StructType::classof(&StoredType));
    const auto &RecordCopy = static_cast<const StructType &>(StoredType);
    ASSERT_EQ(RecordCopy.fields().size(), 1U);
    EXPECT_EQ(&static_cast<const PointerType &>(RecordCopy.fields()[0].type()).pointeeType(), &RecordCopy);
    EXPECT_EQ(RecordCopy.methods().size(), 1U);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Function and global operands resolve by identity despite duplicate names, and record-owned methods remain nested owners.
  TEST(IRModuleTextTest, ResolvesFunctionMethodAndGlobalOperandsByIdentity)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    Module Original;
    const FunctionType &TargetSignature = Types.getFunctionType(Types.getVoidType());
    const core::Name DuplicateName = Names.intern("SameName");
    Function &First = Original.createFunction(TargetSignature, DuplicateName);
    Function &Second = Original.createFunction(TargetSignature, DuplicateName);
    StructType &Record = Types.createStructType();
    Original.addStructType(Record);
    Function &MethodFunction = Record.addMethod(TargetSignature, DuplicateName).function();
    GlobalVariable &Global = Original.createGlobalVariable(Types.getPointerType(Types.getBoolType()), DuplicateName, SymbolVisibility::Public, NativeLinkage::Export);
    Function &FunctionGetter = Original.createFunction(Types.getFunctionType(TargetSignature));
    Function &MethodGetter = Original.createFunction(Types.getFunctionType(TargetSignature));
    Function &GlobalGetter = Original.createFunction(Types.getFunctionType(Global.getType()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(FunctionGetter.createBasicBlock());
    static_cast<void>(Builder.createReturn(Second));
    Builder.setInsertPoint(MethodGetter.createBasicBlock());
    static_cast<void>(Builder.createReturn(MethodFunction));
    Builder.setInsertPoint(GlobalGetter.createBasicBlock());
    static_cast<void>(Builder.createReturn(Global));
    static_cast<void>(First);
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const Module &Copy = Parsed->module();
    ASSERT_EQ(Copy.functionCount(), 5U);
    const Function &FirstCopy = *Copy.functions().begin();
    const Function &SecondCopy = *std::next(Copy.functions().begin());
    const Function &FunctionGetterCopy = *std::next(Copy.functions().begin(), 2);
    const Function &MethodGetterCopy = *std::next(Copy.functions().begin(), 3);
    const Function &GlobalGetterCopy = *std::next(Copy.functions().begin(), 4);
    EXPECT_EQ(FirstCopy.name(), SecondCopy.name());
    EXPECT_NE(&FirstCopy, &SecondCopy);
    EXPECT_EQ(SecondCopy.parent(), &Copy);
    EXPECT_EQ(static_cast<const ReturnInstruction *>(FunctionGetterCopy.basicBlocks().begin()->terminator())->getReturnValue(), &SecondCopy);
    ASSERT_EQ(Copy.structTypes().begin()->methods().size(), 1U);
    EXPECT_EQ(static_cast<const ReturnInstruction *>(MethodGetterCopy.basicBlocks().begin()->terminator())->getReturnValue(), &Copy.structTypes().begin()->methods()[0].function());
    ASSERT_EQ(Copy.globalVariableCount(), 1U);
    const GlobalVariable &GlobalCopy = *Copy.globalVariables().begin();
    EXPECT_EQ(GlobalCopy.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(GlobalCopy.nativeLinkage(), NativeLinkage::Export);
    EXPECT_EQ(GlobalCopy.parent(), &Copy);
    EXPECT_EQ(static_cast<const ReturnInstruction *>(GlobalGetterCopy.basicBlocks().begin()->terminator())->getReturnValue(), &GlobalCopy);
    verifyModule(Copy, core::DiagnosticEngine{});
  }

  // All instruction kinds survive a loop whose exit block appears before its dominating phi in declaration order.
  TEST(IRModuleTextTest, PreservesMemoryOperationsLoopBackedgesAndBlockOrder)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    core::NamePool &Names = core::NamePool::instance();
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *One = Constants.getInteger(Int32, 1);
    ASSERT_NE(One, nullptr);
    Module Original;
    Function &FunctionValue = Original.createFunction(Types.getFunctionType(Int32, std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Entry = FunctionValue.createBasicBlock(Names.intern("Entry"));
    BasicBlock &Exit = FunctionValue.createBasicBlock(Names.intern("Exit"));
    BasicBlock &Loop = FunctionValue.createBasicBlock(Names.intern("Loop"));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    AllocaInstruction &Slot = Builder.createAlloca(Int32);
    static_cast<void>(Builder.createStore(Slot, *One));
    LoadInstruction &Loaded = Builder.createLoad(Slot);
    static_cast<void>(Builder.createBranch(Loop));
    Builder.setInsertPoint(Loop);
    PhiInstruction &Phi = Builder.createPhi(Int32);
    AddInstruction &Next = Builder.createAdd(Phi, *One);
    static_cast<void>(Builder.createCondBranch(FunctionValue.argument(0), Loop, Exit));
    Phi.addIncoming(Loaded, Entry);
    Phi.addIncoming(Next, Loop);
    Builder.setInsertPoint(Exit);
    static_cast<void>(Builder.createReturn(Phi));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    ASSERT_EQ(Parsed->module().functionCount(), 1U);
    const Function &Copy = *Parsed->module().functions().begin();
    ASSERT_EQ(Copy.basicBlockCount(), 3U);
    const BasicBlock &EntryCopy = *Copy.basicBlocks().begin();
    const BasicBlock &ExitCopy = *std::next(Copy.basicBlocks().begin());
    const BasicBlock &LoopCopy = *std::next(Copy.basicBlocks().begin(), 2);
    EXPECT_EQ(EntryCopy.name(), Names.intern("Entry"));
    EXPECT_EQ(ExitCopy.name(), Names.intern("Exit"));
    EXPECT_EQ(LoopCopy.name(), Names.intern("Loop"));
    ASSERT_EQ(EntryCopy.instructionCount(), 4U);
    auto EntryPosition = EntryCopy.instructions().begin();
    ASSERT_TRUE(AllocaInstruction::classof(&*EntryPosition));
    const auto &SlotCopy = static_cast<const AllocaInstruction &>(*EntryPosition++);
    ASSERT_TRUE(StoreInstruction::classof(&*EntryPosition));
    const auto &StoreCopy = static_cast<const StoreInstruction &>(*EntryPosition++);
    ASSERT_TRUE(LoadInstruction::classof(&*EntryPosition));
    const auto &LoadCopy = static_cast<const LoadInstruction &>(*EntryPosition++);
    EXPECT_EQ(&StoreCopy.getDestination(), &SlotCopy);
    EXPECT_EQ(&LoadCopy.getSource(), &SlotCopy);
    EXPECT_EQ(&EntryCopy.successor(0), &LoopCopy);
    ASSERT_EQ(LoopCopy.instructionCount(), 3U);
    auto LoopPosition = LoopCopy.instructions().begin();
    ASSERT_TRUE(PhiInstruction::classof(&*LoopPosition));
    const auto &PhiCopy = static_cast<const PhiInstruction &>(*LoopPosition++);
    ASSERT_TRUE(AddInstruction::classof(&*LoopPosition));
    const auto &NextCopy = static_cast<const AddInstruction &>(*LoopPosition++);
    EXPECT_EQ(&NextCopy.getLHS(), &PhiCopy);
    ASSERT_EQ(PhiCopy.incomingCount(), 2U);
    EXPECT_EQ(&PhiCopy.incomingValue(0), &LoadCopy);
    EXPECT_EQ(&PhiCopy.incomingBlock(0), &EntryCopy);
    EXPECT_EQ(&PhiCopy.incomingValue(1), &NextCopy);
    EXPECT_EQ(&PhiCopy.incomingBlock(1), &LoopCopy);
    EXPECT_EQ(&LoopCopy.successor(0), &LoopCopy);
    EXPECT_EQ(&LoopCopy.successor(1), &ExitCopy);
    EXPECT_EQ(static_cast<const ReturnInstruction *>(ExitCopy.terminator())->getReturnValue(), &PhiCopy);
    EXPECT_TRUE(Copy.hasValidControlFlow());
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Duplicate conditional edges retain both phi pairs and reuse the identical incoming value after parsing.
  TEST(IRModuleTextTest, PreservesRepeatedPredecessorEdges)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *One = Constants.getInteger(Int32, 1);
    ASSERT_NE(One, nullptr);
    Module Original;
    Function &FunctionValue = Original.createFunction(Types.getFunctionType(Int32, std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    BasicBlock &Merge = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createCondBranch(FunctionValue.argument(0), Merge, Merge));
    Builder.setInsertPoint(Merge);
    PhiInstruction &Phi = Builder.createPhi(Int32);
    Phi.addIncoming(*One, Entry);
    Phi.addIncoming(*One, Entry);
    static_cast<void>(Builder.createReturn(Phi));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const Function &Copy = *Parsed->module().functions().begin();
    const BasicBlock &EntryCopy = *Copy.basicBlocks().begin();
    const BasicBlock &MergeCopy = *std::next(Copy.basicBlocks().begin());
    const auto &PhiCopy = static_cast<const PhiInstruction &>(*MergeCopy.instructions().begin());
    ASSERT_EQ(PhiCopy.incomingCount(), 2U);
    EXPECT_EQ(&PhiCopy.incomingValue(0), &PhiCopy.incomingValue(1));
    EXPECT_EQ(&PhiCopy.incomingBlock(0), &EntryCopy);
    EXPECT_EQ(&PhiCopy.incomingBlock(1), &EntryCopy);
    EXPECT_EQ(&EntryCopy.successor(0), &MergeCopy);
    EXPECT_EQ(&EntryCopy.successor(1), &MergeCopy);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Integer signedness and all supported widths retain their exact stored bits, including the entire uint64 range.
  TEST(IRModuleTextTest, PreservesIntegerConstantBitsForEveryWidthAndSignedness)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    Module Original;
    const IntegerType::Width Widths[] = {
        IntegerType::Width::Bits8,
        IntegerType::Width::Bits16,
        IntegerType::Width::Bits32,
        IntegerType::Width::Bits64,
    };
    const IntegerType::Signedness Signs[] = {
        IntegerType::Signedness::Signed,
        IntegerType::Signedness::Unsigned,
    };
    InstructionBuilder Builder(Types);
    std::vector<const IntegerConstant *> Expected;
    for (IntegerType::Width Width : Widths)
    {
      for (IntegerType::Signedness Sign : Signs)
      {
        const IntegerType &TypeValue = Types.getIntegerType(Width, Sign);
        const std::uint64_t Maximum = TypeValue.bitWidth() == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << TypeValue.bitWidth()) - 1;
        for (std::uint64_t Bits : {std::uint64_t{0}, Maximum})
        {
          const IntegerConstant *ConstantValue = Constants.getInteger(TypeValue, Bits);
          ASSERT_NE(ConstantValue, nullptr);
          Expected.push_back(ConstantValue);
          Function &FunctionValue = Original.createFunction(Types.getFunctionType(TypeValue));
          Builder.setInsertPoint(FunctionValue.createBasicBlock());
          static_cast<void>(Builder.createReturn(*ConstantValue));
        }
      }
    }
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    ASSERT_EQ(Parsed->module().functionCount(), Expected.size());
    std::size_t Index = 0;
    for (const Function &Copy : Parsed->module().functions())
    {
      const Value *Returned = static_cast<const ReturnInstruction *>(Copy.basicBlocks().begin()->terminator())->getReturnValue();
      ASSERT_NE(Returned, nullptr);
      ASSERT_EQ(Returned->valueKind(), ValueKind::IntegerConstant);
      const auto &ConstantCopy = static_cast<const IntegerConstant &>(*Returned);
      const auto &TypeCopy = static_cast<const IntegerType &>(ConstantCopy.getType());
      const auto &ExpectedType = static_cast<const IntegerType &>(Expected[Index]->getType());
      EXPECT_EQ(ConstantCopy.bits(), Expected[Index]->bits());
      EXPECT_EQ(TypeCopy.bitWidth(), ExpectedType.bitWidth());
      EXPECT_EQ(TypeCopy.signedness(), ExpectedType.signedness());
      ++Index;
    }
  }

  // IEEE encodings preserve positive/negative zero, infinities, and NaN payloads without decimal conversion.
  TEST(IRModuleTextTest, PreservesFloatSpecialValueBitPatterns)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    Module Original;
    InstructionBuilder Builder(Types);
    const std::pair<FloatType::Width, std::uint64_t> Values[] = {
        {FloatType::Width::Bits32, 0x00000000ULL},
        {FloatType::Width::Bits32, 0x80000000ULL},
        {FloatType::Width::Bits32, 0x7f800000ULL},
        {FloatType::Width::Bits32, 0x7fc12345ULL},
        {FloatType::Width::Bits64, 0x0000000000000000ULL},
        {FloatType::Width::Bits64, 0x8000000000000000ULL},
        {FloatType::Width::Bits64, 0xfff0000000000000ULL},
        {FloatType::Width::Bits64, 0x7ff8123456789abcULL},
    };
    for (const auto &[Width, Bits] : Values)
    {
      const FloatType &TypeValue = Types.getFloatType(Width);
      const FloatConstant *ConstantValue = Constants.getFloatBits(TypeValue, Bits);
      ASSERT_NE(ConstantValue, nullptr);
      Function &FunctionValue = Original.createFunction(Types.getFunctionType(TypeValue));
      Builder.setInsertPoint(FunctionValue.createBasicBlock());
      static_cast<void>(Builder.createReturn(*ConstantValue));
    }
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    ASSERT_EQ(Parsed->module().functionCount(), std::size(Values));
    std::size_t Index = 0;
    for (const Function &Copy : Parsed->module().functions())
    {
      const Value *Returned = static_cast<const ReturnInstruction *>(Copy.basicBlocks().begin()->terminator())->getReturnValue();
      ASSERT_NE(Returned, nullptr);
      ASSERT_EQ(Returned->valueKind(), ValueKind::FloatConstant);
      const auto &ConstantCopy = static_cast<const FloatConstant &>(*Returned);
      EXPECT_EQ(ConstantCopy.bits(), Values[Index].second);
      EXPECT_EQ(static_cast<const FloatType &>(ConstantCopy.getType()).bitWidth(), static_cast<std::uint32_t>(Values[Index].first));
      ++Index;
    }
  }

  // Strings preserve zero length and every possible byte, with array lengths independent of trailing NUL conventions.
  TEST(IRModuleTextTest, PreservesEmptyAndArbitraryByteStringConstants)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    Module Original;
    std::string AllBytes;
    for (unsigned Byte = 0; Byte < 256; ++Byte)
    {
      AllBytes.push_back(static_cast<char>(Byte));
    }
    const std::array<std::string_view, 2> Values{std::string_view{}, std::string_view(AllBytes)};
    InstructionBuilder Builder(Types);
    for (std::string_view Bytes : Values)
    {
      const StringConstant &ConstantValue = Constants.getString(Bytes);
      Function &FunctionValue = Original.createFunction(Types.getFunctionType(ConstantValue.getType()));
      Builder.setInsertPoint(FunctionValue.createBasicBlock());
      static_cast<void>(Builder.createReturn(ConstantValue));
    }
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    ASSERT_EQ(Parsed->module().functionCount(), Values.size());
    std::size_t Index = 0;
    for (const Function &Copy : Parsed->module().functions())
    {
      const Value *Returned = static_cast<const ReturnInstruction *>(Copy.basicBlocks().begin()->terminator())->getReturnValue();
      ASSERT_NE(Returned, nullptr);
      ASSERT_EQ(Returned->valueKind(), ValueKind::StringConstant);
      const auto &ConstantCopy = static_cast<const StringConstant &>(*Returned);
      EXPECT_EQ(ConstantCopy.value(), Values[Index]);
      EXPECT_EQ(static_cast<const ArrayType &>(ConstantCopy.getType()).elementCount(), Values[Index].size());
      ++Index;
    }
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // ABI, calling convention, C ellipsis, and fixed parameter kinds remain distinct signature metadata after parsing.
  TEST(IRModuleTextTest, PreservesCompleteFunctionSignatureMetadata)
  {
    TypePool Types;
    Module Original;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const CallingConvention Conventions[] = {
        CallingConvention::Default,
        CallingConvention::CDecl,
        CallingConvention::StdCall,
        CallingConvention::FastCall,
        CallingConvention::VectorCall,
    };
    for (ABIKind ABI : {ABIKind::Ink, ABIKind::C})
    {
      for (CallingConvention Convention : Conventions)
      {
        for (bool Variadic : {false, true})
        {
          const std::array Parameters{Parameter(Int32), Parameter(Types.getBoolType(), Parameter::Kind::Named)};
          Original.createFunction(Types.getFunctionType(Int32, Parameters, CallABI{ABI, Convention}, Variadic));
        }
      }
    }
    Original.createFunction(Types.getFunctionType(Types.getVoidType(), {}, CallABI{ABIKind::C, CallingConvention::CDecl}, true));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    ASSERT_EQ(Parsed->module().functionCount(), Original.functionCount());
    auto Expected = Original.functions().begin();
    for (const Function &Copy : Parsed->module().functions())
    {
      EXPECT_EQ(Copy.signature().callABI(), Expected->signature().callABI());
      EXPECT_EQ(Copy.signature().isVariadic(), Expected->signature().isVariadic());
      ASSERT_EQ(Copy.argumentCount(), Expected->argumentCount());
      for (std::size_t Index = 0; Index < Copy.argumentCount(); ++Index)
      {
        EXPECT_EQ(Copy.argument(Index).kind(), Expected->argument(Index).kind());
      }
      ++Expected;
    }
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Type-valued arguments remain shared identity edges in nominal fields, composite types, and dependent function declarations.
  TEST(IRModuleTextTest, ResolvesGenericArgumentDependenciesAcrossDeclarations)
  {
    TypePool Types;
    Module Original;
    Function &TypeOwner = Original.createFunction(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    TypeOwner.argument(0).setName(core::NamePool::instance().intern("T"));
    Argument &T = TypeOwner.argument(0);
    StructType &Record = Types.createStructType();
    Original.addStructType(Record);
    Record.addBase(T);
    Record.addField(core::NAME_None, T);
    Record.addField(core::NAME_None, Types.getArrayType(Types.getPointerType(T), 7));
    Original.createFunction(Types.getFunctionType(T, std::array{Parameter(Types.getPointerType(T), Parameter::Kind::Named)}));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    ASSERT_EQ(Parsed->module().functionCount(), 2U);
    const Function &OwnerCopy = *Parsed->module().functions().begin();
    const Function &DependentCopy = *std::next(Parsed->module().functions().begin());
    const Argument &TCopy = OwnerCopy.argument(0);
    EXPECT_TRUE(TCopy.isTypeValue());
    EXPECT_EQ(TCopy.name(), TypeOwner.argument(0).name());
    const StructType &RecordCopy = *Parsed->module().structTypes().begin();
    ASSERT_EQ(RecordCopy.bases().size(), 1U);
    EXPECT_EQ(RecordCopy.bases()[0], &TCopy);
    ASSERT_EQ(RecordCopy.fields().size(), 2U);
    EXPECT_EQ(&RecordCopy.fields()[0].type(), &TCopy);
    const auto &ArrayCopy = static_cast<const ArrayType &>(RecordCopy.fields()[1].type());
    EXPECT_EQ(ArrayCopy.elementCount(), 7U);
    EXPECT_EQ(&static_cast<const PointerType &>(ArrayCopy.elementType()).pointeeType(), &TCopy);
    EXPECT_EQ(&DependentCopy.signature().returnType(), &TCopy);
    EXPECT_EQ(&static_cast<const PointerType &>(DependentCopy.argument(0).getType()).pointeeType(), &TCopy);
    EXPECT_EQ(DependentCopy.argument(0).kind(), Parameter::Kind::Named);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // A type-producing load can supply allocation metadata and nested pointer types without losing its SSA identity.
  TEST(IRModuleTextTest, ResolvesTypeProducingInstructionsInTypeExpressions)
  {
    TypePool Types;
    Module Original;
    Function &FunctionValue = Original.createFunction(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getPointerType(Types.getMetaType()))}));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(FunctionValue.createBasicBlock());
    LoadInstruction &T = Builder.createLoad(FunctionValue.argument(0));
    static_cast<void>(Builder.createAlloca(T));
    static_cast<void>(Builder.createAlloca(Types.getPointerType(T)));
    static_cast<void>(Builder.createReturn());
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const Function &Copy = *Parsed->module().functions().begin();
    const BasicBlock &Block = *Copy.basicBlocks().begin();
    ASSERT_EQ(Block.instructionCount(), 4U);
    auto Position = Block.instructions().begin();
    const auto &TCopy = static_cast<const LoadInstruction &>(*Position++);
    const auto &AllocationCopy = static_cast<const AllocaInstruction &>(*Position++);
    const auto &NestedCopy = static_cast<const AllocaInstruction &>(*Position++);
    EXPECT_TRUE(TCopy.isTypeValue());
    EXPECT_EQ(&AllocationCopy.getAllocatedType(), &TCopy);
    EXPECT_EQ(&static_cast<const PointerType &>(NestedCopy.getAllocatedType()).pointeeType(), &TCopy);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Forward function declarations, void calls, and C variadic calls retain callee and argument identities across owning deserialization.
  TEST(IRModuleTextTest, RoundTripsDirectVoidAndVariadicCalls)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType &Int64 = Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Signed);
    IntegerConstant Seven(Int32, 7);
    IntegerConstant Extra(Int64, 9);
    Module Original;
    Function &Caller = Original.createFunction(Types.getFunctionType(Int32));
    Function &Echo = Original.createFunction(Types.getFunctionType(Int32, std::array{Parameter(Int32)}));
    Function &Sink = Original.createFunction(Types.getFunctionType(Types.getVoidType()));
    Function &Variadic = Original.createFunction(Types.getFunctionType(Int32, std::array{Parameter(Int32)}, {ABIKind::C, CallingConvention::CDecl}, true));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Caller.createBasicBlock());
    CallInstruction &First = Builder.createCall(Echo, std::array<ValueRef, 1>{Seven});
    static_cast<void>(Builder.createCall(Sink));
    CallInstruction &Last = Builder.createCall(Variadic, std::array<ValueRef, 2>{First, Extra});
    static_cast<void>(Builder.createReturn(Last));
    std::string Text;
    auto Parsed = roundTrip(Original, &Text);
    ASSERT_NE(Parsed, nullptr);
    EXPECT_NE(Text.find("call @f2(): void;"), std::string::npos);
    auto FunctionPosition = Parsed->module().functions().begin();
    const Function &CallerCopy = *FunctionPosition++;
    const Function &EchoCopy = *FunctionPosition++;
    const Function &SinkCopy = *FunctionPosition++;
    const Function &VariadicCopy = *FunctionPosition++;
    const BasicBlock &Block = *CallerCopy.basicBlocks().begin();
    ASSERT_EQ(Block.instructionCount(), 4U);
    auto Position = Block.instructions().begin();
    const auto &FirstCopy = static_cast<const CallInstruction &>(*Position++);
    const auto &VoidCopy = static_cast<const CallInstruction &>(*Position++);
    const auto &LastCopy = static_cast<const CallInstruction &>(*Position++);
    EXPECT_EQ(&FirstCopy.getCallee(), &EchoCopy);
    EXPECT_EQ(FirstCopy.argumentCount(), 1U);
    EXPECT_EQ(static_cast<const IntegerConstant &>(FirstCopy.argument(0)).bits(), 7U);
    EXPECT_EQ(&VoidCopy.getCallee(), &SinkCopy);
    EXPECT_EQ(VoidCopy.argumentCount(), 0U);
    EXPECT_TRUE(VoidType::classof(&VoidCopy.getType()));
    EXPECT_EQ(&LastCopy.getCallee(), &VariadicCopy);
    EXPECT_TRUE(LastCopy.signature().isVariadic());
    EXPECT_EQ(LastCopy.argumentCount(), 2U);
    EXPECT_EQ(&LastCopy.argument(0), &FirstCopy);
    EXPECT_EQ(static_cast<const IntegerConstant &>(LastCopy.argument(1)).bits(), 9U);
    EXPECT_EQ(static_cast<const ReturnInstruction &>(*Position).getReturnValue(), &LastCopy);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Direct method calls and function-valued arguments preserve the same explicit object receiver without inserting an extra argument.
  TEST(IRModuleTextTest, RoundTripsMethodAndIndirectCallsWithExplicitReceivers)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    ObjectType &Object = Types.createObjectType();
    const FunctionType &MethodSignature = Types.getFunctionType(Int32, std::array{Parameter(Object), Parameter(Int32)});
    Function &MethodFunction = Object.addMethod(MethodSignature, core::NamePool::instance().intern("apply"), MemberKind::Instance, SymbolVisibility::Public).function();
    Module Original;
    Original.addObjectType(Object);
    Function &Caller = Original.createFunction(Types.getFunctionType(Int32, std::array{Parameter(Object), Parameter(MethodSignature), Parameter(Int32)}));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Caller.createBasicBlock());
    CallInstruction &Direct = Builder.createCall(MethodFunction, std::array<ValueRef, 2>{Caller.argument(0), Caller.argument(2)});
    CallInstruction &Indirect = Builder.createCall(Caller.argument(1), std::array<ValueRef, 2>{Caller.argument(0), Direct});
    static_cast<void>(Builder.createReturn(Indirect));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const ObjectType &ObjectCopy = *Parsed->module().objectTypes().begin();
    const Function &CallerCopy = *Parsed->module().functions().begin();
    auto Position = CallerCopy.basicBlocks().begin()->instructions().begin();
    const auto &DirectCopy = static_cast<const CallInstruction &>(*Position++);
    const auto &IndirectCopy = static_cast<const CallInstruction &>(*Position++);
    EXPECT_EQ(&DirectCopy.getCallee(), &ObjectCopy.methods()[0].function());
    EXPECT_EQ(DirectCopy.argumentCount(), 2U);
    EXPECT_EQ(&DirectCopy.argument(0), &CallerCopy.argument(0));
    EXPECT_EQ(&DirectCopy.argument(0).getType(), &ObjectCopy);
    EXPECT_EQ(&IndirectCopy.getCallee(), &CallerCopy.argument(1));
    EXPECT_EQ(&IndirectCopy.argument(0), &CallerCopy.argument(0));
    EXPECT_EQ(&IndirectCopy.argument(1), &DirectCopy);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Self calls can return type-valued results that later instructions use as allocation metadata.
  TEST(IRModuleTextTest, RoundTripsRecursiveCallsAndTypeValuedResults)
  {
    TypePool Types;
    Module Original;
    Function &Recursive = Original.createFunction(Types.getFunctionType(Types.getMetaType()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Recursive.createBasicBlock());
    CallInstruction &T = Builder.createCall(Recursive);
    static_cast<void>(Builder.createAlloca(T));
    static_cast<void>(Builder.createReturn(T));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const Function &Copy = *Parsed->module().functions().begin();
    auto Position = Copy.basicBlocks().begin()->instructions().begin();
    const auto &Call = static_cast<const CallInstruction &>(*Position++);
    const auto &Allocation = static_cast<const AllocaInstruction &>(*Position++);
    EXPECT_EQ(&Call.getCallee(), &Copy);
    EXPECT_TRUE(Call.isTypeValue());
    EXPECT_EQ(&Allocation.getAllocatedType(), &Call);
    EXPECT_EQ(static_cast<const ReturnInstruction &>(*Position).getReturnValue(), &Call);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // A function-valued call result remains a valid callee when its dominating producer block appears later in textual block order.
  TEST(IRModuleTextTest, ResolvesInstructionCalleesAcrossBlockOrder)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType &CallbackSignature = Types.getFunctionType(Int32);
    Module Original;
    Function &Factory = Original.createFunction(Types.getFunctionType(CallbackSignature));
    Function &Caller = Original.createFunction(CallbackSignature);
    BasicBlock &Entry = Caller.createBasicBlock();
    BasicBlock &Consumer = Caller.createBasicBlock();
    BasicBlock &Producer = Caller.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createBranch(Producer));
    Builder.setInsertPoint(Producer);
    CallInstruction &Callback = Builder.createCall(Factory);
    static_cast<void>(Builder.createBranch(Consumer));
    Builder.setInsertPoint(Consumer);
    CallInstruction &Result = Builder.createCall(Callback);
    static_cast<void>(Builder.createReturn(Result));
    auto Parsed = roundTrip(Original);
    ASSERT_NE(Parsed, nullptr);
    const Function &CallerCopy = *std::next(Parsed->module().functions().begin());
    auto BlockPosition = CallerCopy.basicBlocks().begin();
    const BasicBlock &ConsumerCopy = *++BlockPosition;
    const BasicBlock &ProducerCopy = *++BlockPosition;
    const auto &ResultCopy = static_cast<const CallInstruction &>(*ConsumerCopy.instructions().begin());
    const auto &CallbackCopy = static_cast<const CallInstruction &>(*ProducerCopy.instructions().begin());
    EXPECT_EQ(&ResultCopy.getCallee(), &CallbackCopy);
    verifyModule(Parsed->module(), core::DiagnosticEngine{});
  }

  // Invalid callable types, fixed/variadic arguments, return annotations, result IDs, and list syntax produce recoverable diagnostics.
  TEST(IRModuleTextTest, RejectsInvalidCallSignaturesAndArgumentLists)
  {
    constexpr std::string_view InvalidTexts[] = {
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call int<i32>(0)(): void; return; } })",
        R"(module "" { define private none void @f0 name ""(positional ptr<fn<ink default fixed>() -> void> %a0 name "") abi ink cc default fixed { %bb0 name "": call %a0(): void; return; } })",
        R"(module "" { declare private none i32 @callee name ""() abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(): i32; return; } })",
        R"(module "" { declare private none void @callee name ""() abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = call @callee(): void; return; } })",
        R"(module "" { declare private none i32 @callee name ""() abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = call @callee(): i64; return; } })",
        R"(module "" { declare private none void @callee name ""(positional i32 %a0 name "") abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(): void; return; } })",
        R"(module "" { declare private none void @callee name ""() abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(int<i32>(0)): void; return; } })",
        R"(module "" { declare private none void @callee name ""(positional i32 %a0 name "") abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(int<i64>(0)): void; return; } })",
        R"(module "" { declare private none void @callee name ""(positional i32 %a0 name "") abi c cc cdecl variadic; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(): void; return; } })",
        R"(module "" { declare private none void @callee name ""(positional i32 %a0 name "") abi c cc cdecl variadic; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(int<i64>(0)): void; return; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @missing(): void; return; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call (): void; return; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @f0: void; return; } })",
        R"(module "" { declare private none void @callee name ""(positional i32 %a0 name "") abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(int<i32>(0),): void; return; } })",
        R"(module "" { declare private none void @callee name ""(positional i32 %a0 name "", positional i32 %a1 name "") abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call @callee(int<i32>(0) int<i32>(1)): void; return; } })",
        R"(module "" { define private none void @f0 name ""(positional fn<ink default fixed>() -> void %a0 name "") abi ink cc default fixed { %bb0 name "": return; } define private none void @f1 name ""() abi ink cc default fixed { %bb1 name "": call %a0(): void; return; } })",
        R"(module "" { declare private none fn<ink default fixed>() -> void @factory name ""() abi ink cc default fixed; define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": call %v0(): void; %v0 = call @factory(): fn<ink default fixed>() -> void; return; } })",
    };
    for (std::string_view Text : InvalidTexts)
    {
      SCOPED_TRACE(Text);
      expectReadError(Text);
    }
  }

  // Truncation, trailing tokens, malformed strings, and unknown instruction keywords return ordinary Core diagnostics.
  TEST(IRModuleTextTest, RejectsMalformedTextWithoutFatalVerification)
  {
    TypePool Types;
    Module Original;
    Function &FunctionValue = Original.createFunction(Types.getFunctionType(Types.getVoidType()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(FunctionValue.createBasicBlock());
    static_cast<void>(Builder.createReturn());
    auto Written = writeModuleText(Original);
    ASSERT_TRUE(Written.has_value());
    expectReadError("");
    expectReadError(*Written + " unexpected");
    expectReadError(Written->substr(0, Written->find_last_of('}')));
    std::string UnknownInstruction = *Written;
    const std::size_t ReturnPosition = UnknownInstruction.find("return");
    ASSERT_NE(ReturnPosition, std::string::npos);
    UnknownInstruction.replace(ReturnPosition, 6, "unknown_instruction");
    expectReadError(UnknownInstruction);
    expectReadError("module @\"unterminated");
  }

  // Undefined and duplicate value, function, and block identifiers are diagnosed without constructing dangling references.
  TEST(IRModuleTextTest, RejectsUndefinedAndDuplicateIdentifiers)
  {
    constexpr std::string_view InvalidTexts[] = {
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": return %missing; } })",
        R"(module "" { declare private none void @f0 name ""() abi ink cc default fixed; declare private none void @f0 name ""() abi ink cc default fixed; })",
        R"(module "" { define private none i32 @f0 name ""(positional i32 %v0 name "") abi ink cc default fixed { %bb0 name "": %v0 = add int<i32>(1), int<i32>(2): i32; return %v0; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": branch %missing; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": return; %bb0 name "": return; } })",
        R"(module "" { %t0 = type struct {} %t0 = type class {} })",
    };
    for (std::string_view Text : InvalidTexts)
    {
      SCOPED_TRACE(Text);
      expectReadError(Text);
    }
  }

  // Incompatible operand/result types, void parameters, non-pointer globals, and overflowing constant bits are recoverable input errors.
  TEST(IRModuleTextTest, RejectsInvalidTypesAndOutOfRangeConstants)
  {
    constexpr std::string_view InvalidTexts[] = {
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = add int<i32>(1), int<i64>(2): i32; return %v0; } })",
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = add int<i32>(1), int<i32>(2): i64; return int<i32>(0); } })",
        R"(module "" { declare private none void @f0 name ""(positional void %a0 name "") abi ink cc default fixed; })",
        R"(module "" { global private none @g0 name "": i32; })",
        R"(module "" { define private none u8 @f0 name ""() abi ink cc default fixed { %bb0 name "": return int<u8>(256); } })",
        R"(module "" { define private none f32 @f0 name ""() abi ink cc default fixed { %bb0 name "": return float<f32>(4294967296); } })",
        R"(module "" { %t0 = type struct { field public instance "Bad": int<i32>(1); } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = phi: void; return; } })",
    };
    for (std::string_view Text : InvalidTexts)
    {
      SCOPED_TRACE(Text);
      expectReadError(Text);
    }
  }

  // Missing terminators, post-terminator instructions, local forward uses, and invalid phi edges are rejected by nonfatal graph validation.
  TEST(IRModuleTextTest, RejectsInvalidControlFlowAndSsa)
  {
    constexpr std::string_view InvalidTexts[] = {
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = add int<i32>(1), int<i32>(2): i32; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": return; %v0 = add int<i32>(1), int<i32>(2): i32; } })",
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": %v0 = add %v1, int<i32>(1): i32; %v1 = add int<i32>(1), int<i32>(2): i32; return %v0; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": return; } define private none void @f1 name ""() abi ink cc default fixed { %bb1 name "": branch %bb0; } })",
        R"(module "" { define private none i32 @f0 name ""(positional i32 %a0 name "") abi ink cc default fixed { %bb0 name "": return %a0; } define private none i32 @f1 name ""() abi ink cc default fixed { %bb1 name "": return %a0; } })",
        R"(module "" { define private none void @f0 name ""() abi ink cc default fixed { %bb0 name "": branch %bb0; } })",
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": branch %bb1; %bb1 name "": %v0 = phi: i32; return %v0; } })",
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": branch %bb1; %bb1 name "": %v0 = phi [int<i64>(1), %bb0]: i32; return %v0; } })",
        R"(module "" { define private none i32 @f0 name ""(positional bool %a0 name "") abi ink cc default fixed { %bb0 name "": cond_branch %a0, %bb1, %bb2; %bb1 name "": %v0 = add int<i32>(1), int<i32>(2): i32; branch %bb3; %bb2 name "": %v1 = add %v0, int<i32>(1): i32; branch %bb3; %bb3 name "": return int<i32>(0); } })",
    };
    for (std::string_view Text : InvalidTexts)
    {
      SCOPED_TRACE(Text);
      expectReadError(Text);
    }
  }

  // Reader accepts whitespace and comments between tokens while canonical output removes layout-only differences.
  TEST(IRModuleTextTest, CanonicalizesWhitespaceAndLineComments)
  {
    constexpr std::string_view Input = R"(// module heading
module "Layout" { // declarations
declare public import void @decl name "native" ( ) abi c cc cdecl fixed ;
} // end
)";
    auto Parsed = readModuleText(Input);
    ASSERT_TRUE(Parsed.has_value()) << describeDiagnostic(Parsed.error());
    ASSERT_EQ((*Parsed)->module().functionCount(), 1U);
    const Function &Declaration = *(*Parsed)->module().functions().begin();
    EXPECT_EQ(Declaration.name(), core::NamePool::instance().intern("native"));
    EXPECT_EQ(Declaration.nativeLinkage(), NativeLinkage::Import);
    EXPECT_EQ(Declaration.abi(), ABIKind::C);
    auto Written = writeModuleText((*Parsed)->module());
    ASSERT_TRUE(Written.has_value());
    EXPECT_EQ(*Written, "module \"Layout\"\n{\n  declare public import void @f0 name \"native\"() abi c cc cdecl fixed;\n}\n");
  }

  // Quoted byte escapes and numeric tokens reject malformed or overflowing encodings before graph construction.
  TEST(IRModuleTextTest, RejectsInvalidEscapesAndUnsignedNumericOverflow)
  {
    constexpr std::string_view InvalidTexts[] = {
        R"(module "\GG" {})",
        R"(module "\0" {})",
        R"(module "\x00" {})",
        "module \"line\nbreak\" {}",
        R"(module "" { define private none u64 @f0 name ""() abi ink cc default fixed { %bb0 name "": return int<u64>(18446744073709551616); } })",
        R"(module "" { define private none u64 @f0 name ""() abi ink cc default fixed { %bb0 name "": return int<u64>(0x10000000000000000); } })",
        R"(module "" { %t0 = type struct { field public instance "": i24; } })",
        R"(module "" { %t0 = type struct { field public instance "": f16; } })",
    };
    for (std::string_view Text : InvalidTexts)
    {
      SCOPED_TRACE(Text);
      expectReadError(Text);
    }
  }

  // A maximal hexadecimal uint64 constant is accepted and canonicalized without signed conversion or bit loss.
  TEST(IRModuleTextTest, AcceptsTheFullUnsignedHexadecimalRange)
  {
    constexpr std::string_view Input = R"(module "" { define private none u64 @f0 name ""() abi ink cc default fixed { %bb0 name "": return int<u64>(0xffffffffffffffff); } })";
    auto Parsed = readModuleText(Input);
    ASSERT_TRUE(Parsed.has_value()) << describeDiagnostic(Parsed.error());
    const Function &FunctionValue = *(*Parsed)->module().functions().begin();
    const Value *Returned = static_cast<const ReturnInstruction *>(FunctionValue.basicBlocks().begin()->terminator())->getReturnValue();
    ASSERT_NE(Returned, nullptr);
    ASSERT_EQ(Returned->valueKind(), ValueKind::IntegerConstant);
    EXPECT_EQ(static_cast<const IntegerConstant &>(*Returned).bits(), std::numeric_limits<std::uint64_t>::max());
    auto Written = writeModuleText((*Parsed)->module());
    ASSERT_TRUE(Written.has_value());
    EXPECT_NE(Written->find("18446744073709551615"), std::string::npos);
    auto Reparsed = readModuleText(*Written);
    ASSERT_TRUE(Reparsed.has_value());
  }

  // Parameter and phi lists require separators and reject incomplete or trailing elements rather than silently truncating the list.
  TEST(IRModuleTextTest, RejectsMissingListSeparatorsAndTrailingPhiCommas)
  {
    constexpr std::string_view InvalidTexts[] = {
        R"(module "" { declare private none void @f0 name ""(positional i32 %a0 name "" positional i32 %a1 name "") abi ink cc default fixed; })",
        R"(module "" { declare private none void @f0 name ""(positional i32 %a0 name "",) abi ink cc default fixed; })",
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": branch %bb1; %bb1 name "": %v0 = phi [int<i32>(1), %bb0],: i32; return %v0; } })",
        R"(module "" { define private none i32 @f0 name ""() abi ink cc default fixed { %bb0 name "": branch %bb1; %bb1 name "": %v0 = phi [int<i32>(1) %bb0]: i32; return %v0; } })",
    };
    for (std::string_view Text : InvalidTexts)
    {
      SCOPED_TRACE(Text);
      expectReadError(Text);
    }
  }

  // Exactly 128 nested type-expression nodes can round-trip, while the next pointer layer produces a bounded diagnostic.
  TEST(IRModuleTextTest, EnforcesTheTypeExpressionNestingBoundary)
  {
    std::string TypeExpression = "i32";
    for (unsigned Index = 0; Index < 127; ++Index)
    {
      TypeExpression = "ptr<" + TypeExpression + ">";
    }
    const std::string Prefix = "module \"\" { %t0 = type struct { field public instance \"\": ";
    const std::string Suffix = "; } }";
    auto Parsed = readModuleText(Prefix + TypeExpression + Suffix);
    ASSERT_TRUE(Parsed.has_value()) << describeDiagnostic(Parsed.error());
    auto Written = writeModuleText((*Parsed)->module());
    ASSERT_TRUE(Written.has_value()) << describeDiagnostic(Written.error());
    auto Reparsed = readModuleText(*Written);
    ASSERT_TRUE(Reparsed.has_value()) << describeDiagnostic(Reparsed.error());
    expectReadError(Prefix + "ptr<" + TypeExpression + ">" + Suffix);
  }

  // External function operands are explicitly rejected because an independent parsed module cannot borrow their original owners.
  TEST(IRModuleTextTest, ReportsUnsupportedFunctionDependenciesOutsideTheModule)
  {
    TypePool Types;
    Function External(Types.getFunctionType(Types.getVoidType()));
    Module Original;
    Function &Getter = Original.createFunction(Types.getFunctionType(External.getType()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Getter.createBasicBlock());
    static_cast<void>(Builder.createReturn(External));
    auto Written = writeModuleText(Original);
    ASSERT_FALSE(Written.has_value());
    EXPECT_EQ(Written.error().Kind, core::DiagnosticKind::IRTextInvalid);
    EXPECT_FALSE(describeDiagnostic(Written.error()).empty());
  }
} // namespace ink::ir::test
