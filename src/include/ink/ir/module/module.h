#ifndef INK_IR_MODULE_MODULE_H
#define INK_IR_MODULE_MODULE_H

#include "ink/core/name/name.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/package/package_path.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace ink::ir
{
  // Owns its package path, global functions and global variables, and registers borrowed nominal types for one Ink module.
  // TypeContext and borrowed constants must outlive the module.
  // Registered types own their methods and member functions independently of this module.
  // Declaration addresses stay valid until the module is destroyed; operations require external synchronization.
  class Module final
  {
    public:
      explicit Module(core::Name Name = core::NAME_None, PackagePath Package = {}) noexcept;
      ~Module() noexcept;

      Module(const Module &) = delete;
      Module &operator=(const Module &) = delete;
      Module(Module &&) = delete;
      Module &operator=(Module &&) = delete;

      // Optional canonical short module name, excluding the package path.
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      // The module owns an independent absolute package path; the default empty path denotes the root package.
      const PackagePath &package() const noexcept;
      void setPackage(PackagePath NewPackage) noexcept;
      // Ownership is limited to global functions and global variables; contains also includes registered types.
      bool owns(const Value *Declaration) const noexcept;
      bool contains(const Value *Declaration) const noexcept;

      const std::vector<std::unique_ptr<GlobalVariable>> &globalVariables() const noexcept;
      std::size_t globalVariableCount() const noexcept;
      auto *globalVariable(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.GlobalVariables.size() ? Self.GlobalVariables[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      GlobalVariable *createGlobalVariable(const Type &VariableType, core::Name VariableName = core::NAME_None);
      // Takes ownership; nullptr is rejected without changing the module.
      GlobalVariable *appendGlobalVariable(std::unique_ptr<GlobalVariable> Variable);

      const std::vector<std::unique_ptr<Function>> &globalFunctions() const noexcept;
      std::size_t globalFunctionCount() const noexcept;
      auto *globalFunction(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.GlobalFunctions.size() ? Self.GlobalFunctions[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

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

      // Types remain owned by TypeContext. Registration rejects repeated pointers and preserves insertion order.
      // Indexed access returns nullptr out of bounds.
      const std::vector<const ClassType *> &classes() const noexcept;
      std::size_t classCount() const noexcept;
      const ClassType *classType(std::size_t Index) const noexcept;
      bool addClass(const ClassType &DeclaredType);

      const std::vector<const InterfaceType *> &interfaces() const noexcept;
      std::size_t interfaceCount() const noexcept;
      const InterfaceType *interfaceType(std::size_t Index) const noexcept;
      bool addInterface(const InterfaceType &DeclaredType);

      const std::vector<const StructType *> &structs() const noexcept;
      std::size_t structCount() const noexcept;
      const StructType *structType(std::size_t Index) const noexcept;
      bool addStruct(const StructType &DeclaredType);

    private:
      core::Name Name = core::NAME_None;
      PackagePath Package;
      std::vector<std::unique_ptr<GlobalVariable>> GlobalVariables;
      std::vector<std::unique_ptr<Function>> GlobalFunctions;
      Function *InitializationFunction = nullptr;
      Function *DeinitializationFunction = nullptr;
      std::vector<const ClassType *> Classes;
      std::vector<const InterfaceType *> Interfaces;
      std::vector<const StructType *> Structs;
  };
} // namespace ink::ir

#endif
