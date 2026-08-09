#ifndef INK_FRONTEND_IR_GENERATOR_H
#define INK_FRONTEND_IR_GENERATOR_H

#include "ink/core/source_range.h"
#include "ink/ir/module.h"
#include "ink/sema/verifier.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::frontend
{
  struct PendingForceValue
  {
    ir::IrPlanNodeId PlanNode;
    ir::IrFunctionId Function;
    ir::IrValueId Value;
    ir::IrValueId Output;
  };

  struct IrGenerationError
  {
    core::SourceRange Range;
    std::string Message;
  };

  struct IrGenerationResult
  {
    std::optional<ir::UnverifiedStagedModule> Module;
    std::vector<PendingForceValue> PendingForceValues;
    std::vector<IrGenerationError> Errors;

    bool succeeded() const noexcept;
  };

  class IrGenerator
  {
  public:
    explicit IrGenerator(const sema::VerifiedSemanticModule &SemanticModule) noexcept;
    IrGenerationResult generate() const;

  private:
    const sema::VerifiedSemanticModule &SemanticModule;
  };

  IrGenerationResult generateIr(const sema::VerifiedSemanticModule &SemanticModule);
} // namespace ink::frontend

#endif
