#include "ink/semantic/analyzer/analyzer.h"

#include "ink/ir/constant/string_constant.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::convertToExpectedType(AnalyzerState &State, core::SourceRange Range, const ir::Value &Value, const ir::Type &ExpectedType)
  {
    auto &Context = State.semanticContext();
    const bool Storage = Value.isStorage();
    if (ir::MetaType::classof(&ExpectedType))
    {
      RETURN_REPORT_IF(Context, Storage, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Range, "a type", "variable storage");
      return requireType(Context, Range, &Value);
    }
    RETURN_IF(!Storage && !requireRuntimeValue(Context, Range, &Value), nullptr);
    // Value compatibility drops only the object's top-level qualifier; pointee and element qualifiers remain part of the type.
    const ir::Type *Type = Value.type();
    const bool Compatible = !Storage && ir::Constant::classof(&Value) ? static_cast<const ir::Constant &>(Value).canInitialize(&ExpectedType) : Type->unqualifiedType() == ExpectedType.unqualifiedType();
    RETURN_IF(Compatible, &Value);
    if (ir::StringConstant::classof(&Value) && ir::PointerType::classof(&ExpectedType))
    {
      const ir::Type *Pointee = static_cast<const ir::PointerType &>(ExpectedType).pointeeType();
      if (!Pointee->isMutable() && Pointee->unqualifiedType() == Context.typePool().getUInt8Type())
      {
        auto &Builder = State.irBuilder();
        // Validate insertion before materializing storage; address instructions belong to this use's function and block.
        RETURN_REPORT_IF(Context, !Builder.insertFunction() || !Builder.insertBlock() || Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
        const ir::GlobalVariable *StringStorage = State.Output.getOrCreateStringStorage(static_cast<const ir::StringConstant &>(Value));
        RETURN_REPORT_IF_FALSE(Context, StringStorage, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
        const auto *Zero = Context.irContext().constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt32Type(), 0);
        RETURN_REPORT_IF_FALSE(Context, Zero, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
        const auto *Address = Builder.createAddressOf(*StringStorage);
        RETURN_REPORT_IF_FALSE(Context, Address, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
        // Empty strings retain zero-byte storage: computing its start address does not read an element.
        const auto *Pointer = Builder.createGetElementPtr(*StringStorage->type(), *Address, {Zero, Zero});
        RETURN_REPORT_IF_FALSE(Context, Pointer, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
        return Pointer;
      }
    }
    Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, ExpectedType.toString(), Type->toString());
    return nullptr;
  }
} // namespace ink::semantic
