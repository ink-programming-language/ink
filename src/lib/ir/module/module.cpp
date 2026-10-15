#include "ink/ir/module/module.h"
#include "../type/type_layout.h"

#include <utility>

namespace ink::ir
{
  namespace
  {
    bool validLifecycleFunction(const Module &Owner, const Function *Callable) noexcept
    {
      return !Callable || (Owner.owns(Callable) && Callable->functionType()->parameterCount() == 0 && VoidType::classof(Callable->functionType()->returnType()));
    }
  } // namespace

  Module::Module(IRContext &Context, ModuleID ID)
      : Value(ValueKind::Module),
        Context(Context),
        ID(std::move(ID))
  {
  }

  Module::~Module() noexcept
  {
    // Release every body while all field metadata, constants and types remain alive.
    GlobalFunctions.clear();
    for (const auto &TypeValue : Classes)
    {
      TypeValue->Methods.clear();
    }
    for (const auto &TypeValue : Interfaces)
    {
      TypeValue->Methods.clear();
    }
    for (const auto &TypeValue : Structs)
    {
      TypeValue->Methods.clear();
    }
    GlobalVariables.clear();
    for (const auto &TypeValue : Classes)
    {
      TypeValue->Fields.clear();
      Context.unregisterNominalType(TypeValue.get());
    }
    for (const auto &TypeValue : Interfaces)
    {
      TypeValue->Fields.clear();
      Context.unregisterNominalType(TypeValue.get());
    }
    for (const auto &TypeValue : Structs)
    {
      TypeValue->Fields.clear();
      Context.unregisterNominalType(TypeValue.get());
    }
  }

  const target::TargetContext &Module::targetContext() const noexcept
  {
    return Context.targetContext();
  }

  bool Module::ownsType(const Type *TypeValue) const noexcept
  {
    if (typePool().owns(TypeValue))
    {
      return true;
    }
    for (const auto &DeclaredType : Classes)
    {
      if (DeclaredType.get() == TypeValue)
      {
        return true;
      }
    }
    for (const auto &DeclaredType : Interfaces)
    {
      if (DeclaredType.get() == TypeValue)
      {
        return true;
      }
    }
    for (const auto &DeclaredType : Structs)
    {
      if (DeclaredType.get() == TypeValue)
      {
        return true;
      }
    }
    return false;
  }

  const ModuleID &Module::moduleID() const noexcept
  {
    return ID;
  }

  void Module::setModuleID(ModuleID NewID) noexcept
  {
    ID = std::move(NewID);
  }

  bool Module::owns(const Value *Declaration) const noexcept
  {
    if (!Declaration)
    {
      return false;
    }
    if (Type::classof(Declaration) && ownsType(static_cast<const Type *>(Declaration)))
    {
      return true;
    }
    for (const auto &Variable : GlobalVariables)
    {
      if (Variable.get() == Declaration)
      {
        return true;
      }
    }
    for (const auto &Callable : GlobalFunctions)
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
    return owns(Declaration);
  }

  const std::vector<std::unique_ptr<GlobalVariable>> &Module::globalVariables() const noexcept
  {
    return GlobalVariables;
  }

  std::size_t Module::globalVariableCount() const noexcept
  {
    return GlobalVariables.size();
  }

  GlobalVariable *Module::createGlobalVariable(const Type &VariableType, core::Name VariableName)
  {
    if (!ownsType(&VariableType))
    {
      return nullptr;
    }
    return appendGlobalVariable(std::make_unique<GlobalVariable>(VariableType, VariableName));
  }

  GlobalVariable *Module::appendGlobalVariable(std::unique_ptr<GlobalVariable> Variable)
  {
    if (!Variable || !ownsType(Variable->type()) || (Variable->hasInitializer() && !constantPool().owns(Variable->initializer())))
    {
      return nullptr;
    }
    GlobalVariable *Result = Variable.get();
    GlobalVariables.push_back(std::move(Variable));
    return Result;
  }

  const std::vector<std::unique_ptr<Function>> &Module::globalFunctions() const noexcept
  {
    return GlobalFunctions;
  }

  std::size_t Module::globalFunctionCount() const noexcept
  {
    return GlobalFunctions.size();
  }

  Function *Module::createGlobalFunction(const FunctionType &Signature, core::Name FunctionName, SymbolLinkage SymbolLink, LanguageLinkage LanguageLink, FunctionVisibility Visibility)
  {
    if (!ownsType(&Signature))
    {
      return nullptr;
    }
    return appendGlobalFunction(std::make_unique<Function>(Signature, FunctionName, SymbolLink, LanguageLink, Visibility));
  }

  Function *Module::appendGlobalFunction(std::unique_ptr<Function> Callable)
  {
    if (!Callable || !ownsType(Callable->functionType()))
    {
      return nullptr;
    }
    Function *Result = Callable.get();
    GlobalFunctions.push_back(std::move(Callable));
    return Result;
  }

  bool Module::setInitializationFunction(Function *Callable) noexcept
  {
    if (!validLifecycleFunction(*this, Callable))
    {
      return false;
    }
    InitializationFunction = Callable;
    return true;
  }

  bool Module::setDeinitializationFunction(Function *Callable) noexcept
  {
    if (!validLifecycleFunction(*this, Callable))
    {
      return false;
    }
    DeinitializationFunction = Callable;
    return true;
  }

  const std::vector<std::unique_ptr<ClassType>> &Module::classes() const noexcept
  {
    return Classes;
  }

  std::size_t Module::classCount() const noexcept
  {
    return Classes.size();
  }

  ClassType *Module::createClassType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name)
  {
    if (!detail::validAlignment(Alignment))
    {
      return nullptr;
    }
    auto DeclaredType = std::unique_ptr<ClassType>(new ClassType(*this, Size, Alignment, Name));
    ClassType *Result = DeclaredType.get();
    Classes.push_back(std::move(DeclaredType));
    Context.registerNominalType(Result);
    return Result;
  }

  const std::vector<std::unique_ptr<InterfaceType>> &Module::interfaces() const noexcept
  {
    return Interfaces;
  }

  std::size_t Module::interfaceCount() const noexcept
  {
    return Interfaces.size();
  }

  InterfaceType *Module::createInterfaceType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name)
  {
    if (!detail::validAlignment(Alignment))
    {
      return nullptr;
    }
    auto DeclaredType = std::unique_ptr<InterfaceType>(new InterfaceType(*this, Size, Alignment, Name));
    InterfaceType *Result = DeclaredType.get();
    Interfaces.push_back(std::move(DeclaredType));
    Context.registerNominalType(Result);
    return Result;
  }

  const std::vector<std::unique_ptr<StructType>> &Module::structs() const noexcept
  {
    return Structs;
  }

  std::size_t Module::structCount() const noexcept
  {
    return Structs.size();
  }

  StructType *Module::createStructType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name)
  {
    if (!detail::validAlignment(Alignment))
    {
      return nullptr;
    }
    auto DeclaredType = std::unique_ptr<StructType>(new StructType(*this, Size, Alignment, Name));
    StructType *Result = DeclaredType.get();
    Structs.push_back(std::move(DeclaredType));
    Context.registerNominalType(Result);
    return Result;
  }

  bool Module::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Module;
  }
} // namespace ink::ir
