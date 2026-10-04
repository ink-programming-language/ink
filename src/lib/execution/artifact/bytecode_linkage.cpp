#include "bytecode_internal.h"

#include "ink/abi/name_mangling.h"

namespace ink::execution::artifact_detail
{
  namespace
  {
    bool matches(const abi::Record &Type, RuntimeTypeId Id, const RuntimeTypeTable &Types, std::size_t Depth = 0)
    {
      const auto *Layout = Types.get(Id);
      if (!Layout || Depth >= abi::ManglingLimits{}.MaxDepth)
      {
        return false;
      }
      switch (Type.Tag)
      {
      case 'v':
        return Layout->Kind == RuntimeKind::Void;
      case 'b':
        return Layout->Kind == RuntimeKind::Boolean;
      case 'i':
      case 'u':
        return Layout->Kind == RuntimeKind::Integer && Layout->isSigned() == (Type.Tag == 'i') && Type.Payload == std::to_string(Layout->bitWidth());
      case 'f':
        return Layout->Kind == RuntimeKind::Float && Type.Payload == std::to_string(Layout->bitWidth());
      case 'c':
        return Layout->Kind == RuntimeKind::Class && Layout->classDesc().NominalIdentity == abi::mangle(abi::record('T', {Type})).Name;
      case 's':
        return Layout->Kind == RuntimeKind::String && Type.Payload == "r" + abi::encodeRecord({'u', "8"});
      case 'p':
      case 'r':
      {
        const auto C = abi::childRecords(Type, 1);
        return Layout->Kind == RuntimeKind::Pointer && C && C->size() == 1 && !Type.Payload.empty() && Layout->pointerDesc().Writable == (Type.Payload.front() == 'w') && matches(C->front(), Layout->pointerDesc().Pointee, Types, Depth + 1);
      }
      case 'a':
      {
        const auto C = abi::childRecords(Type);
        return Layout->Kind == RuntimeKind::Array && C && C->size() == 2 && C->front().Payload == std::to_string(Layout->arrayDesc().ElementCount) && matches(C->back(), Layout->arrayDesc().ElementType, Types, Depth + 1);
      }
      case 'q':
      {
        const auto C = abi::childRecords(Type);
        const auto P = C && C->size() == 2 ? abi::childRecords(C->front()) : std::nullopt;
        const auto Q = C && C->size() == 2 ? abi::childRecords(C->back()) : std::nullopt;
        if (Layout->Kind != RuntimeKind::Function || !P || !Q || Q->size() != 1 || P->size() != Layout->functionDesc().Parameters.size() || !matches(Q->front(), Layout->functionDesc().ReturnType, Types, Depth + 1))
        {
          return false;
        }
        for (std::size_t Index = 0; Index < P->size(); ++Index)
        {
          if (!matches((*P)[Index], Layout->functionDesc().Parameters[Index], Types, Depth + 1))
          {
            return false;
          }
        }
        return true;
      }
      default:
        return false;
      }
    }
  } // namespace

  bool validSymbolLinkage(const BytecodeSymbol &Symbol, const RuntimeFunctionDescriptor &Descriptor, const RuntimeTypeTable &Types, const std::unordered_map<std::string, RuntimeTypeId> &IdentityTypes)
  {
    if (Symbol.Kind == BytecodeSymbolKind::Native)
    {
      return Symbol.Identity.LinkName == Descriptor.Symbol && !Symbol.Identity.LinkName.starts_with("_INK") && Symbol.Identity.GenericArguments.empty();
    }
    const auto Parsed = abi::demangle(Symbol.Identity.LinkName);
    const auto Root = Parsed ? abi::childRecords(*Parsed.Identity) : std::nullopt;
    if (!Root || Root->size() != 3 || (Parsed.Identity->Tag != 'F' && Parsed.Identity->Tag != 'I'))
    {
      return false;
    }
    const auto S = abi::childRecords(Root->back());
    if (!S || S->size() != 5 || (*S)[0].Payload != (Descriptor.CAbi ? "c" : "i") || (*S)[1].Payload != "c")
    {
      return false;
    }
    auto Parameters = abi::childRecords((*S)[3]);
    if (!Parameters)
    {
      return false;
    }
    const auto &Receiver = (*S)[2];
    if (Receiver.Payload != "n")
    {
      const auto C = abi::childRecords(Receiver, 1);
      if (!C || C->size() != 1 || Receiver.Payload.front() == 'v')
      {
        return false;
      }
      Parameters->insert(Parameters->begin(), {'p', Receiver.Payload});
    }
    if (!matches(abi::record('q', {abi::record('L', *Parameters), (*S)[4]}), Descriptor.Signature, Types))
    {
      return false;
    }
    if (Parsed.Identity->Tag == 'I')
    {
      return Symbol.Identity.GenericArguments.empty();
    }
    const auto Arguments = abi::childRecords((*Root)[1]);
    if (!Arguments || Arguments->size() != Symbol.Identity.GenericArguments.size())
    {
      return false;
    }
    for (std::size_t Index = 0; Index < Arguments->size(); ++Index)
    {
      const auto &Argument = Symbol.Identity.GenericArguments[Index];
      const auto &Encoded = (*Arguments)[Index];
      const auto C = abi::childRecords(Encoded);
      if (!C || C->empty() || Encoded.Tag != (Argument.Kind == core::GenericArgumentKind::Type ? 'T' : 'V') || (Encoded.Tag == 'V' && (C->size() != 2 || (*C)[1].Tag != 'B' || (*C)[1].Payload != Argument.Value)))
      {
        return false;
      }
      const auto Found = IdentityTypes.find(Argument.Type);
      if (Found == IdentityTypes.end() || !matches(C->front(), Found->second, Types))
      {
        return false;
      }
    }
    return true;
  }
} // namespace ink::execution::artifact_detail
