#include "ink/semantic/module_import.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  namespace
  {
    bool validComponent(std::string_view Component)
    {
      if (Component.empty())
      {
        return false;
      }
      for (unsigned char Character : Component)
      {
        if (Character <= 0x20 || Character == '.' || Character == '/' || Character == '\\' || Character == ':' || Character == '#' || Character == 0x7f)
        {
          return false;
        }
      }
      return true;
    }
  } // namespace

  std::string resolveImportModuleName(std::string_view Importer, std::span<const parser::NameToken> Path, std::size_t RelativeLevel)
  {
    std::string Result;
    if (RelativeLevel)
    {
      std::size_t ComponentStart = 0;
      while (ComponentStart <= Importer.size())
      {
        const std::size_t Separator = Importer.find('.', ComponentStart);
        const std::size_t ComponentEnd = Separator == std::string_view::npos ? Importer.size() : Separator;
        if (!validComponent(Importer.substr(ComponentStart, ComponentEnd - ComponentStart)))
        {
          return {};
        }
        if (Separator == std::string_view::npos)
        {
          break;
        }
        ComponentStart = Separator + 1;
      }
      std::size_t End = Importer.size();
      for (std::size_t Level = 0; Level < RelativeLevel; ++Level)
      {
        if (End == 0)
        {
          return {};
        }
        const std::size_t Separator = Importer.rfind('.', End - 1);
        if (Separator == std::string_view::npos)
        {
          return {};
        }
        End = Separator;
      }
      Result = Importer.substr(0, End);
      std::size_t Begin = 0;
      while (Begin < Result.size())
      {
        const std::size_t Separator = Result.find('.', Begin);
        const std::size_t EndPart = Separator == std::string::npos ? Result.size() : Separator;
        if (!validComponent(std::string_view(Result).substr(Begin, EndPart - Begin)))
        {
          return {};
        }
        Begin = EndPart + 1;
      }
    }
    for (const parser::NameToken &Part : Path)
    {
      if (!validComponent(Part.Text))
      {
        return {};
      }
      if (!Result.empty())
      {
        Result += '.';
      }
      Result += Part.Text;
    }
    return Result;
  }
} // namespace ink::semantic
