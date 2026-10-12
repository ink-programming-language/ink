#ifndef INK_IR_FUNCTION_FUNCTION_H
#define INK_IR_FUNCTION_FUNCTION_H

#include "ink/core/name/name.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/param/parameter.h"
#include "ink/ir/type/structure_type/function_type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace ink::ir
{
  // Symbol direction relative to the current module, independent of language and visibility.
  enum class SymbolLinkage : std::uint8_t
  {
    Local,
    Import,
    Export,
  };

  // Resolved language linkage; source ABI strings remain frontend metadata.
  enum class LanguageLinkage : std::uint8_t
  {
    Ink,
    C,
  };

  // Resolved Ink access visibility. An omitted source modifier resolves to Public.
  enum class FunctionVisibility : std::uint8_t
  {
    Public,
    Private,
  };

  // Owns formal parameter values and the IR body; the borrowed signature must outlive this function.
  class Function final : public Value
  {
    public:
      explicit Function(const FunctionType &Signature, core::Name Name = core::NAME_None, SymbolLinkage SymbolLink = SymbolLinkage::Local, LanguageLinkage LanguageLink = LanguageLinkage::Ink, FunctionVisibility Visibility = FunctionVisibility::Public);
      ~Function() noexcept override;

      Function(const Function &) = delete;
      Function &operator=(const Function &) = delete;
      Function(Function &&) = delete;
      Function &operator=(Function &&) = delete;

      const FunctionType *functionType() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;

      // Import covers both Ink module imports and foreign language declarations.
      // These properties are explicit and do not change when blocks are created.
      SymbolLinkage symbolLinkage() const noexcept;
      void setSymbolLinkage(SymbolLinkage NewLinkage) noexcept;
      LanguageLinkage languageLinkage() const noexcept;
      void setLanguageLinkage(LanguageLinkage NewLinkage) noexcept;
      FunctionVisibility visibility() const noexcept;
      void setVisibility(FunctionVisibility NewVisibility) noexcept;

      const std::vector<std::unique_ptr<Parameter>> &parameters() const noexcept;
      std::size_t parameterCount() const noexcept;
      // Indexed access returns nullptr outside the owned parameter or block list.
      Parameter *parameter(std::size_t Index) noexcept;
      const Parameter *parameter(std::size_t Index) const noexcept;

      const std::vector<std::unique_ptr<BasicBlock>> &blocks() const noexcept;
      std::size_t blockCount() const noexcept;
      BasicBlock *block(std::size_t Index) noexcept;
      const BasicBlock *block(std::size_t Index) const noexcept;
      BasicBlock *entryBlock() noexcept;
      const BasicBlock *entryBlock() const noexcept;
      // Returned block addresses stay valid until this function is destroyed.
      BasicBlock *createBlock(core::Name BlockName = core::NAME_None);
      // Means no IR body is present, including while a definition is awaiting construction.
      bool isDeclaration() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const FunctionType *Signature = nullptr;
      core::Name Name = core::NAME_None;
      SymbolLinkage SymbolLink = SymbolLinkage::Local;
      LanguageLinkage LanguageLink = LanguageLinkage::Ink;
      FunctionVisibility Visibility = FunctionVisibility::Public;
      std::vector<std::unique_ptr<Parameter>> Parameters;
      std::vector<std::unique_ptr<BasicBlock>> Blocks;
  };
} // namespace ink::ir

#endif
