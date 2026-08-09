#ifndef INK_IR_VERIFIER_H
#define INK_IR_VERIFIER_H

#include "ink/ir/module.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ink::ir
{
  enum class IrVerificationErrorCode : std::uint8_t
  {
    InvalidTableReference,
    InvalidOwner,
    InvalidType,
    InvalidConstant,
    InvalidPayload,
    InvalidArity,
    InvalidFunction,
    InvalidBlock,
    InvalidOperation,
    MissingTerminator,
    OperationAfterTerminator,
    InvalidControlFlow,
    InvalidStage,
    UseBeforeDefinition,
    NonDominatingValue,
  };

  struct IrVerificationError
  {
    IrVerificationErrorCode Code = IrVerificationErrorCode::InvalidOperation;
    std::string Message;
    IrFunctionId Function;
    IrBlockId Block;
    IrOperationId Operation;
    IrValueId Value;
  };

  class IrStagedVerificationResult
  {
  public:
    bool succeeded() const noexcept;
    const std::vector<IrVerificationError> &errors() const noexcept;
    const VerifiedStagedModule &verified() const;
    VerifiedStagedModule takeVerified();

  private:
    IrStagedVerificationResult(std::optional<VerifiedStagedModule> Verified, std::vector<IrVerificationError> Errors);

    std::optional<VerifiedStagedModule> Verified;
    std::vector<IrVerificationError> Errors;

    friend IrStagedVerificationResult verifyStaged(const UnverifiedStagedModule &Module);
  };

  class IrClosedVerificationResult
  {
  public:
    bool succeeded() const noexcept;
    const std::vector<IrVerificationError> &errors() const noexcept;
    const VerifiedClosedModule &verified() const;
    VerifiedClosedModule takeVerified();

  private:
    IrClosedVerificationResult(std::optional<VerifiedClosedModule> Verified, std::vector<IrVerificationError> Errors);

    std::optional<VerifiedClosedModule> Verified;
    std::vector<IrVerificationError> Errors;

    friend IrClosedVerificationResult closeAndVerify(const VerifiedStagedModule &Module, target::TargetKey TargetKey, const std::vector<IrForceValueResolution> &Resolutions);
  };

  std::vector<IrVerificationError> verifyFunction(const IrModule &Module, IrFunctionId Function, IrStage Stage);
  std::vector<IrVerificationError> verifyModule(const IrModule &Module, IrStage Stage);
  IrStagedVerificationResult verifyStaged(const UnverifiedStagedModule &Module);
  IrClosedVerificationResult closeAndVerify(const VerifiedStagedModule &Module, target::TargetKey TargetKey, const std::vector<IrForceValueResolution> &Resolutions = {});
} // namespace ink::ir

#endif
