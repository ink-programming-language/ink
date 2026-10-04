#include "ink/abi/name_mangling.h"

#include <utf8proc.h>

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <limits>

namespace ink::abi
{
  namespace
  {
    bool letter(char C)
    {
      return (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z');
    }

    bool digit(char C)
    {
      return C >= '0' && C <= '9';
    }

    int hex(char C)
    {
      return digit(C) ? C - '0' : C >= 'A' && C <= 'F' ? C - 'A' + 10 : -1;
    }

    bool number(std::string_view Text, std::uint64_t &Value)
    {
      if (Text.empty() || (Text.size() > 1 && Text.front() == '0') || !std::all_of(Text.begin(), Text.end(), digit))
      {
        return false;
      }
      const auto Result = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
      return Result.ec == std::errc{} && Result.ptr == Text.data() + Text.size();
    }

    bool readRecord(std::string_view Text, std::size_t &Offset, Record &Result)
    {
      if (Offset >= Text.size() || !letter(Text[Offset]))
      {
        return false;
      }
      Result.Tag = Text[Offset++];
      const auto Start = Offset;
      while (Offset < Text.size() && digit(Text[Offset]))
      {
        ++Offset;
      }
      std::uint64_t Length = 0;
      if (Offset == Text.size() || Text[Offset] != '_' || !number(Text.substr(Start, Offset - Start), Length) || Length > Text.size() - Offset - 1)
      {
        return false;
      }
      ++Offset;
      Result.Payload = Text.substr(Offset, static_cast<std::size_t>(Length));
      Offset += static_cast<std::size_t>(Length);
      return true;
    }

    bool normalized(std::string_view Text)
    {
      if (Text.empty() || Text.find('\0') != std::string_view::npos)
      {
        return false;
      }
      utf8proc_uint8_t *Output = nullptr;
      const auto Length = utf8proc_map(reinterpret_cast<const utf8proc_uint8_t *>(Text.data()), static_cast<utf8proc_ssize_t>(Text.size()), &Output, static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_COMPOSE));
      const bool Valid = Length >= 0 && static_cast<std::size_t>(Length) == Text.size() && std::equal(Text.begin(), Text.end(), reinterpret_cast<const char *>(Output));
      std::free(Output);
      return Valid;
    }
  } // namespace

  std::string encodeRecord(const Record &Value)
  {
    return Value.Tag + std::to_string(Value.Payload.size()) + '_' + Value.Payload;
  }

  Record record(char Tag, std::span<const Record> Children)
  {
    Record Result{Tag, {}};
    for (const auto &Child : Children)
    {
      Result.Payload += encodeRecord(Child);
    }
    return Result;
  }

  Record record(char Tag, std::initializer_list<Record> Children)
  {
    return record(Tag, std::span<const Record>(Children.begin(), Children.size()));
  }

  Record nameRecord(std::string_view Name)
  {
    constexpr char Hex[] = "0123456789ABCDEF";
    Record Result{'N', {}};
    for (const unsigned char Byte : Name)
    {
      if (letter(static_cast<char>(Byte)) || digit(static_cast<char>(Byte)))
      {
        Result.Payload += static_cast<char>(Byte);
      }
      else
      {
        Result.Payload += '_';
        Result.Payload += Hex[Byte >> 4];
        Result.Payload += Hex[Byte & 15];
      }
    }
    return Result;
  }

  std::optional<std::string> decodeName(const Record &Name)
  {
    if (Name.Tag != 'N')
    {
      return std::nullopt;
    }
    std::string Result;
    for (std::size_t Index = 0; Index < Name.Payload.size(); ++Index)
    {
      const char C = Name.Payload[Index];
      if (letter(C) || digit(C))
      {
        Result += C;
      }
      else if (C == '_' && Index + 2 < Name.Payload.size() && hex(Name.Payload[Index + 1]) >= 0 && hex(Name.Payload[Index + 2]) >= 0)
      {
        const char Decoded = static_cast<char>((hex(Name.Payload[Index + 1]) << 4) | hex(Name.Payload[Index + 2]));
        if (letter(Decoded) || digit(Decoded))
        {
          return std::nullopt;
        }
        Result += Decoded;
        Index += 2;
      }
      else
      {
        return std::nullopt;
      }
    }
    return normalized(Result) ? std::optional<std::string>(std::move(Result)) : std::nullopt;
  }

