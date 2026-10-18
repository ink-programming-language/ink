#ifndef INK_SEMANTIC_ANALYZER_EXPR_RESULT_H
#define INK_SEMANTIC_ANALYZER_EXPR_RESULT_H

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class Binding;

  enum class ExprResultKind
  {
    RValue,
    LValue,
    Type,
    Module,
  };

  // Describes a source expression independently of the IR object's pointer type or identity.
  // All pointers are borrowed. A null Value represents failure after a diagnostic.
  struct ExprResult
  {
      ExprResultKind Kind = ExprResultKind::RValue;
      const ir::Value *Value = nullptr;
      // For an lvalue this is the stored object's type, rather than the address's pointer type.
      const ir::Type *Type = nullptr;
      // Direct variable references retain their stable binding for flow checks and diagnostics.
      const Binding *Variable = nullptr;

      explicit operator bool() const noexcept
      {
        return Value != nullptr;
      }

      // Classifies descriptors and global storage; other IR values, including addresses, are rvalues.
      static ExprResult fromValue(const ir::Value *Value) noexcept;
      static ExprResult lvalue(const ir::Value &Storage, const ir::Type &Type, const Binding *Variable = nullptr) noexcept;
  };
} // namespace ink::semantic

#endif
