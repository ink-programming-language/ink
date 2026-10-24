#ifndef INK_SEMANTIC_ANALYZER_EXPR_RESULT_H
#define INK_SEMANTIC_ANALYZER_EXPR_RESULT_H

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  // Borrows an expression's IR value or object address. Location/value classification belongs to this use, not the IR value.
  // A direct declaration is retained only for declaration-specific diagnostics; indirect locations have no declaration state.
  class ExprResult final
  {
    public:
      ExprResult(const ir::Value *Value = nullptr) noexcept;
      static ExprResult location(const ir::Value &Address, const ir::Value *Declaration = nullptr) noexcept;

      explicit operator bool() const noexcept;
      const ir::Value *value() const noexcept;
      // Locations have the pointed-to object's type; ordinary values and descriptors retain their IR type.
      const ir::Type *type() const noexcept;
      bool isLocation() const noexcept;
      const ir::Value *declaration() const noexcept;

    private:
      const ir::Value *Value = nullptr;
      const ir::Value *Declaration = nullptr;
      bool Location = false;
  };
} // namespace ink::semantic

#endif