  std::optional<std::vector<Record>> childRecords(const Record &Value, std::size_t PrefixBytes)
  {
    if (PrefixBytes > Value.Payload.size())
    {
      return std::nullopt;
    }
    std::vector<Record> Result;
    std::size_t Offset = PrefixBytes;
    while (Offset < Value.Payload.size())
    {
      Record Child;
      if (!readRecord(Value.Payload, Offset, Child))
      {
        return std::nullopt;
      }
      Result.push_back(std::move(Child));
    }
    return Result;
  }

  Record packageRecord(const PackageIdentity &Package)
  {
    std::vector<Record> Names;
    for (const auto &Name : Package.Name)
    {
      Names.push_back(nameRecord(Name));
    }
    auto Variants = Package.Variant;
    std::sort(Variants.begin(), Variants.end());
    std::vector<Record> Entries;
    for (const auto &[Key, Value] : Variants)
    {
      Entries.push_back(record('E', {nameRecord(Key), nameRecord(Value)}));
    }
    return record('P', {nameRecord(Package.Authority), record('L', Names), nameRecord(Package.Revision), record('L', Entries)});
  }

  Record moduleRecord(const ModuleIdentity &Module)
  {
    std::vector<Record> Names;
    for (const auto &Name : Module.Path)
    {
      Names.push_back(nameRecord(Name));
    }
    return record('M', Names);
  }

  std::vector<std::string> modulePath(std::string_view DottedName)
  {
    std::vector<std::string> Result;
    std::size_t Begin = 0;
    do
    {
      const auto End = DottedName.find('.', Begin);
      Result.emplace_back(DottedName.substr(Begin, End == std::string_view::npos ? DottedName.size() - Begin : End - Begin));
      if (End == std::string_view::npos)
      {
        break;
      }
      Begin = End + 1;
    } while (Begin <= DottedName.size());
    return Result;
  }

  std::optional<ModuleIdentity> moduleIdentity(const Record &Reflection)
  {
    if (Reflection.Tag != 'J' || !mangle(Reflection))
    {
      return std::nullopt;
    }
    const auto Root = *childRecords(Reflection);
    const auto Package = *childRecords(Root[0]);
    ModuleIdentity Result;
    Result.Package.Authority = *decodeName(Package[0]);
    Result.Package.Name.clear();
    const auto Names = *childRecords(Package[1]);
    for (const auto &Name : Names)
    {
      Result.Package.Name.push_back(*decodeName(Name));
    }
    Result.Package.Revision = *decodeName(Package[2]);
    const auto Entries = *childRecords(Package[3]);
    for (const auto &Entry : Entries)
    {
      const auto Pair = *childRecords(Entry);
      Result.Package.Variant.emplace_back(*decodeName(Pair[0]), *decodeName(Pair[1]));
    }
    const auto Path = *childRecords(Root[1]);
    for (const auto &Name : Path)
    {
      Result.Path.push_back(*decodeName(Name));
    }
    return Result;
  }

  namespace
  {
    // Schemas use the same records for IR-produced and decoded identities.
    class Validator
    {
      public:
        explicit Validator(ManglingLimits Limits)
            : Limits(Limits)
        {
        }

