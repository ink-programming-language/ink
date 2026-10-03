namespace ink::ir::archive
{
  namespace
  {
    class TextWriter
    {
      public:
        explicit TextWriter(State &Data)
            : Data(Data),
              Names(Data.Records.size() + 1),
              Children(Data.Records.size() + 1),
              BlockCounts(Data.Records.size() + 1),
              Used(Data.Records.size() + 1),
              TypeAliases(Data.Records.size() + 1)
        {
        }

        std::string run()
        {
          std::size_t TypeNumber = 0, ValueNumber = 0, BlockNumber = 0, ASTNumber = 0;
          for (std::size_t Id = 1; Id <= Data.Records.size(); ++Id)
          {
            const auto &R = record(Id);
            if (R.Parent)
            {
              Children[R.Parent].push_back(Id);
            }
            if (R.Kind >= Tag::Array && R.Kind <= Tag::FunctionType)
            {
              Names[Id] = "!t" + std::to_string(TypeNumber++);
            }
            else if (R.Kind == Tag::AST)
            {
              Names[Id] = "!ast" + std::to_string(ASTNumber++);
            }
            else if (R.Kind == Tag::Module || R.Kind == Tag::Function)
            {
              Names[Id] = unique('@', R.Text);
            }
            else if (R.Kind == Tag::Parameter)
            {
              Names[Id] = unique('%', R.Text.empty() ? "arg" : R.Text);
            }
            else if (R.Kind == Tag::Block)
            {
              const auto ParentKind = record(R.Parent).Kind;
              Names[Id] = unique('^', ParentKind == Tag::Module ? "module_entry" : ParentKind == Tag::Function && BlockCounts[R.Parent]++ == 0 ? "entry"
                                                                                                                                               : "bb" + std::to_string(BlockNumber++));
            }
            else if (isInstruction(R.Kind))
            {
              Names[Id] = unique('%', "v" + std::to_string(ValueNumber++));
            }
            if (R.Kind != Tag::Function && R.Type)
            {
              TypeAliases[R.Type] = true;
            }
            if (R.Kind == Tag::Array || R.Kind == Tag::Slice || R.Kind == Tag::Pointer || R.Kind == Tag::Reference || R.Kind == Tag::Alloca)
            {
              TypeAliases[R.Fields[0]] = true;
            }
            if (R.Kind == Tag::FunctionType)
            {
              for (auto Ref : R.Fields)
              {
                TypeAliases[Ref] = true;
              }
            }
            if (isInstruction(R.Kind))
            {
              for (std::size_t Index = R.Kind == Tag::Compare ? 1 : 0; Index < R.Fields.size(); ++Index)
              {
                const auto Ref = R.Fields[Index];
                Used[Ref] = true;
                if (record(Ref).Kind <= Tag::FunctionType)
                {
                  TypeAliases[Ref] = true;
                }
              }
            }
          }
          put("ink-ir " + std::to_string(ModuleTextVersion) + "\n");
          module(1, 0);
          return Data.good() ? std::move(Output) : std::string{};
        }

      private:
        const Record &record(std::size_t Id) const
        {
          return Data.Records[Id - 1];
        }
        void put(std::string_view Text)
        {
          if (!Data.good())
          {
            return;
          }
          if (Text.size() > Data.Limits.MaxArchiveBytes - Output.size())
          {
            Data.fail("Module archive byte limit exceeded", ModuleArchiveStatus::LimitExceeded);
            return;
          }
          if (Data.charge(Text.size(), 2))
          {
            Output += Text;
          }
        }
        void line(std::size_t Depth, std::string_view Text)
        {
          put(std::string(Depth * 2, ' '));
          put(Text);
          put("\n");
        }
        std::string unique(char Prefix, std::string_view Preferred)
        {
          std::string Name(Preferred);
          const auto Key = std::string(1, Prefix) + Name;
          auto &Count = Counts[Key];
          if (Count++)
          {
            Name += "." + std::to_string(Count - 1);
          }
          // Reserve generated names as well, so a user name such as f.1 never collides.
          while (!Symbols.insert(std::string(1, Prefix) + Name).second)
          {
            Name += ".";
          }
          return std::string(1, Prefix) + spelling(Name);
        }
        static std::string spelling(std::string_view Name)
        {
          const auto Valid = [](char C)
          {
            return (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || (C >= '0' && C <= '9') || C == '_' || C == '.' || C == '-';
          };
          return !Name.empty() && std::all_of(Name.begin(), Name.end(), Valid) ? std::string(Name) : core::archive::quote(Name);
        }
        std::string originalName(std::size_t Id) const
        {
          return Names[Id].substr(1) == spelling(record(Id).Text) ? "" : " name " + core::archive::quote(record(Id).Text);
        }
        std::string type(std::size_t Id) const
        {
          const auto &R = record(Id);
          switch (R.Kind)
          {
          case Tag::Meta:
            return "type";
          case Tag::Void:
            return "void";
          case Tag::Bool:
            return "bool";
          case Tag::Label:
            return "label";
          case Tag::ModuleType:
            return "module";
          case Tag::Integer:
            return std::string(R.Fields[1] ? "i" : "u") + std::to_string(R.Fields[0]);
          case Tag::Float:
            return "f" + std::to_string(R.Fields[0]);
          default:
            return Names[Id];
          }
        }
        std::string typeDefinition(std::size_t Id) const
        {
          const auto &R = record(Id);
          switch (R.Kind)
          {
          case Tag::Class:
            return "class " + core::archive::quote(R.Text);
          case Tag::Enum:
            return "enum " + core::archive::quote(R.Text);
          case Tag::Interface:
            return "interface " + core::archive::quote(R.Text);
          case Tag::Array:
            return "[" + std::to_string(R.Fields[1]) + " x " + type(R.Fields[0]) + "]";
          case Tag::Slice:
          case Tag::Pointer:
          case Tag::Reference:
            return std::string(R.Kind == Tag::Slice ? "slice" : R.Kind == Tag::Pointer ? "ptr"
                                                                                       : "ref") +
                   "<" + (R.Fields[1] ? "rw, " : "ro, ") + type(R.Fields[0]) + ">";
          case Tag::FunctionType:
          {
            std::string Result = "fn(";
            for (std::size_t I = 1; I < R.Fields.size(); ++I)
            {
              if (I != 1)
              {
                Result += ", ";
              }
              Result += type(R.Fields[I]);
            }
            return Result + ") -> " + type(R.Fields[0]);
          }
          default:
            return type(Id);
          }
        }
        std::string value(std::size_t Id) const
        {
          const auto &R = record(Id);
          if (R.Kind <= Tag::FunctionType)
          {
            return type(Id);
          }
          if (R.Kind == Tag::IntegerConstant)
          {
            const auto &T = record(R.Type);
            llvm::APInt Bits(static_cast<unsigned>(T.Fields[0]), llvm::ArrayRef<std::uint64_t>(R.Fields));
            llvm::SmallString<64> Text;
            Bits.toString(Text, 10, T.Fields[1] != 0);
            return std::string(Text);
          }
          if (R.Kind == Tag::FloatConstant)
          {
            char Buffer[17];
            const auto End = std::to_chars(Buffer, Buffer + sizeof(Buffer), R.Fields[0], 16).ptr;
            return "bits(0x" + std::string(Buffer, End) + ")";
          }
          if (R.Kind == Tag::BoolConstant)
          {
            return R.Fields[0] ? "true" : "false";
          }
          if (R.Kind == Tag::StringConstant)
          {
            return core::archive::quote(R.Text);
          }
          return Names[Id];
        }
        std::string operand(std::size_t Id) const
        {
          return type(record(Id).Type) + " " + value(Id);
        }
        void declarations(std::size_t Id, std::size_t Depth)
        {
          const auto &R = record(Id);
          const auto Prefix = R.Kind == Tag::ModuleDecl ? std::string("declarations") : std::string(R.Kind == Tag::FunctionDecl ? "func " : "class ") + core::archive::quote(R.Text);
          line(Depth, Prefix + " from " + Names[R.Fields[0]] + " %" + std::to_string(R.Fields[1]) + " {");
          for (auto Child : Children[Id])
          {
            declarations(Child, Depth + 1);
          }
          line(Depth, "}");
        }
        void module(std::size_t Id, std::size_t Depth)
        {
          const auto Block = Children[Id][0];
          line(Depth, "module " + Names[Id] + originalName(Id) + (Used[Block] ? " entry " + Names[Block] : "") + " {");
          if (Id == 1)
          {
            for (std::size_t T = 1; T <= Data.Records.size(); ++T)
            {
              if (record(T).Kind >= Tag::Array && record(T).Kind <= Tag::FunctionType && TypeAliases[T])
              {
                line(Depth + 1, "type " + Names[T] + " = " + typeDefinition(T));
              }
            }
            for (std::size_t A = 1; A <= Data.Records.size(); ++A)
            {
              if (record(A).Kind == Tag::AST)
              {
                line(Depth + 1, "syntax " + Names[A] + " " + record(A).Text);
              }
            }
          }
          for (auto Child : Children[Id])
          {
            if (record(Child).Kind == Tag::ModuleDecl)
            {
              declarations(Child, Depth + 1);
            }
          }
          for (auto Child : Children[Block])
          {
            object(Child, Depth + 1);
          }
          line(Depth, "}");
        }
        void function(std::size_t Id, std::size_t Depth)
        {
          const auto &R = record(Id);
          const bool Body = std::any_of(Children[Id].begin(), Children[Id].end(), [&](auto Child)
                                        {
                                          return record(Child).Kind == Tag::Block;
                                        });
          std::string Header = std::string(Body ? "define " : "declare ") + type(record(R.Type).Fields[0]) + " " + Names[Id] + originalName(Id) + "(";
          bool First = true;
          for (auto Child : Children[Id])
          {
            const auto &P = record(Child);
            if (P.Kind != Tag::Parameter)
            {
              continue;
            }
            if (!First)
            {
              Header += ", ";
            }
            First = false;
            Header += type(P.Type) + " " + (P.Fields[1] == 1 ? "named " : P.Fields[1] == 2 ? "variadic "
                                                                                           : "") +
                      Names[Child] + originalName(Child);
          }
          Header += ")";
          if (R.Fields[0])
          {
            Header += R.Fields[0] == 1 ? " cc fast" : " cc cold";
          }
          if (R.Fields[1])
          {
            Header += " linkage c";
          }
          if (R.Fields[2])
          {
            Header += " visibility private";
          }
          if (R.Fields[3])
          {
            Header += R.Fields[3] == 1 ? " binding import" : " binding export";
          }
          line(Depth, Header + (Body ? " {" : ""));
          if (!Body)
          {
            return;
          }
          for (auto Child : Children[Id])
          {
            if (record(Child).Kind != Tag::Block)
            {
              continue;
            }
            line(Depth, Names[Child].substr(1) + ":");
            for (auto Item : Children[Child])
            {
              object(Item, Depth + 1);
            }
          }
          line(Depth, "}");
        }
        void object(std::size_t Id, std::size_t Depth)
        {
          const auto &R = record(Id);
          if (R.Kind == Tag::Module)
          {
            module(Id, Depth);
            return;
          }
          if (R.Kind == Tag::Function)
          {
            function(Id, Depth);
            return;
          }
          if (R.Kind == Tag::Block)
          {
            line(Depth, "block " + Names[Id] + " {");
            for (auto Child : Children[Id])
            {
              object(Child, Depth + 1);
            }
            line(Depth, "}");
            return;
          }
          std::string Text = record(R.Type).Kind != Tag::Void || Used[Id] ? Names[Id] + " = " : "";
          const auto &F = R.Fields;
          switch (R.Kind)
          {
          case Tag::Add:
            Text += "add " + type(R.Type) + " " + value(F[0]) + ", " + value(F[1]);
            break;
          case Tag::LogicalNot:
            Text += "not " + operand(F[0]);
            break;
          case Tag::LogicalAnd:
            Text += "and " + operand(F[0]) + ", " + value(F[1]);
            break;
          case Tag::LogicalOr:
            Text += "or " + operand(F[0]) + ", " + value(F[1]);
            break;
          case Tag::Compare:
            Text += "cmp " + std::string(comparisonPredicateInfo(F[0])->Text) + " " + operand(F[1]) + ", " + value(F[2]);
            break;
          case Tag::Alloca:
            Text += "alloca " + type(F[0]);
            break;
          case Tag::Load:
            Text += "load " + type(R.Type) + ", " + operand(F[0]);
            break;
          case Tag::Store:
            Text += "store " + operand(F[1]) + ", " + operand(F[0]);
            break;
          case Tag::CString:
            Text += "cstring " + operand(F[0]);
            break;
          case Tag::Return:
            Text += "ret " + (F.empty() ? std::string("void") : operand(F[0]));
            break;
          case Tag::Branch:
            Text += "br " + value(F[0]);
            break;
          case Tag::ConditionalBranch:
            Text += "br " + operand(F[0]) + ", " + value(F[1]) + ", " + value(F[2]);
            break;
          case Tag::Call:
            Text += "call " + type(R.Type) + " " + value(F[0]) + "(";
            for (std::size_t I = 1; I < F.size(); ++I)
            {
              if (I != 1)
              {
                Text += ", ";
              }
              Text += operand(F[I]);
            }
            Text += ")";
            break;
          default:
            Data.fail("Unsupported text IR value", ModuleArchiveStatus::InvalidInput);
            return;
          }
          line(Depth, Text);
        }

        State &Data;
        std::string Output;
        std::vector<std::string> Names;
        std::vector<std::vector<std::size_t>> Children;
        std::vector<std::size_t> BlockCounts;
        std::vector<bool> Used;
        std::vector<bool> TypeAliases;
        std::unordered_map<std::string, std::size_t> Counts;
        std::unordered_set<std::string> Symbols;
    };
  } // namespace
} // namespace ink::ir::archive
