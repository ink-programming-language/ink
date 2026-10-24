#include "ink/semantic/analyzer/analyzer.h"

#include "ink/ir/constant/string_constant.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::convertToExpectedType(AnalyzerState &State, const AnalysisSite &Site, const ir::Value &Value, const ir::Type &ExpectedType)
  {
    auto &Context = State.semanticContext();
    const bool Storage = Value.isStorage();
    if (ir::MetaType::classof(&ExpectedType))
    {
      RETURN_ERROR_IF(Site, Storage, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type", "variable storage");
      return requireType(Site, &Value);
    }
    RETURN_IF(!Storage && !requireRuntimeValue(Site, &Value), nullptr);
    // Value compatibility drops only the object's top-level qualifier; pointee and element qualifiers remain part of the type.
    const ir::Type *Type = Value.type();
    const bool Compatible = !Storage && ir::Constant::classof(&Value) ? static_cast<const ir::Constant &>(Value).canInitialize(&ExpectedType) : Type->unqualifiedType() == ExpectedType.unqualifiedType();
    RETURN_IF(Compatible, &Value);
    if (ir::StringConstant::classof(&Value) && ir::PointerType::classof(&ExpectedType))
    {
      const ir::Type *Pointee = static_cast<const ir::PointerType &>(ExpectedType).pointeeType();
      if (!Pointee->isMutable() && Pointee->unqualifiedType() == Context.typePool().get<ir::IntegerType>(8, false))
      {
        auto &Builder = State.irBuilder();
        // Validate insertion before materializing storage; address instructions belong to this use's function and block.
        CHECK_ICE(Site, Builder.insertFunction() && Builder.insertBlock() && !Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed);
        const ir::GlobalVariable *StringStorage = State.Output.getOrCreateStringStorage(static_cast<const ir::StringConstant &>(Value));
        CHECK_ICE(Site, StringStorage, core::DiagnosticKind::SemanticConstructionFailed);
        const auto *Zero = Context.irContext().constantPool().get<ir::IntegerConstant>(Context.typePool().get<ir::IntegerType>(32, false), 0U);
        CHECK_ICE(Site, Zero, core::DiagnosticKind::SemanticConstructionFailed);
        const auto *Address = Builder.createAddressOf(*StringStorage);
        CHECK_ICE(Site, Address, core::DiagnosticKind::SemanticConstructionFailed);
        // Empty strings retain zero-byte storage: computing its start address does not read an element.
        const auto *Pointer = Builder.createGetElementPtr(*StringStorage->type(), *Address, {Zero, Zero});
        CHECK_ICE(Site, Pointer, core::DiagnosticKind::SemanticConstructionFailed);
        return Pointer;
      }
    }
    Site.reportError<core::DiagnosticKind::SemanticTypeMismatch>(ExpectedType.toString(), Type->toString());
    return nullptr;
  }
} // namespace ink::semantic
