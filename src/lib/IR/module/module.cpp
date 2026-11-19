#include "ink/ir/module/module.h"

#include "ink/ir/verification/verifier.h"

#include <utility>

namespace ink::ir
{
    Module::Module(ModulePath Path) noexcept
        : Path(std::move(Path))
    {
    }

    ModuleImport& Module::addImport(ModulePath PathValue)
    {
        if (ModuleImport* Existing = findImport(PathValue))
        {
            return *Existing;
        }
        ModuleImport& Created = Imports.emplace_back(std::move(PathValue));
        Created.Parent = this;
        return Created;
    }

    void Module::addStructType(StructType& TypeValue)
    {
        StructTypes.push_back(&TypeValue);
    }

    void Module::addObjectType(ObjectType& TypeValue)
    {
        ObjectTypes.push_back(&TypeValue);
    }

    void Module::addImportedStructType(const ModuleImport& Source, StructType& TypeValue)
    {
        StructTypes.push_back(&TypeValue);
        RecordImports.insert_or_assign(&TypeValue, &Source);
    }

    void Module::addImportedObjectType(const ModuleImport& Source, ObjectType& TypeValue)
    {
        ObjectTypes.push_back(&TypeValue);
        RecordImports.insert_or_assign(&TypeValue, &Source);
    }

    const ModuleImport* Module::findRecordImportSource(const RecordType& TypeValue) const noexcept
    {
        const auto Found = RecordImports.find(&TypeValue);
        return Found != RecordImports.end() ? Found->second : nullptr;
    }

    bool Module::isImportedRecord(const RecordType& TypeValue) const noexcept
    {
        return findRecordImportSource(TypeValue) != nullptr;
    }

    Function& Module::createFunction(ValueRef Signature, core::Name FunctionName, SymbolVisibility Visibility, NativeLinkage Linkage)
    {
        Function& Created = Functions.emplace_back(Signature, FunctionName, Visibility, Linkage);
        Created.Parent = this;
        return Created;
    }

    Function& Module::declareImportedFunction(const ModuleImport& Source, ValueRef Signature, core::Name SymbolName)
    {
        Function& Created = Functions.emplace_back(Signature, SymbolName, SymbolVisibility::Public, NativeLinkage::None);
        Created.Parent = this;
        Created.ImportSource = &Source;
        return Created;
    }

    GlobalVariable& Module::createGlobalVariable(const PointerType& ResultType, core::Name GlobalName, SymbolVisibility Visibility, NativeLinkage Linkage)
    {
        GlobalVariable& Created = GlobalVariables.emplace_back(ResultType, GlobalName, Visibility, Linkage);
        Created.Parent = this;
        return Created;
    }

    GlobalVariable& Module::declareImportedGlobal(const ModuleImport& Source, const PointerType& ResultType, core::Name SymbolName)
    {
        GlobalVariable& Created = GlobalVariables.emplace_back(ResultType, SymbolName, SymbolVisibility::Public, NativeLinkage::None);
        Created.Parent = this;
        Created.ImportSource = &Source;
        return Created;
    }

    void Module::verify(const core::DiagnosticEngine& Diagnostics) const
    {
        verifyModule(*this, Diagnostics);
    }

    bool Module::verify(std::string* Error) const
    {
        return verifyModule(*this, Error);
    }
} // namespace ink::ir
