#ifndef INK_AST_VERIFIER_H
#define INK_AST_VERIFIER_H

#include "ink/ast/ast_context.h"
#include "ink/core/string_interner.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ink::ast
{
  enum class AstVerificationErrorKind
  {
    InvalidSourceFile,
    InvalidFileOwner,
    InvalidArenaRange,
    InvalidRoot,
    InvalidKind,
    InvalidPayload,
    InvalidEnum,
    InvalidNodeReference,
    ForeignNodeReference,
    InvalidList,
    ForeignListReference,
    InvalidRecovery,
    ForeignRecoveryReference,
    KindCategoryMismatch,
    InvalidSourceRange,
    ChildRangeOutsideParent,
    InvalidOrigin,
    DuplicateOrigin,
    DuplicateNode,
    UnreachableNode,
    InvalidString,
  };

  const char *astVerificationErrorKindName(AstVerificationErrorKind Kind) noexcept;

  struct AstVerificationError
  {
    AstVerificationErrorKind Kind = AstVerificationErrorKind::InvalidRoot;
    AstNodeRef Node;
    std::size_t ElementIndex = 0;
    std::string Message;
  };

  struct AstVerificationResult
  {
    std::vector<AstVerificationError> Errors;

    bool succeeded() const noexcept
    {
      return Errors.empty();
    }
  };

  class AstVerifier
  {
  public:
    AstVerificationResult verify(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings) const;
  };

  AstVerificationResult verifyAst(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings);
} // namespace ink::ast

#endif
