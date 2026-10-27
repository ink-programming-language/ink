#ifndef INK_IR_SERIALIZATION_SCHEMA_H
#define INK_IR_SERIALIZATION_SCHEMA_H

#include "ink/ir/function/function.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/target/target_context.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

namespace ink::ir::detail
{
  inline constexpr std::uint64_t InvalidEnumValue = std::numeric_limits<std::uint64_t>::max();
  inline constexpr std::uint64_t NoVirtualSlot = std::numeric_limits<std::uint64_t>::max();

  enum class RecordCategory : std::uint8_t
  {
    Other,
    Type,
    Nominal,
    Constant,
    Instruction,
  };

  // Wire IDs, categories, spellings and layouts are declared together in Record.def.
  enum class RecordKind : std::uint32_t
  {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) Kind = WireId,
#include "Record.def"
#undef INK_IR_RECORD
  };

  struct RecordShape
  {
      std::size_t NumberCount = 0;
      std::size_t ReferenceCount = 0;
      std::size_t StringCount = 0;
      bool VariableNumbers = false;
      bool VariableReferences = false;
      bool VariableStrings = false;

      constexpr bool matches(std::size_t Numbers, std::size_t References, std::size_t Strings) const noexcept
      {
        return (VariableNumbers ? Numbers >= NumberCount : Numbers == NumberCount) && (VariableReferences ? References >= ReferenceCount : References == ReferenceCount) && (VariableStrings ? Strings >= StringCount : Strings == StringCount);
      }
  };

  // Field indices are relative to the corresponding Numbers, References or Strings group.
  // Counts describe the fixed prefix, or the minimum length when that group is variable.
  namespace schema
  {
    struct Named
    {
        static constexpr std::size_t NumberCount = 0;
        static constexpr std::size_t ReferenceCount = 0;
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
    };

    struct Typed
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
    };

    struct Type
    {
        enum NumberField : std::size_t
        {
          Mutable,
          NumberCount,
        };
        static constexpr std::size_t ReferenceCount = 0;
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Instruction
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t OperandsBegin = ReferenceCount;
    };

    struct Module
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Init,
          Deinit,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t ComponentsBegin = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, true};
    };

    struct IntegerType
    {
        enum NumberField : std::size_t
        {
          Mutable,
          Width,
          Signed,
          NumberCount,
        };
        static constexpr std::size_t ReferenceCount = 0;
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct FloatType
    {
        enum NumberField : std::size_t
        {
          Mutable,
          Width,
          NumberCount,
        };
        static constexpr std::size_t ReferenceCount = 0;
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Empty
    {
        static constexpr std::size_t NumberCount = 0;
        static constexpr std::size_t ReferenceCount = 0;
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct PointerType
    {
        enum NumberField : std::size_t
        {
          Mutable,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Element,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct ArrayType
    {
        enum NumberField : std::size_t
        {
          Mutable,
          Count,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Element,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct FunctionType
    {
        enum NumberField : std::size_t
        {
          Mutable,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Return,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t VariadicsBegin = NumberCount;
        static constexpr std::size_t ParametersBegin = ReferenceCount;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, true, true, false};
    };

    struct NominalType
    {
        enum NumberField : std::size_t
        {
          Size,
          Alignment,
          Frozen,
          NumberCount,
        };
        static constexpr std::size_t ReferenceCount = 0;
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct QualifiedType
    {
        enum NumberField : std::size_t
        {
          Mutable,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Declaration,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Field
    {
        enum NumberField : std::size_t
        {
          Offset,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Owner,
          Type,
          Initializer,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Function
    {
        enum NumberField : std::size_t
        {
          Linkage,
          Language,
          Visibility,
          Dispatch,
          Override,
          Final,
          Slot,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Signature,
          Owner,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Parameter
    {
        enum NumberField : std::size_t
        {
          Index,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Function,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Block
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Function,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Global
    {
        enum NumberField : std::size_t
        {
          StringStorage,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Type,
          Initializer,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Import
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 1;
        static constexpr std::size_t PathBegin = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, true};
    };

    struct Export
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Declaration,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct BoolConstant
    {
        enum NumberField : std::size_t
        {
          Value,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct IntegerConstant
    {
        static constexpr std::size_t NumberCount = 1;
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t WordsBegin = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, true, false, false};
    };

    struct FloatConstant
    {
        enum NumberField : std::size_t
        {
          Bits,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct NullPointerConstant
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct StringConstant
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Bytes,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct ArrayConstant
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t ElementsBegin = ReferenceCount;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, true, false};
    };

    struct Alloca
    {
        enum NumberField : std::size_t
        {
          Initialized,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          ReferenceCount,
        };
        enum StringField : std::size_t
        {
          Name,
          StringCount,
        };
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Load
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Pointer,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Store
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Value,
          Pointer,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Neg
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Operand,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct ICmp
    {
        enum NumberField : std::size_t
        {
          Predicate,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Left,
          Right,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Branch
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Target,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct CondBranch
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Condition,
          TrueTarget,
          FalseTarget,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Return
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Value,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, false, false};
    };

    struct Call
    {
        static constexpr std::size_t NumberCount = 0;
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          Signature,
          Callee,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t ArgumentsBegin = ReferenceCount;
        static constexpr std::size_t ArgumentNamesBegin = StringCount;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, true, true};
    };

    struct GetElementPtr
    {
        enum NumberField : std::size_t
        {
          InBounds,
          NumberCount,
        };
        enum ReferenceField : std::size_t
        {
          Type,
          Block,
          SourceType,
          Pointer,
          ReferenceCount,
        };
        static constexpr std::size_t StringCount = 0;
        static constexpr std::size_t IndicesBegin = ReferenceCount;
        static constexpr RecordShape Shape{NumberCount, ReferenceCount, StringCount, false, true, false};
    };

    using BoolType = Type;
    using VoidType = Type;
    using NullPointerType = Type;
    using MetaType = Empty;
    using VariadicType = Empty;
    using ReferenceType = PointerType;
    using ClassType = NominalType;
    using InterfaceType = NominalType;
    using StructType = NominalType;

    template <typename... Layouts>
    consteval bool validMutablePrefixes()
    {
      return ((static_cast<std::size_t>(Layouts::Mutable) == Type::Mutable) && ...);
    }

    template <typename... Layouts>
    consteval bool validTypedPrefixes()
    {
      return ((static_cast<std::size_t>(Layouts::Type) == Typed::Type) && ...);
    }

    template <typename... Layouts>
    consteval bool validNamedPrefixes()
    {
      return ((static_cast<std::size_t>(Layouts::Name) == Named::Name) && ...);
    }

    template <typename... Layouts>
    consteval bool validInstructionPrefixes()
    {
      return ((static_cast<std::size_t>(Layouts::Type) == Instruction::Type && static_cast<std::size_t>(Layouts::Block) == Instruction::Block && static_cast<std::size_t>(Layouts::ReferenceCount) >= Instruction::OperandsBegin) && ...);
    }

    // Generic readers use these common prefixes; a layout edit must not silently change their meaning.
    static_assert(validMutablePrefixes<BoolType, IntegerType, FloatType, VoidType, NullPointerType, PointerType, ReferenceType, ArrayType, FunctionType, QualifiedType>());
    static_assert(validTypedPrefixes<Instruction, Global, Import, BoolConstant, IntegerConstant, FloatConstant, NullPointerConstant, StringConstant, ArrayConstant>());
    static_assert(validNamedPrefixes<NominalType, Field, Function, Parameter, Block, Global, Export, Alloca>());
    static_assert(validInstructionPrefixes<Alloca, Load, Store, Neg, ICmp, Branch, CondBranch, Return, Call, GetElementPtr>());
    static_assert(static_cast<std::size_t>(Function::Signature) == Typed::Type);
  } // namespace schema

  constexpr bool isValidRecordKind(RecordKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) case RecordKind::Kind:
#include "Record.def"
#undef INK_IR_RECORD
      return true;
    default:
      return false;
    }
  }

  constexpr RecordCategory recordCategory(RecordKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) case RecordKind::Kind: return RecordCategory::Category;
#include "Record.def"
#undef INK_IR_RECORD
    default:
      return RecordCategory::Other;
    }
  }

  constexpr bool isType(RecordKind Kind) noexcept
  {
    return recordCategory(Kind) == RecordCategory::Type || recordCategory(Kind) == RecordCategory::Nominal;
  }

  constexpr bool isNominal(RecordKind Kind) noexcept
  {
    return recordCategory(Kind) == RecordCategory::Nominal;
  }

  constexpr bool isConstant(RecordKind Kind) noexcept
  {
    return recordCategory(Kind) == RecordCategory::Constant;
  }

  constexpr bool isInstruction(RecordKind Kind) noexcept
  {
    return recordCategory(Kind) == RecordCategory::Instruction;
  }

  constexpr std::string_view recordKindName(RecordKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) case RecordKind::Kind: return Name;
#include "Record.def"
#undef INK_IR_RECORD
    default:
      return {};
    }
  }

  constexpr std::string_view recordTextName(RecordKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) case RecordKind::Kind: return TextName;
#include "Record.def"
#undef INK_IR_RECORD
    default:
      return {};
    }
  }

  constexpr RecordShape recordShape(RecordKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) case RecordKind::Kind: return schema::Layout::Shape;
#include "Record.def"
#undef INK_IR_RECORD
    default:
      return {};
    }
  }

  constexpr bool validateRecordShape(RecordKind Kind, std::size_t Numbers, std::size_t References, std::size_t Strings) noexcept
  {
    if (!isValidRecordKind(Kind) || !recordShape(Kind).matches(Numbers, References, Strings))
    {
      return false;
    }
    if (Kind == RecordKind::FunctionType)
    {
      return Numbers - schema::FunctionType::VariadicsBegin == References - schema::FunctionType::ParametersBegin;
    }
    if (Kind == RecordKind::Call)
    {
      return Strings - schema::Call::ArgumentNamesBegin == References - schema::Call::ArgumentsBegin;
    }
    return true;
  }

  constexpr bool isNullableReference(RecordKind Kind, std::size_t Index) noexcept
  {
    switch (Kind)
    {
    case RecordKind::Module:
      return Index == schema::Module::Init || Index == schema::Module::Deinit;
    case RecordKind::Function:
      return Index == schema::Function::Owner;
    case RecordKind::Field:
      return Index == schema::Field::Initializer;
    case RecordKind::Global:
      return Index == schema::Global::Initializer;
    case RecordKind::Return:
      return Index == schema::Return::Value;
    default:
      return false;
    }
  }

  template <typename Enum>
  struct EnumEntry
  {
      Enum Value;
      std::uint64_t WireValue;
      std::string_view Name;
  };

  template <typename Enum>
  struct EnumSchema;

  template <>
  struct EnumSchema<ICmpPredicate>
  {
      inline static constexpr EnumEntry<ICmpPredicate> Entries[] = {
          {ICmpPredicate::Equal, 0, "eq"},
          {ICmpPredicate::NotEqual, 1, "ne"},
          {ICmpPredicate::UnsignedGreaterThan, 2, "ugt"},
          {ICmpPredicate::UnsignedGreaterEqual, 3, "uge"},
          {ICmpPredicate::UnsignedLessThan, 4, "ult"},
          {ICmpPredicate::UnsignedLessEqual, 5, "ule"},
          {ICmpPredicate::SignedGreaterThan, 6, "sgt"},
          {ICmpPredicate::SignedGreaterEqual, 7, "sge"},
          {ICmpPredicate::SignedLessThan, 8, "slt"},
          {ICmpPredicate::SignedLessEqual, 9, "sle"},
      };
  };

  template <>
  struct EnumSchema<SymbolLinkage>
  {
      inline static constexpr EnumEntry<SymbolLinkage> Entries[] = {
          {SymbolLinkage::Local, 0, "local"},
          {SymbolLinkage::Import, 1, "import"},
          {SymbolLinkage::Export, 2, "export"},
      };
  };

  template <>
  struct EnumSchema<LanguageLinkage>
  {
      inline static constexpr EnumEntry<LanguageLinkage> Entries[] = {
          {LanguageLinkage::Ink, 0, "ink"},
          {LanguageLinkage::C, 1, "C"},
      };
  };

  template <>
  struct EnumSchema<FunctionVisibility>
  {
      inline static constexpr EnumEntry<FunctionVisibility> Entries[] = {
          {FunctionVisibility::Public, 0, "public"},
          {FunctionVisibility::Private, 1, "private"},
      };
  };

  template <>
  struct EnumSchema<MethodDispatch>
  {
      inline static constexpr EnumEntry<MethodDispatch> Entries[] = {
          {MethodDispatch::Direct, 0, "direct"},
          {MethodDispatch::Virtual, 1, "virtual"},
          {MethodDispatch::Abstract, 2, "abstract"},
      };
  };

  template <>
  struct EnumSchema<target::ByteOrder>
  {
      inline static constexpr EnumEntry<target::ByteOrder> Entries[] = {
          {target::ByteOrder::LittleEndian, 0, "little"},
          {target::ByteOrder::BigEndian, 1, "big"},
      };
  };

  template <typename Enum>
  constexpr std::uint64_t encodeEnum(Enum Value) noexcept
  {
    for (const auto &Entry : EnumSchema<Enum>::Entries)
    {
      if (Entry.Value == Value)
      {
        return Entry.WireValue;
      }
    }
    return InvalidEnumValue;
  }

  template <typename Enum>
  constexpr std::optional<Enum> decodeEnum(std::uint64_t WireValue) noexcept
  {
    for (const auto &Entry : EnumSchema<Enum>::Entries)
    {
      if (Entry.WireValue == WireValue)
      {
        return Entry.Value;
      }
    }
    return std::nullopt;
  }

  template <typename Enum>
  constexpr std::string_view enumName(std::uint64_t WireValue) noexcept
  {
    for (const auto &Entry : EnumSchema<Enum>::Entries)
    {
      if (Entry.WireValue == WireValue)
      {
        return Entry.Name;
      }
    }
    return {};
  }

  template <typename Enum>
  constexpr std::optional<std::uint64_t> parseEnum(std::string_view Name) noexcept
  {
    for (const auto &Entry : EnumSchema<Enum>::Entries)
    {
      if (Entry.Name == Name)
      {
        return Entry.WireValue;
      }
    }
    return std::nullopt;
  }

  template <typename Enum>
  consteval bool validEnumSchema()
  {
    for (const auto &Left : EnumSchema<Enum>::Entries)
    {
      if (Left.Name.empty() || Left.WireValue == InvalidEnumValue)
      {
        return false;
      }
      for (const auto &Right : EnumSchema<Enum>::Entries)
      {
        if (&Left != &Right && (Left.Value == Right.Value || Left.WireValue == Right.WireValue || Left.Name == Right.Name))
        {
          return false;
        }
      }
    }
    return true;
  }

  consteval bool validRecordSchema()
  {
    constexpr RecordKind Kinds[] = {
#define INK_IR_RECORD(Kind, WireId, Category, Name, TextName, Layout) RecordKind::Kind,
#include "Record.def"
#undef INK_IR_RECORD
    };
    for (const auto &Left : Kinds)
    {
      if (static_cast<std::uint32_t>(Left) == 0 || recordKindName(Left).empty())
      {
        return false;
      }
      for (const auto &Right : Kinds)
      {
        if (&Left != &Right && (Left == Right || recordKindName(Left) == recordKindName(Right)))
        {
          return false;
        }
      }
    }
    return true;
  }

  static_assert(validRecordSchema());
  static_assert(validEnumSchema<ICmpPredicate>());
  static_assert(validEnumSchema<SymbolLinkage>());
  static_assert(validEnumSchema<LanguageLinkage>());
  static_assert(validEnumSchema<FunctionVisibility>());
  static_assert(validEnumSchema<MethodDispatch>());
  static_assert(validEnumSchema<target::ByteOrder>());
} // namespace ink::ir::detail

#endif