        bool symbol(const Record &Value, std::size_t Depth = 0)
        {
          if (!enter(Depth))
          {
            return false;
          }
          const auto Children = childRecords(Value);
          if (!Children)
          {
            return false;
          }
          const auto &C = *Children;
          switch (Value.Tag)
          {
          case 'F':
          {
            if (C.size() != 3 || !declaration(C[0], 'f', Depth + 1) || !instance(C[1], C[0], Depth + 1) || !signature(C[2], Depth + 1))
            {
              return false;
            }
            const auto Pattern = closedPattern(C[0], C[1], Depth + 1);
            const auto Signature = childRecords(C[2]);
            return Pattern && Signature && *Pattern == record('H', {(*Signature)[2], (*Signature)[3]});
          }
          case 'V':
            return C.size() == 4 && declaration(C[0], 'v', Depth + 1) && instance(C[1], C[0], Depth + 1) && atom(C[2], 'W', "rw") && C[3].Tag != 'v' && type(C[3], Depth + 1);
          case 'T':
            return C.size() == 1 && type(C[0], Depth + 1);
          case 'J':
            return C.size() == 2 && package(C[0], Depth + 1) && names(C[1], 'M');
          case 'I':
            return C.size() == 3 && C[0].Tag == 'c' && type(C[0], Depth + 1) && name(C[1]) && signature(C[2], Depth + 1);
          case 'H':
            return C.size() == 3 && name(C[0]) && (C[1].Tag == 'F' || C[1].Tag == 'I') && symbol(C[1], Depth + 1) && signature(C[2], Depth + 1);
          default:
            return false;
          }
        }

      private:
        bool enter(std::size_t Depth)
        {
          return Depth < Limits.MaxDepth && ++Records <= Limits.MaxRecords;
        }

        bool atom(const Record &Value, char Tag, std::string_view Choices)
        {
          return Value.Tag == Tag && Value.Payload.size() == 1 && Choices.find(Value.Payload.front()) != std::string_view::npos;
        }

        bool name(const Record &Value)
        {
          return ++Records <= Limits.MaxRecords && decodeName(Value).has_value();
        }

        bool names(const Record &Value, char Tag)
        {
          const auto Children = childRecords(Value);
          return Value.Tag == Tag && Children && !Children->empty() && std::all_of(Children->begin(), Children->end(), [&](const Record &Child)
          {
            return name(Child);
          });
        }

        bool package(const Record &Value, std::size_t Depth)
        {
          const auto Children = childRecords(Value);
          if (!enter(Depth) || Value.Tag != 'P' || !Children || Children->size() != 4)
          {
            return false;
          }
          const auto &C = *Children;
          if (!name(C[0]) || !names(C[1], 'L') || !name(C[2]) || C[3].Tag != 'L')
          {
            return false;
          }
          const auto Entries = childRecords(C[3]);
          if (!Entries)
          {
            return false;
          }
          std::string Previous;
          for (const auto &Entry : *Entries)
          {
            const auto Pair = childRecords(Entry);
            if (Entry.Tag != 'E' || !Pair || Pair->size() != 2 || !name((*Pair)[0]) || !name((*Pair)[1]))
            {
              return false;
            }
            const auto Key = *decodeName((*Pair)[0]);
            if (!Previous.empty() && Previous >= Key)
            {
              return false;
            }
            Previous = Key;
          }
          return true;
        }

