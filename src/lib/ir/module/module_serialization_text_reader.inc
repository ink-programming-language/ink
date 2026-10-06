namespace ink::ir::archive
{
  namespace
  {
    class TextReader
    {
      public:
        TextReader(std::string_view Input, State &Data)
            : Data(Data),
              Lex(Input)
        {
        }

        bool run()
        {
          Lex.expect("ink-ir");
          const auto Version = Lex.number();
          if (Lex.good() && Version != ModuleTextVersion)
          {
            return Data.fail("Unsupported module text version", core::ArchiveStatus::UnsupportedVersion);
          }
          const auto Root = add(Tag::Module);
          module(Root, 0, 1);
          if (!Lex.token().empty())
          {
            Lex.fail("Trailing module text");
          }
          for (const auto &[Name, Alias] : Aliases)
          {
            if (!Alias.Defined)
            {
              Lex.fail("Undefined type alias " + Name);
            }
          }
          for (const auto &Fix : Fixups)
          {
            const auto Found = Symbols.find(Fix.Value.Name);
            if (Found == Symbols.end())
            {
              Lex.fail("Undefined symbol " + Fix.Value.Name);
              break;
            }
            record(Fix.Record).Fields[Fix.Field] = Found->second;
            if (Fix.Value.Type)
            {
              Data.OperandTypes.emplace_back(Found->second, Fix.Value.Type);
            }
          }
          if (!Lex.good())
          {
            return Data.fail(Lex.message());
          }
          return Data.good();
        }

      private:
        struct Alias
        {
            std::size_t Id;
            bool Defined = false;
        };
        struct Operand
        {
            std::size_t Id = 0;
            std::size_t Type = 0;
            std::string Name;
        };
        struct Fixup
        {
            std::size_t Record;
            std::size_t Field;
            Operand Value;
        };
        bool good() const
        {
          return Data.good() && Lex.good();
        }
        Record &record(std::size_t Id)
        {
          return Data.Records[Id - 1];
        }
        bool depth(std::size_t Depth)
        {
          return Depth <= Data.Limits.MaxNestingDepth || Data.fail("Module text nesting limit exceeded", core::ArchiveStatus::LimitExceeded);
        }
        std::size_t add(Tag Kind, std::size_t Type = 0, std::size_t Parent = 0, std::vector<std::uint64_t> Fields = {}, std::string Text = {})
        {
          if (!Data.fields(Fields.size()) || !Data.string(Text.size()) || !Data.addRecord())
          {
            return 0;
          }
          Data.Records.back() = {Kind, Type, Parent, std::move(Fields), std::move(Text)};
          return Data.Records.size();
        }
        std::string string()
        {
          if (!good() || !Data.charge(Lex.token().size(), 2))
          {
            return {};
          }
          auto Value = Lex.string();
          if (Value.size() > Data.Limits.MaxStringBytes)
          {
            Data.fail("Module text string limit exceeded", core::ArchiveStatus::LimitExceeded);
            return {};
          }
          return Value;
        }
        std::string symbol(std::string_view Prefix)
        {
          Lex.expect(Prefix);
          std::string Name;
          if (Lex.quoted())
          {
            Name = string();
          }
          else if (!Lex.token().empty() && Lex.token() != "{" && Lex.token() != "}" && Lex.token() != "," && Lex.token() != ")")
          {
            Name = Lex.token();
            Lex.next();
          }
          else
          {
            Lex.fail("Expected symbol name");
          }
          if (!Data.string(Name.size()))
          {
            return {};
          }
          return std::string(Prefix) + Name;
        }
        void define(const std::string &Name, std::size_t Id)
        {
          if (good() && !Symbols.emplace(Name, Id).second)
          {
            Lex.fail("Duplicate symbol " + Name);
          }
        }
        std::string name(const std::string &Symbol)
        {
          return Lex.take("name") ? string() : Symbol.empty() ? std::string{}
                                                              : Symbol.substr(1);
        }
        std::string typeKey(Tag Kind, const std::vector<std::uint64_t> &Fields)
        {
          std::string Key = std::to_string(static_cast<unsigned>(Kind));
          for (auto Field : Fields)
          {
            Key += "/" + std::to_string(Field);
          }
          return Key;
        }
        std::size_t poolType(Tag Kind, std::vector<std::uint64_t> Fields = {})
        {
          if (!good())
          {
            return 0;
          }
          const auto Key = typeKey(Kind, Fields);
          if (const auto Found = Types.find(Key); Found != Types.end())
          {
            return Found->second;
          }
          const auto Meta = Kind == Tag::Meta ? 0 : poolType(Tag::Meta);
          const auto Id = add(Kind, Meta, 0, std::move(Fields));
          if (Id && Kind == Tag::Meta)
          {
            record(Id).Type = Id;
          }
          Types.emplace(Key, Id);
          return Id;
        }
        std::size_t type(std::size_t Depth = 1)
        {
          if (!good() || !depth(Depth))
          {
            return 0;
          }
          if (Lex.token() == "!")
          {
            auto Name = symbol("!");
            if (const auto Found = Aliases.find(Name); Found != Aliases.end())
            {
              return Found->second.Id;
            }
            const auto Meta = poolType(Tag::Meta);
            const auto Id = add(Tag::Meta, Meta);
            Aliases.emplace(std::move(Name), Alias{Id});
            return Id;
          }
          if (Lex.take("type"))
          {
            return poolType(Tag::Meta);
          }
          if (Lex.take("void"))
          {
            return poolType(Tag::Void);
          }
          if (Lex.take("bool"))
          {
            return poolType(Tag::Bool);
          }
          if (Lex.take("label"))
          {
            return poolType(Tag::Label);
          }
          if (Lex.take("module"))
          {
            return poolType(Tag::ModuleType);
          }
          if (Lex.take("["))
          {
            const auto Count = Lex.number();
            Lex.expect("x");
            const auto Element = type(Depth + 1);
            Lex.expect("]");
            return poolType(Tag::Array, {Element, Count});
          }
          const auto Word = Lex.token();
          Lex.next();
          if (Word == "ptr" || Word == "ref" || Word == "slice")
          {
            Lex.expect("<");
            std::uint64_t Access = 0;
            if (Lex.take("rw"))
            {
              Access = 1;
            }
            else
            {
              Lex.expect("ro");
            }
            Lex.expect(",");
            const auto Element = type(Depth + 1);
            Lex.expect(">");
            return poolType(Word == "ptr" ? Tag::Pointer : Word == "ref" ? Tag::Reference
                                                                         : Tag::Slice,
                            {Element, Access});
          }
          if (Word == "fn")
          {
            std::vector<std::uint64_t> Fields{0};
            Lex.expect("(");
            while (good() && !Lex.take(")"))
            {
              if (Fields.size() > 1)
              {
                Lex.expect(",");
              }
              if (Fields.size() >= Data.Limits.MaxFields)
              {
                Data.fail("Function type field limit exceeded", core::ArchiveStatus::LimitExceeded);
                return 0;
              }
              if (!Data.fields(1))
              {
                return 0;
              }
              Fields.push_back(type(Depth + 1));
            }
            Lex.expect("-");
            Lex.expect(">");
            Fields[0] = type(Depth + 1);
            return poolType(Tag::FunctionType, std::move(Fields));
          }
          if (Word.size() > 1 && (Word[0] == 'i' || Word[0] == 'u' || Word[0] == 'f'))
          {
            std::uint32_t Width = 0;
            const auto Result = std::from_chars(Word.data() + 1, Word.data() + Word.size(), Width);
            if (Result.ec == std::errc{} && Result.ptr == Word.data() + Word.size() && Width)
            {
              return Word[0] == 'f' ? poolType(Tag::Float, {Width}) : poolType(Tag::Integer, {Width, Word[0] == 'i'});
            }
          }
          Lex.fail("Unknown IR type");
          return 0;
        }
        void typeDefinition()
        {
          Lex.expect("type");
          const auto AliasName = symbol("!");
          auto Found = Aliases.find(AliasName);
          if (Found == Aliases.end())
          {
            const auto Meta = poolType(Tag::Meta);
            Found = Aliases.emplace(AliasName, Alias{add(Tag::Meta, Meta)}).first;
          }
          const auto Id = Found->second.Id;
          Lex.expect("=");
          if (!good())
          {
            return;
          }
          if (Found->second.Defined)
          {
            Lex.fail("Expected a new type alias");
            return;
          }
          Found->second.Defined = true;
          const auto Word = Lex.token();
          if (Word == "class" || Word == "enum" || Word == "interface")
          {
            Lex.next();
            const auto Text = string();
            record(Id).Kind = Word == "class" ? Tag::Class : Word == "enum" ? Tag::Enum
                                                                            : Tag::Interface;
            record(Id).Text = Text;
            if (Word == "class" && Lex.take("identity"))
            {
              const auto Identity = string();
              if (!Data.fields(2) || !Data.string(Text.size() + Identity.size()))
              {
                return;
              }
              record(Id).Fields = {Text.size(), Identity.size()};
              record(Id).Text += Identity;
              Lex.expect("{");
              bool First = true;
              while (good() && !Lex.take("}"))
              {
                if (!First)
                {
                  Lex.expect(",");
                }
                First = false;
                const bool Private = Lex.take("private");
                if (!Private)
                {
                  Lex.expect("public");
                }
                const auto FieldName = string();
                Lex.expect(":");
                const auto FieldType = type();
                if (Data.Limits.MaxFields < 3 || record(Id).Fields.size() > Data.Limits.MaxFields - 3 || !Data.fields(3) || !Data.string(FieldName.size()))
                {
                  Data.fail("Class field limit exceeded", core::ArchiveStatus::LimitExceeded);
                  return;
                }
                if (FieldName.size() > Data.Limits.MaxStringBytes - record(Id).Text.size())
                {
                  Data.fail("Class name storage limit exceeded", core::ArchiveStatus::LimitExceeded);
                  return;
                }
                record(Id).Fields.insert(record(Id).Fields.end(), {FieldType, Private, FieldName.size()});
                record(Id).Text += FieldName;
              }
            }
          }
          else
          {
            const auto Target = type();
            if (!good() || Target == Id)
            {
              Lex.fail("Invalid type alias definition");
              return;
            }
            if (record(Target).Kind == Tag::Meta && record(Target).Type != Target)
            {
              Lex.fail("A direct type alias requires a defined target");
              return;
            }
            if (!Data.fields(record(Target).Fields.size()) || !Data.string(record(Target).Text.size()))
            {
              return;
            }
            record(Id) = record(Target);
            Data.TypeAliases.emplace(Id, Target);
            if (record(Id).Kind == Tag::Meta)
            {
              record(Id).Type = Id;
            }
            Types[typeKey(record(Id).Kind, record(Id).Fields)] = Id;
          }
        }
        Operand value(std::size_t Expected, std::size_t Depth = 0)
        {
          if (!good() || !depth(Depth))
          {
            return {};
          }
          if (Lex.token() == "@" || Lex.token() == "%" || Lex.token() == "^")
          {
            const auto Prefix = Lex.token();
            return {0, Expected, symbol(Prefix)};
          }
          if (!Expected)
          {
            Lex.fail("Expected a callee symbol");
            return {};
          }
          const auto Kind = record(Expected).Kind;
          if (Kind == Tag::Class)
          {
            Lex.expect("{");
            std::vector<Operand> Fields;
            const auto Types = record(Expected).Fields;
            while (good() && !Lex.take("}"))
            {
              if (!Fields.empty())
              {
                Lex.expect(",");
              }
              const auto Index = 2 + Fields.size() * 3;
              if (Index >= Types.size() || !Data.fields(1))
              {
                Lex.fail("Invalid class constant field count");
                return {};
              }
              Fields.push_back(value(Types[Index], Depth + 1));
            }
            const auto Id = add(Tag::ClassConstant, Expected);
            operands(Id, Fields);
            return {Id, Expected, {}};
          }
          if (Kind == Tag::Array)
          {
            const auto ElementType = record(Expected).Fields[0];
            Lex.expect("[");
            std::vector<Operand> Elements;
            while (good() && !Lex.take("]"))
            {
              if (!Elements.empty())
              {
                Lex.expect(",");
              }
              if (Elements.size() >= Data.Limits.MaxFields || !Data.fields(1))
              {
                Data.fail("Array constant element limit exceeded", core::ArchiveStatus::LimitExceeded);
                return {};
              }
              Elements.push_back(value(ElementType, Depth + 1));
            }
            const auto Id = add(Tag::ArrayConstant, Expected);
            operands(Id, Elements);
            return {Id, Expected, {}};
          }
          if (Kind == Tag::Meta)
          {
            return {type(), Expected, {}};
          }
          if (Kind == Tag::Bool)
          {
            const auto Value = Lex.take("true");
            if (!Value)
            {
              Lex.expect("false");
            }
            return {add(Tag::BoolConstant, Expected, 0, {Value}), Expected, {}};
          }
          if (Kind == Tag::Slice && Lex.quoted())
          {
            return {add(Tag::StringConstant, Expected, 0, {}, string()), Expected, {}};
          }
          if (Kind == Tag::Float)
          {
            Lex.expect("bits");
            Lex.expect("(");
            auto Spelling = Lex.token();
            Lex.next();
            std::uint64_t Bits = 0;
            if (Spelling.starts_with("0x"))
            {
              Spelling.remove_prefix(2);
            }
            else
            {
              Lex.fail("Expected hexadecimal floating-point bits");
            }
            const auto Parsed = std::from_chars(Spelling.data(), Spelling.data() + Spelling.size(), Bits, 16);
            if (Parsed.ec != std::errc{} || Parsed.ptr != Spelling.data() + Spelling.size())
            {
              Lex.fail("Invalid floating-point bits");
            }
            Lex.expect(")");
            const auto Width = record(Expected).Fields[0];
            if (Width < 64 && Bits >> Width)
            {
              Lex.fail("Floating-point bits exceed type width");
            }
            return {add(Tag::FloatConstant, Expected, 0, {Bits}), Expected, {}};
          }
          if (Kind == Tag::Integer)
          {
            auto Spelling = Lex.token();
            Lex.next();
            if (!Data.charge(Spelling.size(), 4))
            {
              return {};
            }
            const auto Width = static_cast<unsigned>(record(Expected).Fields[0]);
            if (!Data.fields((static_cast<std::uint64_t>(Width) + 63) / 64))
            {
              return {};
            }
            bool Negative = Spelling.starts_with("-");
            if (Negative)
            {
              Spelling.remove_prefix(1);
            }
            llvm::APInt Bits;
            if (Spelling.empty() || llvm::StringRef(Spelling.data(), Spelling.size()).getAsInteger(10, Bits))
            {
              Lex.fail("Invalid integer literal");
              return {};
            }
            const bool Signed = record(Expected).Fields[1] != 0;
            if (Bits.getActiveBits() > Width || (Negative && (!Signed || (Bits.getActiveBits() == Width && !Bits.isPowerOf2()))) || (!Negative && Signed && Bits.getActiveBits() >= Width))
            {
              Lex.fail("Integer literal exceeds type width");
              return {};
            }
            Bits = Bits.zextOrTrunc(Width);
            if (Negative)
            {
              Bits.negate();
            }
            std::vector<std::uint64_t> Words(Bits.getRawData(), Bits.getRawData() + Bits.getNumWords());
            return {add(Tag::IntegerConstant, Expected, 0, std::move(Words)), Expected, {}};
          }
          Lex.fail("Expected an operand of the declared type");
          return {};
        }
        Operand operand()
        {
          const auto Type = type();
          return value(Type);
        }
        void operands(std::size_t Id, const std::vector<Operand> &Values)
        {
          if (!Id || !good())
          {
            return;
          }
          for (const auto &Value : Values)
          {
            if (record(Id).Fields.size() >= Data.Limits.MaxFields)
            {
              Data.fail("Module archive field limit exceeded", core::ArchiveStatus::LimitExceeded);
              return;
            }
            if (!Data.fields(1))
            {
              return;
            }
            if (!Value.Name.empty())
            {
              Fixups.push_back({Id, record(Id).Fields.size(), Value});
            }
            record(Id).Fields.push_back(Value.Id);
          }
        }
        void instruction(std::size_t Parent)
        {
          std::string Name;
          if (Lex.token() == "%")
          {
            Name = symbol("%");
            Lex.expect("=");
          }
          const auto Opcode = Lex.token();
          Lex.next();
          std::size_t Result = 0;
          Tag Kind = Tag::Return;
          std::vector<std::uint64_t> Fields;
          std::vector<Operand> Values;
          if (Opcode == "add")
          {
            Kind = Tag::Add;
            Result = type();
            Values.push_back(value(Result));
            Lex.expect(",");
            Values.push_back(value(Result));
          }
          else if (Opcode == "class.value")
          {
            Kind = Tag::ClassValue;
            Result = type();
            if (!good() || record(Result).Kind != Tag::Class)
            {
              Lex.fail("Expected class result type");
              return;
            }
            const auto Types = record(Result).Fields;
            Lex.expect("{");
            while (good() && !Lex.take("}"))
            {
              if (!Values.empty())
              {
                Lex.expect(",");
              }
              const auto Index = 2 + Values.size() * 3;
              if (Index >= Types.size() || !Data.fields(1))
              {
                Lex.fail("Invalid class constructor field count");
                return;
              }
              Values.push_back(value(Types[Index]));
            }
          }
          else if (Opcode == "field.pointer" || Opcode == "field.extract")
          {
            Kind = Opcode == "field.pointer" ? Tag::FieldPointer : Tag::FieldExtract;
            Result = type();
            Lex.expect(",");
            Fields.push_back(Lex.number());
            Lex.expect(",");
            Values.push_back(operand());
          }
          else if (Opcode == "array" || Opcode == "array.repeat")
          {
            Kind = Tag::ArrayValue;
            Result = type();
            if (!good() || record(Result).Kind != Tag::Array)
            {
              Lex.fail("Expected array result type");
              return;
            }
            const auto ElementType = record(Result).Fields[0];
            Fields.push_back(Opcode == "array.repeat");
            Lex.expect("[");
            while (good() && !Lex.take("]"))
            {
              if (!Values.empty())
              {
                Lex.expect(",");
              }
              if (Values.size() >= Data.Limits.MaxFields || !Data.fields(1))
              {
                Data.fail("Array instruction element limit exceeded", core::ArchiveStatus::LimitExceeded);
                return;
              }
              Values.push_back(value(ElementType));
            }
          }
          else if (Opcode == "array.element" || Opcode == "array.extract")
          {
            Kind = Opcode == "array.element" ? Tag::ArrayElementPointer : Tag::ArrayExtract;
            Result = type();
            Lex.expect(",");
            Values.push_back(operand());
            Lex.expect(",");
            Values.push_back(operand());
          }
          else if (Opcode == "alloca")
          {
            Kind = Tag::Alloca;
            const auto Element = type();
            Result = poolType(Tag::Pointer, {Element, 1});
            Values.push_back({Element});
          }
          else if (Opcode == "not")
          {
            Kind = Tag::LogicalNot;
            Result = poolType(Tag::Bool);
            Values.push_back(operand());
          }
          else if (Opcode == "and" || Opcode == "or")
          {
            Kind = Opcode == "and" ? Tag::LogicalAnd : Tag::LogicalOr;
            Result = poolType(Tag::Bool);
            const auto OperandType = type();
            Values.push_back(value(OperandType));
            Lex.expect(",");
            Values.push_back(value(OperandType));
          }
          else if (Opcode == "cmp")
          {
            Kind = Tag::Compare;
            Result = poolType(Tag::Bool);
            const auto *Predicate = comparisonPredicateInfo(Lex.token());
            if (!Predicate)
            {
              Lex.fail("Invalid comparison predicate");
              return;
            }
            Fields.push_back(Predicate->Wire);
            Lex.next();
            const auto OperandType = type();
            Values.push_back(value(OperandType));
            Lex.expect(",");
            Values.push_back(value(OperandType));
          }
          else if (Opcode == "load")
          {
            Kind = Tag::Load;
            Result = type();
            Lex.expect(",");
            Values.push_back(operand());
          }
          else if (Opcode == "store")
          {
            Kind = Tag::Store;
            Result = poolType(Tag::Void);
            const auto Stored = operand();
            Lex.expect(",");
            Values.push_back(operand());
            Values.push_back(Stored);
          }
          else if (Opcode == "cstring")
          {
            Kind = Tag::CString;
            const auto Byte = poolType(Tag::Integer, {8, false});
            Result = poolType(Tag::Pointer, {Byte, 1});
            Values.push_back(operand());
          }
          else if (Opcode == "ret")
          {
            Kind = Tag::Return;
            Result = poolType(Tag::Void);
            if (!Lex.take("void"))
            {
              Values.push_back(operand());
            }
          }
          else if (Opcode == "call")
          {
            Kind = Tag::Call;
            Result = type();
            Values.push_back(value(0));
            Lex.expect("(");
            while (good() && !Lex.take(")"))
            {
              if (Values.size() > 1)
              {
                Lex.expect(",");
              }
              if (Values.size() >= Data.Limits.MaxFields)
              {
                Data.fail("Call argument limit exceeded", core::ArchiveStatus::LimitExceeded);
                return;
              }
              if (!Data.fields(1))
              {
                return;
              }
              Values.push_back(operand());
            }
          }
          else if (Opcode == "br")
          {
            Result = poolType(Tag::Void);
            if (Lex.token() == "^")
            {
              Kind = Tag::Branch;
              Values.push_back({0, 0, symbol("^")});
            }
            else
            {
              Kind = Tag::ConditionalBranch;
              Values.push_back(operand());
              Lex.expect(",");
              Values.push_back({0, 0, symbol("^")});
              Lex.expect(",");
              Values.push_back({0, 0, symbol("^")});
            }
          }
          else
          {
            Lex.fail("Unknown IR instruction");
            return;
          }
          if (!good())
          {
            return;
          }
          const auto Id = add(Kind, Result, Parent, std::move(Fields));
          operands(Id, Values);
          if (!Name.empty())
          {
            define(Name, Id);
          }
          else if (record(Result).Kind != Tag::Void)
          {
            Lex.fail("Non-void instruction requires a result name");
          }
        }
        void function(std::size_t Parent, std::size_t Depth)
        {
          const bool Body = Lex.take("define");
          if (!Body)
          {
            Lex.expect("declare");
          }
          const auto Return = type();
          const auto Symbol = symbol("@");
          const auto Name = name(Symbol);
          const auto Id = add(Tag::Function, 0, Parent, {0, 0, 0, 0}, Name);
          define(Symbol, Id);
          Lex.expect("(");
          std::vector<std::uint64_t> Signature{Return};
          while (good() && !Lex.take(")"))
          {
            if (Signature.size() > 1)
            {
              Lex.expect(",");
            }
            if (Signature.size() >= Data.Limits.MaxFields)
            {
              Data.fail("Function parameter limit exceeded", core::ArchiveStatus::LimitExceeded);
              return;
            }
            const auto Type = type();
            const auto Kind = Lex.take("named") ? 1 : Lex.take("variadic") ? 2
                                                                           : 0;
            const auto ParameterSymbol = symbol("%");
            const auto ParameterName = name(ParameterSymbol);
            const auto Parameter = add(Tag::Parameter, Type, Id, {Signature.size() - 1, static_cast<std::uint64_t>(Kind)}, ParameterName);
            define(ParameterSymbol, Parameter);
            Signature.push_back(Type);
          }
          const auto Type = poolType(Tag::FunctionType, std::move(Signature));
          if (!good())
          {
            return;
          }
          record(Id).Type = Type;
          if (Lex.take("cc"))
          {
            if (Lex.take("fast"))
            {
              record(Id).Fields[0] = 1;
            }
            else if (Lex.take("cold"))
            {
              record(Id).Fields[0] = 2;
            }
            else
            {
              Lex.expect("c");
            }
          }
          if (Lex.take("linkage"))
          {
            if (Lex.take("c"))
            {
              record(Id).Fields[1] = 1;
            }
            else
            {
              Lex.expect("ink");
            }
          }
          if (Lex.take("visibility"))
          {
            if (Lex.take("private"))
            {
              record(Id).Fields[2] = 1;
            }
            else
            {
              Lex.expect("public");
            }
          }
          if (Lex.take("binding"))
          {
            if (Lex.take("import"))
            {
              record(Id).Fields[3] = 1;
            }
            else if (Lex.take("export"))
            {
              record(Id).Fields[3] = 2;
            }
            else
            {
              Lex.expect("local");
            }
          }
          if (Lex.take("reflect"))
          {
            const auto Owner = type();
            const bool Initializer = Lex.take("initializer");
            std::uint64_t Field = 0;
            if (Initializer)
            {
              Field = Lex.number();
              if (Field == std::numeric_limits<std::uint64_t>::max())
              {
                Lex.fail("Invalid initializer field index");
                return;
              }
              ++Field;
            }
            else
            {
              Lex.expect("method");
            }
            if (!Data.fields(2))
            {
              return;
            }
            record(Id).Fields.insert(record(Id).Fields.end(), {Owner, Field});
          }
          if (Lex.take("generics"))
          {
            if (record(Id).Fields.size() == 4)
            {
              if (!Data.fields(2))
              {
                return;
              }
              record(Id).Fields.insert(record(Id).Fields.end(), {0, 0});
            }
            Lex.expect("[");
            bool First = true;
            while (good() && !Lex.take("]"))
            {
              if (!First)
              {
                Lex.expect(",");
              }
              First = false;
              const auto Argument = type();
              if (!Data.fields(1))
              {
                return;
              }
              record(Id).Fields.push_back(Argument);
            }
          }
          if (Lex.take("scope"))
          {
            record(Id).Linkage = string();
          }
          if ((Body && record(Id).Fields[3] == 1) || (!Body && record(Id).Fields[3] == 2))
          {
            Lex.fail("Native import must be a declaration and native export must have a body");
            return;
          }
          if (!Body || !Lex.expect("{") || !depth(Depth))
          {
            return;
          }
          std::size_t Block = 0;
          while (good() && !Lex.take("}"))
          {
            auto Lookahead = Lex;
            Lookahead.next();
            if (Lex.token() == "^" || Lookahead.token() == ":")
            {
              std::string Label;
              if (Lex.token() == "^")
              {
                Label = symbol("^");
              }
              else if (Lex.quoted())
              {
                Label = "^" + string();
              }
              else
              {
                Label = "^" + std::string(Lex.token());
                Lex.next();
              }
              Lex.expect(":");
              Block = add(Tag::Block, poolType(Tag::Label), Id);
              define(Label, Block);
            }
            else if (!Block)
            {
              Lex.fail("Function body requires a block label");
            }
            else
            {
              object(Block, Depth + 1);
            }
          }
        }
        void syntax()
        {
          Lex.expect("syntax");
          const auto Name = symbol("!");
          Lex.expect("ast");
          const auto Version = Lex.number();
          const auto Block = Lex.block(Data.Limits.MaxNestingDepth);
          if (!Lex.good())
          {
            return;
          }
          define(Name, add(Tag::AST, 0, 0, {}, "ast " + std::to_string(Version) + " " + std::string(Block)));
        }
        void declaration(std::size_t Parent, std::size_t Depth, bool Root)
        {
          if (!depth(Depth))
          {
            return;
          }
          Tag Kind = Tag::ModuleDecl;
          std::string Name;
          if (Root)
          {
            Lex.expect("declarations");
            Name = record(Parent).Text;
          }
          else
          {
            if (Lex.take("func"))
            {
              Kind = Tag::FunctionDecl;
            }
            else
            {
              Lex.expect("class");
              Kind = Tag::ClassDecl;
            }
            Name = string();
          }
          Lex.expect("from");
          const auto AST = symbol("!");
          Lex.expect("%");
          const auto Node = Lex.number();
          const auto Id = add(Kind, 0, Parent, {}, std::move(Name));
          operands(Id, {{0, 0, AST}, {static_cast<std::size_t>(Node)}});
          Lex.expect("{");
          while (good() && !Lex.take("}"))
          {
            declaration(Id, Depth + 1, false);
          }
        }
        void module(std::size_t Id, std::size_t Parent, std::size_t Depth)
        {
          if (!good() || !depth(Depth))
          {
            return;
          }
          Lex.expect("module");
          const auto Symbol = symbol("@");
          const auto Name = name(Symbol);
          const auto Type = poolType(Tag::ModuleType);
          if (!good())
          {
            return;
          }
          record(Id).Type = Type;
          record(Id).Parent = Parent;
          record(Id).Text = Name;
          define(Symbol, Id);
          const auto Block = add(Tag::Block, poolType(Tag::Label), Id);
          if (Lex.take("entry"))
          {
            define(symbol("^"), Block);
          }
          if (Lex.take("identity"))
          {
            record(Id).Linkage = string();
          }
          Lex.expect("{");
          while (good() && !Lex.take("}"))
          {
            if (Lex.token() == "type")
            {
              typeDefinition();
            }
            else if (Lex.take("reflect"))
            {
              Lex.expect("type");
              const auto Class = type();
              if (!Data.fields(1) || record(Id).Fields.size() >= Data.Limits.MaxFields)
              {
                Data.fail("Module reflected type limit exceeded", core::ArchiveStatus::LimitExceeded);
                return;
              }
              record(Id).Fields.push_back(Class);
            }
            else if (Lex.token() == "syntax")
            {
              syntax();
            }
            else if (Lex.token() == "declarations")
            {
              declaration(Id, Depth + 1, true);
            }
            else
            {
              object(Block, Depth + 1);
            }
          }
        }
        void object(std::size_t Parent, std::size_t Depth)
        {
          if (!good() || !depth(Depth))
          {
            return;
          }
          if (Lex.token() == "module")
          {
            const auto Id = add(Tag::Module);
            module(Id, Parent, Depth);
          }
          else if (Lex.token() == "define" || Lex.token() == "declare")
          {
            function(Parent, Depth);
          }
          else if (Lex.take("block"))
          {
            const auto Name = symbol("^");
            const auto Id = add(Tag::Block, poolType(Tag::Label), Parent);
            define(Name, Id);
            Lex.expect("{");
            while (good() && !Lex.take("}"))
            {
              object(Id, Depth + 1);
            }
          }
          else
          {
            instruction(Parent);
          }
        }

        State &Data;
        core::archive::Lexer Lex;
        std::unordered_map<std::string, Alias> Aliases;
        std::unordered_map<std::string, std::size_t> Types;
        std::unordered_map<std::string, std::size_t> Symbols;
        std::vector<Fixup> Fixups;
    };
  } // namespace
} // namespace ink::ir::archive
