#ifndef INK_IR_MODULE_MODULE_H
#define INK_IR_MODULE_MODULE_H

#include "ink/core/name/name.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module_id.h"
#include "ink/ir/module/symbol_import.h"
#include "ink/ir/module/symbol_export.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"
#include "ink/ir/type/type_pool.h"
#include "ink/target/target_context.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace ink::ir
{
  // Owns its types, constants, global declarations, symbol imports/exports and full identifier for one Ink module.
  // Nominal types are owned directly; TypePool owns builtin and structural types.
  // Returned pointers remain valid until the module is destroyed; operations require external synchronization.
  class Module final
  {
    public:
      explicit Module(target::TargetContext Target, ModuleID ID = {});
      ~Module() noexcept;

      Module(const Module &) = delete;
      Module &operator=(const Module &) = delete;
      Module(Module &&) = delete;
      Module &operator=(Module &&) = delete;

      const target::TargetContext &targetContext() const noexcept;

      auto &typePool(this auto &&Self) noexcept
      {
        return Self.Types;
      }

      auto &constantPool(this auto &&Self) noexcept
      {
        return Self.Constants;
      }

      // Includes this module's builtin, structural and nominal types.
      bool ownsType(const Type *TypeValue) const noexcept;

      // Owns an independent absolute identifier, including the module name; the default empty ID denotes an unnamed module.
      const ModuleID &moduleID() const noexcept;
      void setModuleID(ModuleID NewID) noexcept;

      // Symbol records are owned by value; name resolution and validation are handled by the producer.
      auto &imports(this auto &&Self) noexcept
      {
        return Self.Imports;
      }

      auto &exports(this auto &&Self) noexcept
      {
        return Self.Exports;
      }

      // Both queries include local types, global functions and global variables.
      bool owns(const Value *Declaration) const noexcept;
      bool contains(const Value *Declaration) const noexcept;

      const std::vector<std::unique_ptr<GlobalVariable>> &globalVariables() const noexcept;
      std::size_t globalVariableCount() const noexcept;
      auto *globalVariable(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.GlobalVariables.size() ? Self.GlobalVariables[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      // Rejects foreign types. Appending also rejects initializers outside this module's constant pool.
      GlobalVariable *createGlobalVariable(const Type &VariableType, core::Name VariableName = core::NAME_None);
      // Takes ownership by value; invalid input leaves module contents unchanged.
      GlobalVariable *appendGlobalVariable(std::unique_ptr<GlobalVariable> Variable);

      const std::vector<std::unique_ptr<Function>> &globalFunctions() const noexcept;
      std::size_t globalFunctionCount() const noexcept;
      auto *globalFunction(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.GlobalFunctions.size() ? Self.GlobalFunctions[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      // Rejects signatures outside this module. Appending takes ownership by value.
      Function *createGlobalFunction(const FunctionType &Signature, core::Name FunctionName = core::NAME_None, SymbolLinkage SymbolLink = SymbolLinkage::Local, LanguageLinkage LanguageLink = LanguageLinkage::Ink, FunctionVisibility Visibility = FunctionVisibility::Public);
      Function *appendGlobalFunction(std::unique_ptr<Function> Callable);

      // Optional lifecycle entry points borrowed from GlobalFunctions; recording them does not execute them.
      auto *initializationFunction(this auto &&Self) noexcept
      {
        return Self.InitializationFunction ? &std::forward_like<decltype(Self) &>(*Self.InitializationFunction) : nullptr;
      }

      auto *deinitializationFunction(this auto &&Self) noexcept
      {
        return Self.DeinitializationFunction ? &std::forward_like<decltype(Self) &>(*Self.DeinitializationFunction) : nullptr;
      }

      // nullptr clears the entry point. Non-null functions must be owned here and have signature () -> void.
      // A body may be built later. Rejection preserves the previous entry point and does not transfer ownership.
      bool setInitializationFunction(Function *Callable) noexcept;
      bool setDeinitializationFunction(Function *Callable) noexcept;

      // Each creation owns and registers a distinct nominal identity, even for equal layouts.
      // Size is preserved in bytes; zero or non-power-of-two alignment returns nullptr without modifying the module.
      // Type names, fields and owned methods remain editable until freeze(); function names/bodies are not frozen and layouts never change.
      // Indexed access returns nullptr out of bounds.
      const std::vector<std::unique_ptr<ClassType>> &classes() const noexcept;
      std::size_t classCount() const noexcept;
      auto *classType(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.Classes.size() ? Self.Classes[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      ClassType *createClassType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name = core::NAME_None);

      const std::vector<std::unique_ptr<InterfaceType>> &interfaces() const noexcept;
      std::size_t interfaceCount() const noexcept;
      auto *interfaceType(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.Interfaces.size() ? Self.Interfaces[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      InterfaceType *createInterfaceType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name = core::NAME_None);

      const std::vector<std::unique_ptr<StructType>> &structs() const noexcept;
      std::size_t structCount() const noexcept;
      auto *structType(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.Structs.size() ? Self.Structs[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      StructType *createStructType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name = core::NAME_None);

    private:
      const target::TargetContext Target;
      // Nominal identities outlive structural types that refer to them; metadata is cleared by the destructor first.
      std::vector<std::unique_ptr<ClassType>> Classes;
      std::vector<std::unique_ptr<InterfaceType>> Interfaces;
      std::vector<std::unique_ptr<StructType>> Structs;
      TypePool Types;
      // Declared after all types so constants are destroyed before their borrowed types.
      ConstantPool Constants;

      ModuleID ID;
      std::vector<SymbolImport> Imports;
      std::vector<SymbolExport> Exports;
      std::vector<std::unique_ptr<GlobalVariable>> GlobalVariables;
      std::vector<std::unique_ptr<Function>> GlobalFunctions;
      Function *InitializationFunction = nullptr;
      Function *DeinitializationFunction = nullptr;
  };
} // namespace ink::ir

#endif
