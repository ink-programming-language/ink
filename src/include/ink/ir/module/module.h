#ifndef INK_IR_MODULE_MODULE_H
#define INK_IR_MODULE_MODULE_H

#include "ink/core/name/name.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module_id.h"
#include "ink/ir/module/symbol_import.h"
#include "ink/ir/module/symbol_export.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/type/value.h"
#include "ink/target/target_context.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::ir
{
  // Owns nominal types, global declarations, symbol imports/exports and the identifier for one Ink module.
  // Borrows its IRContext, which owns builtin/structural types and constants and must outlive this module.
  // All users of module-owned declarations and nominal types must finish before module destruction.
  // Returned declaration pointers remain valid until the module is destroyed; access requires external synchronization.
  class Module final : public Value
  {
    public:
      explicit Module(IRContext &Context, ModuleID ID = {});
      ~Module() noexcept override;

      Module(const Module &) = delete;
      Module &operator=(const Module &) = delete;
      Module(Module &&) = delete;
      Module &operator=(Module &&) = delete;

      const target::TargetContext &targetContext() const noexcept;

      auto &irContext(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Context);
      }

      auto &typePool(this auto &&Self) noexcept
      {
        return Self.irContext().typePool();
      }

      auto &constantPool(this auto &&Self) noexcept
      {
        return Self.irContext().constantPool();
      }

      // Recognizes shared pooled types and this module's own nominal types.
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

      // Both queries include shared pooled types, local nominal types, global functions and global variables.
      bool owns(const Value *Declaration) const noexcept;
      bool contains(const Value *Declaration) const noexcept;

      const std::vector<std::unique_ptr<GlobalVariable>> &globalVariables() const noexcept;
      std::size_t globalVariableCount() const noexcept;
      auto *globalVariable(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.GlobalVariables.size() ? Self.GlobalVariables[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      // Rejects types outside ownsType() or without runtime layout. Appending also rejects initializers outside the borrowed context's constant pool.
      GlobalVariable *createGlobalVariable(const Type &VariableType, core::Name VariableName = core::NAME_None, bool Mutable = true);
      // Takes ownership by value; invalid input leaves module contents unchanged.
      GlobalVariable *appendGlobalVariable(std::unique_ptr<GlobalVariable> Variable);
      // Reuses module-local read-only storage for a pooled string, preserving its exact bytes without adding a terminator.
      // Rejects constants outside this context without modifying the module.
      const GlobalVariable *getOrCreateStringStorage(const StringConstant &String);

      const std::vector<std::unique_ptr<Function>> &globalFunctions() const noexcept;
      std::size_t globalFunctionCount() const noexcept;
      auto *globalFunction(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.GlobalFunctions.size() ? Self.GlobalFunctions[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      // Rejects signatures outside the borrowed context. Appending takes ownership by value.
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
      static bool classof(const Value *Value) noexcept;

    private:
      IRContext &Context;
      // Pooled references do not extend nominal lifetimes; their users must finish before module destruction.
      std::vector<std::unique_ptr<ClassType>> Classes;
      std::vector<std::unique_ptr<InterfaceType>> Interfaces;
      std::vector<std::unique_ptr<StructType>> Structs;

      ModuleID ID;
      std::vector<SymbolImport> Imports;
      std::vector<SymbolExport> Exports;
      std::vector<std::unique_ptr<GlobalVariable>> GlobalVariables;
      std::unordered_map<const StringConstant *, const GlobalVariable *> StringStorages;
      std::vector<std::unique_ptr<Function>> GlobalFunctions;
      Function *InitializationFunction = nullptr;
      Function *DeinitializationFunction = nullptr;
  };
} // namespace ink::ir

#endif
