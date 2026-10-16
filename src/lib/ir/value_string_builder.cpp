#include "value_string_builder.h"

#include <utility>

namespace ink::ir
{
  std::string ValueStringBuilder::format(const Value &ValueObject)
  {
    ValueStringBuilder Builder;
    Builder.pushValue(ValueObject);
    while (!Builder.Pending.empty())
    {
      FormatPart Part = std::move(Builder.Pending.back());
      Builder.Pending.pop_back();
      if (Part.ValueObject)
      {
        Part.ValueObject->appendToString(Builder);
      }
      else
      {
        Builder.append(Part.Text);
      }
    }
    return std::move(Builder.Result);
  }

  void ValueStringBuilder::append(std::string_view Text)
  {
    Result += Text;
  }

  void ValueStringBuilder::pushValue(const Value &ValueObject)
  {
    Pending.push_back({&ValueObject, {}});
  }

  void ValueStringBuilder::pushText(std::string Text)
  {
    Pending.push_back({nullptr, std::move(Text)});
  }
} // namespace ink::ir