        bool declaration(const Record &Value, char Kind, std::size_t Depth)
        {
          const auto Children = childRecords(Value);
          if (!enter(Depth) || Value.Tag != 'R' || !Children || Children->size() != 7)
          {
            return false;
          }
          const auto &C = *Children;
          if (!package(C[0], Depth + 1) || !names(C[1], 'M') || C[2].Tag != 'O' || !atom(C[3], 'K', std::string(1, Kind)) || !name(C[4]) || C[5].Tag != 'G' || C[6].Tag != 'H')
          {
            return false;
          }
          const auto Owners = childRecords(C[2]);
          const auto Parameters = childRecords(C[5]);
          if (!Owners || !Parameters)
          {
            return false;
          }
          for (const auto &Owner : *Owners)
          {
            if (Owner.Tag == 'F')
            {
              const auto F = childRecords(Owner);
              if (!F || F->size() != 2 || !declaration((*F)[0], 'f', Depth + 1) || !instance((*F)[1], (*F)[0], Depth + 1))
              {
                return false;
              }
            }
            else if (Owner.Tag == 'B')
            {
              const auto B = childRecords(Owner);
              std::uint64_t Index = 0;
              if (!B || B->size() != 1 || (*B)[0].Tag != 'D' || !number((*B)[0].Payload, Index))
              {
                return false;
              }
            }
            else if ((Owner.Tag != 'c' && Owner.Tag != 'e' && Owner.Tag != 'j') || !type(Owner, Depth + 1))
            {
              return false;
            }
          }
          // Generic schemas are representable, but arbitrary dependent expressions are not.
          for (const auto &Parameter : *Parameters)
          {
            if (!genericParameter(Parameter, Depth + 1))
            {
              return false;
            }
          }
          const auto Pattern = childRecords(C[6]);
          return Pattern && (Kind == 'f' ? Pattern->size() == 2 && receiver((*Pattern)[0], Depth + 1, true) && types((*Pattern)[1], Depth + 1, true) : Pattern->empty());
        }

        bool genericParameter(const Record &Value, std::size_t Depth)
        {
          if (!enter(Depth))
          {
            return false;
          }
          if (Value.Tag == 'T')
          {
            return Value.Payload.empty();
          }
          const auto C = childRecords(Value);
          return C && C->size() == 1 && (Value.Tag == 'V' ? type((*C)[0], Depth + 1, true) : Value.Tag == 'B' && genericParameter((*C)[0], Depth + 1));
        }

        bool instance(const Record &Value, const Record &Declaration, std::size_t Depth)
        {
          if (!enter(Depth) || Value.Tag != 'X')
          {
            return false;
          }
          const auto Arguments = childRecords(Value);
          const auto Decl = childRecords(Declaration);
          const auto Parameters = Decl && Decl->size() == 7 ? childRecords((*Decl)[5]) : std::nullopt;
          if (!Arguments || !Parameters || Arguments->size() != Parameters->size())
          {
            return false;
          }
          for (std::size_t Index = 0; Index < Arguments->size(); ++Index)
          {
            if (!argument((*Arguments)[Index], (*Parameters)[Index], Depth + 1))
            {
              return false;
            }
          }
          return closedPattern(Declaration, Value, Depth + 1).has_value();
        }

        using Frames = std::vector<std::vector<Record>>;

        bool frames(const Record &Declaration, const Record &Instance, Frames &Result, std::size_t Depth)
        {
          if (!enter(Depth))
          {
            return false;
          }
          const auto D = childRecords(Declaration);
          const auto X = childRecords(Instance);
          if (!D || D->size() != 7 || !X)
          {
            return false;
          }
          if (!X->empty())
          {
            Result.push_back(*X);
          }
          const auto Owners = childRecords((*D)[2]);
          if (!Owners)
          {
            return false;
          }
          for (auto It = Owners->rbegin(); It != Owners->rend(); ++It)
          {
            if (It->Tag == 'F' || It->Tag == 'c' || It->Tag == 'e' || It->Tag == 'j')
            {
              const auto Owner = childRecords(*It);
              return Owner && Owner->size() == 2 && frames((*Owner)[0], (*Owner)[1], Result, Depth + 1);
            }
          }
          return true;
        }

