#include "ink/ir/linkage.h"

#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/reference_type.h"
#include "ink/ir/type/slice_type.h"

#include <algorithm>

namespace ink::ir
{
  using abi::Record;
  using abi::record;

  bool IRBuilder::setModuleIdentity(Module &Owner, abi::ModuleIdentity Identity)
  {
    std::sort(Identity.Package.Variant.begin(), Identity.Package.Variant.end());
    if (&Owner.context() != &Context || !Owner.entryBlock().values().empty() || !Owner.classTypes().empty() || !abi::mangle(record('J', {abi::packageRecord(Identity.Package), abi::moduleRecord(Identity)})))
    {
      return false;
    }
    Owner.LinkageIdentity = std::move(Identity);
    Context.notifyChanged();
    return true;
  }

  bool IRBuilder::setFunctionLexicalScope(Function &Owner, std::vector<std::uint64_t> Scope)
  {
    if (&Owner.context() != &Context || Scope.size() >= abi::ManglingLimits{}.MaxDepth)
    {
      return false;
    }
    Owner.LexicalScope = std::move(Scope);
    Context.notifyChanged();
    return true;
  }

  Record declarationRecord(const abi::ModuleIdentity &Module, std::span<const Record> Owners, char Kind, std::string_view Name, const Record &Pattern)
  {
    return record('R', {abi::packageRecord(Module.Package), abi::moduleRecord(Module), record('O', Owners), {'K', std::string(1, Kind)}, abi::nameRecord(Name), {'G', {}}, Pattern});
  }

  std::optional<Record> typeRecord(const Type &Value, std::size_t Depth)
  {
    if (Depth >= abi::ManglingLimits{}.MaxDepth)
    {
      return std::nullopt;
    }
    switch (Value.typeKind())
    {
    case TypeKind::Void:
      return Record{'v', {}};
    case TypeKind::Bool:
      return Record{'b', {}};
    case TypeKind::Integer:
    {
      const auto &Integer = static_cast<const IntegerType &>(Value);
      return Record{Integer.isSigned() ? 'i' : 'u', std::to_string(Integer.bitWidth())};
    }
    case TypeKind::Float:
      return Record{'f', std::to_string(static_cast<const FloatType &>(Value).bitWidth())};
    case TypeKind::Pointer:
    case TypeKind::Reference:
    case TypeKind::Slice:
    {
      const bool Pointer = Value.typeKind() == TypeKind::Pointer;
      const bool Reference = Value.typeKind() == TypeKind::Reference;
      const Type &Element = Pointer ? static_cast<const PointerType &>(Value).pointeeType() : Reference ? static_cast<const ReferenceType &>(Value).referentType() : static_cast<const SliceType &>(Value).elementType();
      const AccessKind Access = Pointer ? static_cast<const PointerType &>(Value).access() : Reference ? static_cast<const ReferenceType &>(Value).access() : static_cast<const SliceType &>(Value).access();
      const auto Child = typeRecord(Element, Depth + 1);
      return Child ? std::optional<Record>({Pointer ? 'p' : Reference ? 'r' : 's', std::string(Access == AccessKind::ReadWrite ? "w" : "r") + abi::encodeRecord(*Child)}) : std::nullopt;
    }
    case TypeKind::Array:
    {
      const auto &Array = static_cast<const ArrayType &>(Value);
      const auto Child = typeRecord(Array.elementType(), Depth + 1);
      return Child ? std::optional<Record>(record('a', {{'D', std::to_string(Array.elementCount())}, *Child})) : std::nullopt;
    }
    case TypeKind::Function:
    {
      const auto &Function = static_cast<const FunctionType &>(Value);
      std::vector<Record> Parameters;
      for (const auto *Parameter : Function.parameterTypes())
      {
        const auto Child = typeRecord(*Parameter, Depth + 1);
        if (!Child)
        {
          return std::nullopt;
        }
        Parameters.push_back(*Child);
      }
      const auto Result = typeRecord(Function.returnType(), Depth + 1);
      return Result ? std::optional<Record>(record('q', {record('L', Parameters), record('Q', {*Result})})) : std::nullopt;
    }
    case TypeKind::Class:
    {
      const auto Parsed = abi::demangle(static_cast<const ClassType &>(Value).identity());
      const auto C = Parsed ? abi::childRecords(*Parsed.Identity) : std::nullopt;
      return C && Parsed.Identity->Tag == 'T' && C->size() == 1 && C->front().Tag == 'c' ? std::optional<Record>(C->front()) : std::nullopt;
    }
    default:
      return std::nullopt;
    }
  }

