#include "ink/ir/function/function.h"

#include <utility>

namespace ink::ir
{
  Function::Function(const FunctionType &Signature, core::Name Name, SymbolLinkage SymbolLink, LanguageLinkage LanguageLink, FunctionVisibility Visibility)
      : Value(ValueKind::Function),
        Signature(&Signature),
        Name(Name),
        SymbolLink(SymbolLink),
        LanguageLink(LanguageLink),
        Visibility(Visibility)
  {
    Parameters.reserve(Signature.parameterCount());
    for (std::size_t Index = 0; Index < Signature.parameterCount(); ++Index)
    {
      Parameters.push_back(std::unique_ptr<Parameter>(new Parameter(*this, Index)));
    }
  }

  Function::~Function() noexcept = default;

  const FunctionType *Function::functionType() const noexcept
  {
    return Signature;
  }

  core::Name Function::name() const noexcept
  {
    return Name;
  }

  bool Function::hasName() const noexcept
  {
    return Name.valid();
  }

  void Function::setName(core::Name NewName) noexcept
  {
    Name = NewName;
  }

  SymbolLinkage Function::symbolLinkage() const noexcept
  {
    return SymbolLink;
  }

  void Function::setSymbolLinkage(SymbolLinkage NewLinkage) noexcept
  {
    SymbolLink = NewLinkage;
  }

  LanguageLinkage Function::languageLinkage() const noexcept
  {
    return LanguageLink;
  }

  void Function::setLanguageLinkage(LanguageLinkage NewLinkage) noexcept
  {
    LanguageLink = NewLinkage;
  }

  FunctionVisibility Function::visibility() const noexcept
  {
    return Visibility;
  }

  void Function::setVisibility(FunctionVisibility NewVisibility) noexcept
  {
    Visibility = NewVisibility;
  }

  const std::vector<std::unique_ptr<Parameter>> &Function::parameters() const noexcept
  {
    return Parameters;
  }

  std::size_t Function::parameterCount() const noexcept
  {
    return Parameters.size();
  }

  Parameter *Function::parameter(std::size_t Index) noexcept
  {
    return Index < Parameters.size() ? Parameters[Index].get() : nullptr;
  }

  const Parameter *Function::parameter(std::size_t Index) const noexcept
  {
    return Index < Parameters.size() ? Parameters[Index].get() : nullptr;
  }

  const std::vector<std::unique_ptr<BasicBlock>> &Function::blocks() const noexcept
  {
    return Blocks;
  }

  std::size_t Function::blockCount() const noexcept
  {
    return Blocks.size();
  }

  BasicBlock *Function::block(std::size_t Index) noexcept
  {
    return Index < Blocks.size() ? Blocks[Index].get() : nullptr;
  }

  const BasicBlock *Function::block(std::size_t Index) const noexcept
  {
    return Index < Blocks.size() ? Blocks[Index].get() : nullptr;
  }

  BasicBlock *Function::entryBlock() noexcept
  {
    return block(0);
  }

  const BasicBlock *Function::entryBlock() const noexcept
  {
    return block(0);
  }

  BasicBlock *Function::createBlock(core::Name BlockName)
  {
    auto Block = std::make_unique<BasicBlock>(BlockName);
    Block->Parent = this;
    BasicBlock *Result = Block.get();
    Blocks.push_back(std::move(Block));
    return Result;
  }

  bool Function::isDeclaration() const noexcept
  {
    return Blocks.empty();
  }

  bool Function::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Function;
  }
} // namespace ink::ir