        std::optional<Record> substitute(const Record &Value, const Frames &Bindings, std::size_t Depth, bool Count = false)
        {
          if (!enter(Depth))
          {
            return std::nullopt;
          }
          if (Value.Tag == 'g')
          {
            const auto C = childRecords(Value);
            std::uint64_t Level = 0;
            std::uint64_t Index = 0;
            if (!C || C->size() != 2 || !number((*C)[0].Payload, Level) || !number((*C)[1].Payload, Index) || Level >= Bindings.size() || Index >= Bindings[static_cast<std::size_t>(Level)].size())
            {
              return std::nullopt;
            }
            const auto &Argument = Bindings[static_cast<std::size_t>(Level)][static_cast<std::size_t>(Index)];
            const auto A = childRecords(Argument);
            if (!A || A->empty())
            {
              return std::nullopt;
            }
            if (!Count)
            {
              return Argument.Tag == 'T' && A->size() == 1 ? std::optional<Record>(A->front()) : std::nullopt;
            }
            std::uint64_t Width = 0;
            std::uint64_t Length = 0;
            if (Argument.Tag != 'V' || A->size() != 2 || (A->front().Tag != 'i' && A->front().Tag != 'u') || !number(A->front().Payload, Width) || !Width || (*A)[1].Tag != 'B')
            {
              return std::nullopt;
            }
            std::string_view Bits = (*A)[1].Payload;
            if (Bits.empty() || (A->front().Tag == 'i' && (hex(Bits.front()) & (1 << ((Width - 1) % 4)))))
            {
              return std::nullopt;
            }
            while (Bits.size() > 1 && Bits.front() == '0')
            {
              Bits.remove_prefix(1);
            }
            const auto Parsed = std::from_chars(Bits.data(), Bits.data() + Bits.size(), Length, 16);
            if (Parsed.ec != std::errc{} || Parsed.ptr != Bits.data() + Bits.size())
            {
              return std::nullopt;
            }
            return Record{'D', std::to_string(Length)};
          }
          // Nominal instances have already been checked in their own declaration scope.
          if (std::string_view("vbiufmDcej").find(Value.Tag) != std::string_view::npos || (Value.Tag == 'A' && Value.Payload == "n"))
          {
            return Value;
          }
          const bool Prefix = Value.Tag == 'p' || Value.Tag == 'r' || Value.Tag == 's' || Value.Tag == 'A';
          const auto Children = childRecords(Value, Prefix ? 1 : 0);
          if (!Children)
          {
            return std::nullopt;
          }
          std::vector<Record> Replaced;
          for (std::size_t Index = 0; Index < Children->size(); ++Index)
          {
            const auto Child = substitute((*Children)[Index], Bindings, Depth + 1, Value.Tag == 'a' && Index == 0);
            if (!Child)
            {
              return std::nullopt;
            }
            Replaced.push_back(*Child);
          }
          Record Result = record(Value.Tag, Replaced);
          if (Prefix)
          {
            Result.Payload.insert(Result.Payload.begin(), Value.Payload.front());
          }
          return Result;
        }

        std::optional<Record> closedPattern(const Record &Declaration, const Record &Instance, std::size_t Depth)
        {
          Frames Bindings;
          if (!frames(Declaration, Instance, Bindings, Depth + 1))
          {
            return std::nullopt;
          }
          const auto D = *childRecords(Declaration);
          const auto G = *childRecords(D[5]);
          const auto X = *childRecords(Instance);
          if (G.size() != X.size())
          {
            return std::nullopt;
          }
          for (std::size_t Index = 0; Index < G.size(); ++Index)
          {
            if (G[Index].Tag == 'V')
            {
              const auto Schema = childRecords(G[Index]);
              const auto Argument = childRecords(X[Index]);
              const auto Expected = Schema && Schema->size() == 1 ? substitute(Schema->front(), Bindings, Depth + 1) : std::nullopt;
              if (!Expected || !Argument || Argument->empty() || *Expected != Argument->front())
              {
                return std::nullopt;
              }
            }
          }
          return substitute(D[6], Bindings, Depth + 1);
        }

        bool argument(const Record &Value, const Record &Schema, std::size_t Depth)
        {
          const auto C = childRecords(Value);
          if (!enter(Depth) || Value.Tag != Schema.Tag || !C)
          {
            return false;
          }
          if (Value.Tag == 'T')
          {
            return C->size() == 1 && type((*C)[0], Depth + 1);
          }
          if (Value.Tag == 'V')
          {
            return C->size() == 2 && type((*C)[0], Depth + 1) && constant((*C)[0], (*C)[1], Depth + 1);
          }
          const auto P = childRecords(Schema);
          return Value.Tag == 'B' && P && P->size() == 1 && std::all_of(C->begin(), C->end(), [&](const Record &Child)
          {
            return argument(Child, (*P)[0], Depth + 1);
          });
        }