  std::optional<Record> functionRecord(const Function &Value, const abi::ModuleIdentity *DetachedModule, std::string_view DetachedName, std::size_t Depth)
  {
    if (Depth >= abi::ManglingLimits{}.MaxDepth)
    {
      return std::nullopt;
    }
    for (const auto &Parameter : Value.parameters())
    {
      if (Parameter->parameterKind() == ParameterKind::Variadic)
      {
        return std::nullopt;
      }
    }
    const Module *ModuleValue = nullptr;
    const Function *Parent = nullptr;
    for (const ir::Value *Outer = Value.outer(); Outer; Outer = Outer->outer())
    {
      if (!Parent && Function::classof(Outer))
      {
        Parent = static_cast<const Function *>(Outer);
      }
      if (Module::classof(Outer))
      {
        ModuleValue = static_cast<const Module *>(Outer);
        break;
      }
    }
    const auto *Identity = ModuleValue ? &ModuleValue->linkageIdentity() : DetachedModule;
    if (!Identity)
    {
      return std::nullopt;
    }
    std::vector<Record> Owners;
    if (Parent)
    {
      const auto ParentRecord = functionRecord(*Parent, Identity, {}, Depth + 1);
      const auto C = ParentRecord ? abi::childRecords(*ParentRecord) : std::nullopt;
      if (!C || C->size() != 3 || ParentRecord->Tag != 'F')
      {
        return std::nullopt;
      }
      Owners.push_back(record('F', {(*C)[0], (*C)[1]}));
    }
    for (const auto Index : Value.lexicalScope())
    {
      Owners.push_back(record('B', {{'D', std::to_string(Index)}}));
    }
    Record Receiver{'A', "n"};
    std::size_t FirstParameter = 0;
    const auto *Class = Value.classOwner();
    const bool Initializer = Class && Value.initializerField() != std::numeric_limits<std::size_t>::max();
    std::optional<Record> ClassRecord;
    if (Class)
    {
      ClassRecord = typeRecord(*Class, Depth + 1);
      if (!ClassRecord)
      {
        return std::nullopt;
      }
      Owners.push_back(*ClassRecord);
      if (!Initializer)
      {
        const auto P = Value.functionType().parameterTypes();
        if (P.empty() || P.front()->typeKind() != TypeKind::Pointer || &static_cast<const PointerType *>(P.front())->pointeeType() != Class)
        {
          return std::nullopt;
        }
        Receiver.Payload = std::string(static_cast<const PointerType *>(P.front())->access() == AccessKind::ReadWrite ? "w" : "r") + abi::encodeRecord(*ClassRecord);
        FirstParameter = 1;
      }
    }
    std::vector<Record> Parameters;
    const auto P = Value.functionType().parameterTypes();
    for (std::size_t Index = FirstParameter; Index < P.size(); ++Index)
    {
      const auto Parameter = typeRecord(*P[Index], Depth + 1);
      if (!Parameter)
      {
        return std::nullopt;
      }
      Parameters.push_back(*Parameter);
    }
    const auto Return = typeRecord(Value.functionType().returnType(), Depth + 1);
    if (!Return)
    {
      return std::nullopt;
    }
    const Record ParameterList = record('L', Parameters);
    const auto Convention = Value.callingConvention();
    const Record Signature = record('S', {{'B', Value.languageLinkage() == LanguageLinkage::C ? "c" : "i"}, {'C', Convention == CallingConvention::C ? "c" : Convention == CallingConvention::Fast ? "f" : "k"}, Receiver, ParameterList, record('Q', {*Return})});
    if (Initializer)
    {
      if (Value.initializerField() >= Class->fields().size())
      {
        return std::nullopt;
      }
      return record('I', {*ClassRecord, abi::nameRecord(Value.context().namePool().text(Class->fields()[Value.initializerField()].FieldName)), Signature});
    }
    const auto Name = !ModuleValue && !DetachedName.empty() ? DetachedName : Value.context().namePool().text(Value.name());
    return record('F', {declarationRecord(*Identity, Owners, 'f', Name, record('H', {Receiver, ParameterList})), {'X', {}}, Signature});
  }

  abi::MangleResult functionSymbol(const Function &Value, const abi::ModuleIdentity *DetachedModule, std::string_view DetachedName)
  {
    const auto Identity = functionRecord(Value, DetachedModule, DetachedName);
    return Identity ? abi::mangle(*Identity) : abi::MangleResult{{}, "function has no supported stable linkage identity"};
  }

  abi::MangleResult reflectionSymbol(const Module &Value)
  {
    return abi::mangle(record('J', {abi::packageRecord(Value.linkageIdentity().Package), abi::moduleRecord(Value.linkageIdentity())}));
  }

  abi::MangleResult reflectionThunkSymbol(const Function &Value)
  {
    const auto Function = functionRecord(Value);
    if (!Function)
    {
      return {{}, "reflection target has no linkage identity"};
    }
    // A thunk takes two writable opaque pointers and returns void.
    const Record Pointer{'p', "w" + abi::encodeRecord({'v', {}})};
    const Record Signature = record('S', {{'B', "c"}, {'C', "c"}, {'A', "n"}, record('L', {Pointer, Pointer}), record('Q', {{'v', {}}})});
    // Field initializer targets use the same generated-target schema as functions.
    return abi::mangle(record('H', {abi::nameRecord("reflection"), *Function, Signature}));
  }
} // namespace ink::ir
