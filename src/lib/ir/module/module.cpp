#include "ink/ir/module/module.h"

#include <utility>

namespace ink::ir
{
  Module::Module(core::Name Name) noexcept
      : Name(Name)
  {
  }

  Module::~Module() noexcept = default;

  core::Name Module::name() const noexcept
  {
    return Name;
  }

  bool Module::hasName() const noexcept
  {
    return Name.valid();
  }

  bool Module::owns(const Value *Declaration) const noexcept
  {
    if (!Declaration)
    {
      return false;
    }
    for (const auto &Variable : GlobalVariables)
    {
      if (Variable.get() == Declaration)
      {
        return true;
      }
    }
    for (const auto &Callable : Functions)
    {
      if (Callable.get() == Declaration)
      {
        return true;
      }
    }
    return false;
  }

  bool Module::contains(const Value *Declaration) const noexcept
  {
    if (owns(Declaration))
    {
      return true;
    }
    for (const auto *ClassValue : Classes)
    {
      if (ClassValue == Declaration)
      {
        return true;
      }
    }
    for (const auto *InterfaceValue : Interfaces)
    {
      if (InterfaceValue == Declaration)
      {
        return true;
      }
    }
    for (const auto *StructValue : Structs)
    {
      if (StructValue == Declaration)
      {
        return true;
      }
    }
    return false;
  }

  const std::vector<std::unique_ptr<GlobalVariable>> &Module::globalVariables() const noexcept
  {
    return GlobalVariables;
  }

  std::size_t Module::globalVariableCount() const noexcept
  {
    return GlobalVariables.size();
  }

  GlobalVariable *Module::globalVariable(std::size_t Index) noexcept
  {
    return Index < GlobalVariables.size() ? GlobalVariables[Index].get() : nullptr;
  }

  const GlobalVariable *Module::globalVariable(std::size_t Index) const noexcept
  {
    return Index < GlobalVariables.size() ? GlobalVariables[Index].get() : nullptr;
  }

  GlobalVariable *Module::createGlobalVariable(const Type &VariableType, core::Name VariableName)
  {
    return appendGlobalVariable(std::make_unique<GlobalVariable>(VariableType, VariableName));
  }

  GlobalVariable *Module::appendGlobalVariable(std::unique_ptr<GlobalVariable> Variable)
  {
    if (!Variable)
    {
      return nullptr;
    }
    GlobalVariable *Result = Variable.get();
    GlobalVariables.push_back(std::move(Variable));
    return Result;
  }

  const std::vector<std::unique_ptr<Function>> &Module::functions() const noexcept
  {
    return Functions;
  }

  std::size_t Module::functionCount() const noexcept
  {
    return Functions.size();
  }

  Function *Module::function(std::size_t Index) noexcept
  {
    return Index < Functions.size() ? Functions[Index].get() : nullptr;
  }

  const Function *Module::function(std::size_t Index) const noexcept
  {
    return Index < Functions.size() ? Functions[Index].get() : nullptr;
  }

  Function *Module::createFunction(const FunctionType &Signature, core::Name FunctionName, SymbolLinkage SymbolLink, LanguageLinkage LanguageLink, FunctionVisibility Visibility)
  {
    return appendFunction(std::make_unique<Function>(Signature, FunctionName, SymbolLink, LanguageLink, Visibility));
  }

  Function *Module::appendFunction(std::unique_ptr<Function> Callable)
  {
    if (!Callable)
    {
      return nullptr;
    }
    Function *Result = Callable.get();
    Functions.push_back(std::move(Callable));
    return Result;
  }

  const std::vector<const ClassType *> &Module::classes() const noexcept
  {
    return Classes;
  }

  std::size_t Module::classCount() const noexcept
  {
    return Classes.size();
  }

  const ClassType *Module::classType(std::size_t Index) const noexcept
  {
    return Index < Classes.size() ? Classes[Index] : nullptr;
  }

  bool Module::addClass(const ClassType &DeclaredType)
  {
    for (const auto *ClassValue : Classes)
    {
      if (ClassValue == &DeclaredType)
      {
        return false;
      }
    }
    Classes.push_back(&DeclaredType);
    return true;
  }

  const std::vector<const InterfaceType *> &Module::interfaces() const noexcept
  {
    return Interfaces;
  }

  std::size_t Module::interfaceCount() const noexcept
  {
    return Interfaces.size();
  }

  const InterfaceType *Module::interfaceType(std::size_t Index) const noexcept
  {
    return Index < Interfaces.size() ? Interfaces[Index] : nullptr;
  }

  bool Module::addInterface(const InterfaceType &DeclaredType)
  {
    for (const auto *InterfaceValue : Interfaces)
    {
      if (InterfaceValue == &DeclaredType)
      {
        return false;
      }
    }
    Interfaces.push_back(&DeclaredType);
    return true;
  }

  const std::vector<const StructType *> &Module::structs() const noexcept
  {
    return Structs;
  }

  std::size_t Module::structCount() const noexcept
  {
    return Structs.size();
  }

  const StructType *Module::structType(std::size_t Index) const noexcept
  {
    return Index < Structs.size() ? Structs[Index] : nullptr;
  }

  bool Module::addStruct(const StructType &DeclaredType)
  {
    for (const auto *StructValue : Structs)
    {
      if (StructValue == &DeclaredType)
      {
        return false;
      }
    }
    Structs.push_back(&DeclaredType);
    return true;
  }
} // namespace ink::ir