        bool constant(const Record &Type, const Record &Data, std::size_t Depth)
        {
          if (!enter(Depth))
          {
            return false;
          }
          if (Data.Tag == 'Z')
          {
            return Type.Tag == 'p' && Data.Payload.empty();
          }
          if (Data.Tag == 'B')
          {
            if (!std::all_of(Data.Payload.begin(), Data.Payload.end(), [](char C)
            {
              return hex(C) >= 0;
            }))
            {
              return false;
            }
            if (Type.Tag == 'b')
            {
              return Data.Payload == "0" || Data.Payload == "1";
            }
            std::uint64_t Width = 0;
            if ((Type.Tag == 'i' || Type.Tag == 'u' || Type.Tag == 'f') && number(Type.Payload, Width))
            {
              return Data.Payload.size() == (Width + 3) / 4 && (Width % 4 == 0 || hex(Data.Payload.front()) < (1 << (Width % 4)));
            }
            const auto C = childRecords(Type, Type.Tag == 's' ? 1 : 0);
            if (!C || (Type.Tag != 'a' && Type.Tag != 's') || C->empty() || C->back() != Record{'u', "8"} || Data.Payload.size() % 2 != 0)
            {
              return false;
            }
            std::uint64_t Count = 0;
            return Type.Tag == 's' || (C->size() == 2 && number((*C)[0].Payload, Count) && Count == Data.Payload.size() / 2);
          }
          if (Type.Tag == 'a' && Data.Tag == 'A')
          {
            const auto T = childRecords(Type);
            const auto C = childRecords(Data);
            std::uint64_t Count = 0;
            if (!T || T->size() != 2 || !C || !number((*T)[0].Payload, Count) || Count != C->size())
            {
              return false;
            }
            for (const auto &Element : *C)
            {
              const auto E = childRecords(Element);
              if (Element.Tag != 'V' || !E || E->size() != 2 || (*E)[0] != (*T)[1] || !constant((*E)[0], (*E)[1], Depth + 1))
              {
                return false;
              }
            }
            return true;
          }
          // Class/enum constants need a semantic field/underlying-type schema, not host bytes.
          return false;
        }

        bool types(const Record &Value, std::size_t Depth, bool Open = false)
        {
          const auto C = childRecords(Value);
          return enter(Depth) && Value.Tag == 'L' && C && std::all_of(C->begin(), C->end(), [&](const Record &Child)
          {
            return Child.Tag != 'v' && type(Child, Depth + 1, Open);
          });
        }

        bool receiver(const Record &Value, std::size_t Depth, bool Open = false)
        {
          if (Value.Tag != 'A' || Value.Payload.empty())
          {
            return false;
          }
          if (Value.Payload == "n")
          {
            return true;
          }
          const auto C = childRecords(Value, 1);
          return std::string_view("rwv").find(Value.Payload.front()) != std::string_view::npos && C && C->size() == 1 && ((*C)[0].Tag == 'c' || (*C)[0].Tag == 'e' || (*C)[0].Tag == 'j') && type((*C)[0], Depth + 1, Open);
        }

        bool signature(const Record &Value, std::size_t Depth)
        {
          const auto C = childRecords(Value);
          if (!enter(Depth) || Value.Tag != 'S' || !C || C->size() != 5)
          {
            return false;
          }
          const auto Q = childRecords((*C)[4]);
          return atom((*C)[0], 'B', "ic") && atom((*C)[1], 'C', "cfk") && receiver((*C)[2], Depth + 1) && types((*C)[3], Depth + 1) && (*C)[4].Tag == 'Q' && Q && Q->size() == 1 && type((*Q)[0], Depth + 1);
        }

