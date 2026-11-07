#include "ink/ir/verification/verifier.h"

#include "verifier_internal.h"

#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_visitor.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace ink::ir
{
  namespace
  {
    bool isValidVisibility(SymbolVisibility Visibility) noexcept
    {
      return Visibility == SymbolVisibility::Private || Visibility == SymbolVisibility::Public;
    }

    bool isValidLinkage(NativeLinkage Linkage) noexcept
    {
      return Linkage == NativeLinkage::None || Linkage == NativeLinkage::Import || Linkage == NativeLinkage::Export;
    }

    bool isValidMemberKind(MemberKind Kind) noexcept
    {
      return Kind == MemberKind::Instance || Kind == MemberKind::Static;
    }

    bool isValidParameterKind(Parameter::Kind Kind) noexcept
    {
      return Kind == Parameter::Kind::Positional || Kind == Parameter::Kind::Named;
    }

    bool isValidABI(ABIKind ABI) noexcept
    {
      return ABI == ABIKind::Ink || ABI == ABIKind::C;
    }

    bool isValidCallingConvention(CallingConvention Convention) noexcept
    {
      return Convention == CallingConvention::Default || Convention == CallingConvention::CDecl || Convention == CallingConvention::StdCall || Convention == CallingConvention::FastCall || Convention == CallingConvention::VectorCall;
    }

    bool isKnownTypeKind(TypeKind Kind) noexcept
    {
      switch (Kind)
      {
#define TYPE(Kind, Class, Spelling) case TypeKind::Kind: return true;
#include "ink/ir/type/type.def"
#undef TYPE
      }
      return false;
    }

    void verifyFunctionAttributes(const Function &FunctionValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context)
    {
      if (!isValidVisibility(FunctionValue.visibility()) || !isValidLinkage(FunctionValue.nativeLinkage()))
      {
        detail::failVerification(Diagnostics, Context, "Function visibility or native linkage is invalid");
      }
    }

    class TypeVerifier final : public TypeVisitor<TypeVerifier>
    {
      public:
        TypeVerifier(const core::DiagnosticEngine &Diagnostics, std::string_view Context, bool AllowInstructionDependencies = true)
            : Diagnostics(Diagnostics),
              Context(Context),
              AllowInstructionDependencies(AllowInstructionDependencies)
        {
        }

        void verify(const Value &TypeValue)
        {
          verifyType(TypeValue);
          for (const Type *KnownType : KnownTypes)
          {
            if (RecordType::classof(KnownType))
            {
              const auto &Record = static_cast<const RecordType &>(*KnownType);
              verifyInheritance(Record);
              verifyRecordLayout(Record);
            }
            else if (ArrayType::classof(KnownType))
            {
              verifyStorageLayout(*KnownType);
            }
          }
        }

        void visitMetaType(const MetaType &TypeValue)
        {
          require(&TypeValue == &MetaType::instance(), "MetaType must be the canonical self-typed instance");
        }

        void visitIntegerType(const IntegerType &TypeValue)
        {
          const std::uint32_t Width = TypeValue.bitWidth();
          require(Width == 8 || Width == 16 || Width == 32 || Width == 64, "Integer type has an unsupported bit width");
          require(TypeValue.signedness() == IntegerType::Signedness::Signed || TypeValue.signedness() == IntegerType::Signedness::Unsigned, "Integer type signedness is invalid");
        }

        void visitFloatType(const FloatType &TypeValue)
        {
          require(TypeValue.bitWidth() == 32 || TypeValue.bitWidth() == 64, "Floating-point type has an unsupported bit width");
        }

        void visitBoolType(const BoolType &)
        {
        }

        void visitVoidType(const VoidType &)
        {
        }

        void visitArrayType(const ArrayType &TypeValue)
        {
          verifyType(TypeValue.elementType());
          require(!VoidType::classof(&TypeValue.elementType()), "Array element type must not be void");
        }

        void visitPointerType(const PointerType &TypeValue)
        {
          verifyType(TypeValue.pointeeType());
        }

        void visitStructType(const StructType &TypeValue)
        {
          verifyRecord(TypeValue);
        }

        void visitObjectType(const ObjectType &TypeValue)
        {
          verifyRecord(TypeValue);
        }

        void visitFunctionType(const FunctionType &TypeValue)
        {
          require(isValidABI(TypeValue.abi()) && isValidCallingConvention(TypeValue.callingConvention()), "Function ABI or calling convention is invalid");
          verifyType(TypeValue.returnType());
          // The parameter list contains fixed parameters only; ellipsis is a separate function-type flag.
          for (const Parameter &ParameterValue : TypeValue.parameters())
          {
            require(isValidParameterKind(ParameterValue.kind()), "Function parameter kind is invalid");
            verifyType(ParameterValue.type());
            require(!VoidType::classof(&ParameterValue.type()), "Function parameter type must not be void");
          }
        }

      private:
        void require(bool Condition, std::string_view Description) const
        {
          if (!Condition)
          {
            detail::failVerification(Diagnostics, Context, Description);
          }
        }

        void verifyType(const Value &TypeValue)
        {
          require(&TypeValue.getType() == &MetaType::instance(), "Type position requires a value with the canonical MetaType");
          if (TypeValue.valueKind() != ValueKind::Type)
          {
            require(TypeValue.valueKind() == ValueKind::Argument || TypeValue.valueKind() == ValueKind::Instruction, "Type position contains an unsupported type-valued object");
            require(AllowInstructionDependencies || TypeValue.valueKind() != ValueKind::Instruction, "Declaration type metadata must not depend on a function-body instruction");
            return;
          }
          const Type *Concrete = static_cast<const Type *>(&TypeValue);
          require(isKnownTypeKind(Concrete->getKind()), "Type kind is invalid");
          const TypeSemantics Expected = Concrete->getKind() == TypeKind::Object ? TypeSemantics::Reference : TypeSemantics::Value;
          require(Concrete->getSemantics() == Expected, "Type semantics do not match its concrete kind");
          if (!Visited.insert(Concrete).second)
          {
            return;
          }
          KnownTypes.push_back(Concrete);
          visit(*Concrete);
        }

        void verifyRecord(const RecordType &Record)
        {
          for (const Value *Base : Record.bases())
          {
            require(Base != nullptr, "Record has a null base type");
            verifyType(*Base);
            require(Base->valueKind() != ValueKind::Type || RecordType::classof(Base), "Concrete base type must be a record type");
          }
          for (const Field &FieldValue : Record.fields())
          {
            require(isValidMemberKind(FieldValue.kind()) && isValidVisibility(FieldValue.visibility()), "Field kind or visibility is invalid");
            verifyType(FieldValue.type());
            require(!VoidType::classof(&FieldValue.type()), "Field type must not be void");
          }
          for (const Method &MethodValue : Record.methods())
          {
            require(isValidMemberKind(MethodValue.kind()), "Method kind is invalid");
            require(MethodValue.hasFunction(), "Method has no owned function");
            const Function &FunctionValue = MethodValue.function();
            require(FunctionValue.parent() == nullptr, "Record-owned method function must not have a module parent");
            verifyFunctionAttributes(FunctionValue, Diagnostics, Context);
            verifyType(FunctionValue.getType());
            require(FunctionType::classof(&FunctionValue.getType()), "Method requires a concrete function signature");
          }
        }

        void verifyInheritance(const RecordType &Record)
        {
          if (CompleteInheritance.contains(&Record))
          {
            return;
          }
          require(ActiveInheritance.insert(&Record).second, "Record inheritance contains a cycle");
          for (const Value *Base : Record.bases())
          {
            if (RecordType::classof(Base))
            {
              verifyInheritance(static_cast<const RecordType &>(*Base));
            }
          }
          ActiveInheritance.erase(&Record);
          CompleteInheritance.insert(&Record);
        }

        void verifyStorageLayout(const Value &TypeValue)
        {
          if (TypeValue.valueKind() != ValueKind::Type)
          {
            return;
          }
          const Type *Concrete = static_cast<const Type *>(&TypeValue);
          if (Concrete->getKind() == TypeKind::Object)
          {
            return;
          }
          if (Concrete->getKind() == TypeKind::Struct)
          {
            verifyRecordLayout(static_cast<const StructType &>(*Concrete));
          }
          else if (Concrete->getKind() == TypeKind::Array)
          {
            if (CompleteLayout.contains(Concrete))
            {
              return;
            }
            require(ActiveLayout.insert(Concrete).second, "Type contains a recursive by-value layout");
            verifyStorageLayout(static_cast<const ArrayType &>(*Concrete).elementType());
            ActiveLayout.erase(Concrete);
            CompleteLayout.insert(Concrete);
          }
        }

        void verifyRecordLayout(const RecordType &Record)
        {
          if (CompleteLayout.contains(&Record))
          {
            return;
          }
          require(ActiveLayout.insert(&Record).second, "Record contains a recursive by-value layout");
          for (const Value *Base : Record.bases())
          {
            if (RecordType::classof(Base))
            {
              verifyRecordLayout(static_cast<const RecordType &>(*Base));
            }
          }
          for (const Field &FieldValue : Record.fields())
          {
            if (!FieldValue.isStatic())
            {
              verifyStorageLayout(FieldValue.type());
            }
          }
          ActiveLayout.erase(&Record);
          CompleteLayout.insert(&Record);
        }

        const core::DiagnosticEngine &Diagnostics;
        std::string_view Context;
        bool AllowInstructionDependencies;
        std::unordered_set<const Type *> Visited;
        std::vector<const Type *> KnownTypes;
        std::unordered_set<const RecordType *> ActiveInheritance;
        std::unordered_set<const RecordType *> CompleteInheritance;
        std::unordered_set<const Type *> ActiveLayout;
        std::unordered_set<const Type *> CompleteLayout;
    };

    std::string declarationContext(std::string_view Kind, std::size_t Index)
    {
      return "module / " + std::string(Kind) + " #" + std::to_string(Index);
    }

    void verifyDeclarationTypeValue(const Value &TypeValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context)
    {
      TypeVerifier(Diagnostics, Context, false).verify(TypeValue);
    }
  } // namespace

  namespace detail
  {
    void verifyTypeValue(const Value &TypeValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context)
    {
      TypeVerifier(Diagnostics, Context).verify(TypeValue);
    }

    void verifyConstant(const Value &ConstantValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context)
    {
      verifyTypeValue(ConstantValue.getType(), Diagnostics, Context);
      switch (ConstantValue.valueKind())
      {
        case ValueKind::IntegerConstant:
          if (!IntegerType::classof(&ConstantValue.getType()))
          {
            failVerification(Diagnostics, Context, "Integer constant requires an integer type");
          }
          return;
        case ValueKind::FloatConstant:
          if (!FloatType::classof(&ConstantValue.getType()))
          {
            failVerification(Diagnostics, Context, "Floating-point constant requires a floating-point type");
          }
          return;
        case ValueKind::StringConstant:
        {
          if (!ArrayType::classof(&ConstantValue.getType()))
          {
            failVerification(Diagnostics, Context, "String constant requires an array type");
          }
          const auto &Array = static_cast<const ArrayType &>(ConstantValue.getType());
          const auto &String = static_cast<const StringConstant &>(ConstantValue);
          if (!IntegerType::classof(&Array.elementType()))
          {
            failVerification(Diagnostics, Context, "String constant requires unsigned 8-bit array elements");
          }
          const auto &Element = static_cast<const IntegerType &>(Array.elementType());
          if (Element.bitWidth() != 8 || Element.isSigned())
          {
            failVerification(Diagnostics, Context, "String constant requires unsigned 8-bit array elements");
          }
          if (Array.elementCount() != String.value().size())
          {
            failVerification(Diagnostics, Context, "String constant array length must equal its byte count");
          }
          return;
        }
        default:
          failVerification(Diagnostics, Context, "Value does not have a supported constant kind");
      }
    }
  } // namespace detail

  void verifyModule(const Module &ModuleValue, const core::DiagnosticEngine &Diagnostics)
  {
    std::unordered_set<const RecordType *> Registered;
    std::vector<const RecordType *> Records;
    for (const StructType &Record : ModuleValue.structTypes())
    {
      if (!Registered.insert(&Record).second)
      {
        detail::failVerification(Diagnostics, "module", "Record type is registered more than once");
      }
      Records.push_back(&Record);
    }
    for (const ObjectType &Record : ModuleValue.objectTypes())
    {
      if (!Registered.insert(&Record).second)
      {
        detail::failVerification(Diagnostics, "module", "Record type is registered more than once");
      }
      Records.push_back(&Record);
    }
    for (std::size_t Index = 0; Index < Records.size(); ++Index)
    {
      verifyDeclarationTypeValue(*Records[Index], Diagnostics, declarationContext("record", Index));
    }
    std::size_t Index = 0;
    for (const GlobalVariable &Global : ModuleValue.globalVariables())
    {
      const std::string Context = declarationContext("global", Index++);
      if (Global.parent() != &ModuleValue || !isValidVisibility(Global.visibility()) || !isValidLinkage(Global.nativeLinkage()))
      {
        detail::failVerification(Diagnostics, Context, "Global variable has an invalid parent, visibility, or native linkage");
      }
      verifyDeclarationTypeValue(Global.getType(), Diagnostics, Context);
      if (!PointerType::classof(&Global.getType()))
      {
        detail::failVerification(Diagnostics, Context, "Global variable requires a pointer result type");
      }
      if (VoidType::classof(&Global.valueType()))
      {
        detail::failVerification(Diagnostics, Context, "Global variable stored type must not be void");
      }
    }
    Index = 0;
    for (const Function &FunctionValue : ModuleValue.functions())
    {
      const std::string Context = declarationContext("function", Index++);
      if (FunctionValue.parent() != &ModuleValue)
      {
        detail::failVerification(Diagnostics, Context, "Module-owned function has an incorrect parent");
      }
      verifyFunctionAttributes(FunctionValue, Diagnostics, Context);
      detail::verifyFunctionBody(FunctionValue, Diagnostics, Context);
    }
    for (std::size_t RecordIndex = 0; RecordIndex < Records.size(); ++RecordIndex)
    {
      const RecordType &Record = *Records[RecordIndex];
      std::size_t MethodIndex = 0;
      for (const Method &MethodValue : Record.methods())
      {
        const std::string Context = declarationContext("record", RecordIndex) + " / method #" + std::to_string(MethodIndex++);
        if (!MethodValue.hasFunction())
        {
          detail::failVerification(Diagnostics, Context, "Method has no owned function");
        }
        detail::verifyFunctionBody(MethodValue.function(), Diagnostics, Context);
      }
    }
  }
} // namespace ink::ir