        bool type(const Record &Value, std::size_t Depth, bool Open = false)
        {
          if (!enter(Depth))
          {
            return false;
          }
          if (Value.Tag == 'v' || Value.Tag == 'b')
          {
            return Value.Payload.empty();
          }
          std::uint64_t Width = 0;
          if (Value.Tag == 'i' || Value.Tag == 'u')
          {
            return number(Value.Payload, Width) && Width > 0 && Width <= std::numeric_limits<std::uint32_t>::max();
          }
          if (Value.Tag == 'f')
          {
            return Value.Payload == "16" || Value.Payload == "32" || Value.Payload == "64";
          }
          if (Value.Tag == 'm')
          {
            return Open && Value.Payload.empty();
          }
          if (Value.Tag == 'p' || Value.Tag == 'r' || Value.Tag == 's')
          {
            const auto C = childRecords(Value, 1);
            return !Value.Payload.empty() && (Value.Payload.front() == 'r' || Value.Payload.front() == 'w') && C && C->size() == 1 && ((*C)[0].Tag != 'v' || Value.Tag == 'p') && type((*C)[0], Depth + 1, Open);
          }
          const auto C = childRecords(Value);
          if (!C)
          {
            return false;
          }
          if (Value.Tag == 'g')
          {
            return Open && C->size() == 2 && (*C)[0].Tag == 'D' && (*C)[1].Tag == 'D' && number((*C)[0].Payload, Width) && number((*C)[1].Payload, Width);
          }
          if (Value.Tag == 'a')
          {
            return C->size() == 2 && (((*C)[0].Tag == 'D' && number((*C)[0].Payload, Width)) || (Open && (*C)[0].Tag == 'g' && type((*C)[0], Depth + 1, true))) && (*C)[1].Tag != 'v' && type((*C)[1], Depth + 1, Open);
          }
          if (Value.Tag == 'q')
          {
            const auto Q = C->size() == 2 ? childRecords((*C)[1]) : std::nullopt;
            return C->size() == 2 && types((*C)[0], Depth + 1, Open) && (*C)[1].Tag == 'Q' && Q && Q->size() == 1 && type((*Q)[0], Depth + 1, Open);
          }
          return (Value.Tag == 'c' || Value.Tag == 'e' || Value.Tag == 'j') && C->size() == 2 && declaration((*C)[0], Value.Tag == 'j' ? 'i' : Value.Tag, Depth + 1) && instance((*C)[1], (*C)[0], Depth + 1);
        }

        ManglingLimits Limits;
        std::size_t Records = 0;
    };
  } // namespace

  MangleResult mangle(const Record &Identity, ManglingLimits Limits)
  {
    if (Identity.Payload.size() > Limits.MaxBytes || !Validator(Limits).symbol(Identity))
    {
      return {{}, "invalid or unsupported Ink symbol identity, or mangling budget exceeded"};
    }
    std::string Name = std::string(SymbolPrefix) + encodeRecord(Identity);
    if (Name.size() > Limits.MaxBytes)
    {
      return {{}, "Ink symbol exceeds the configured byte limit"};
    }
    return {std::move(Name), {}};
  }

  DemangleResult demangle(std::string_view Symbol, ManglingLimits Limits)
  {
    if (!Symbol.starts_with(SymbolPrefix))
    {
      return {std::nullopt, "unsupported Ink symbol prefix or version", 0};
    }
    if (Symbol.size() > Limits.MaxBytes)
    {
      return {std::nullopt, "Ink symbol exceeds the configured byte limit", 0};
    }
    std::size_t Offset = SymbolPrefix.size();
    Record Identity;
    if (!readRecord(Symbol, Offset, Identity) || Offset != Symbol.size())
    {
      return {std::nullopt, "invalid record length, spelling or trailing data", Offset};
    }
    if (!Validator(Limits).symbol(Identity))
    {
      return {std::nullopt, "invalid or unsupported Ink symbol schema, or mangling budget exceeded", SymbolPrefix.size()};
    }
    return {std::move(Identity), {}, 0};
  }
} // namespace ink::abi
