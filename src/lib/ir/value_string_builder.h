#ifndef INK_IR_VALUE_STRING_BUILDER_H
#define INK_IR_VALUE_STRING_BUILDER_H

#include "ink/ir/type/value.h"

#include <string>
#include <string_view>
#include <vector>

namespace ink::ir
{
  // Internal formatting state shared by composite values; queued parts are consumed in reverse order.
  class ValueStringBuilder
  {
    public:
      static std::string format(const Value &ValueObject);

      void append(std::string_view Text);
      void pushValue(const Value &ValueObject);
      void pushText(std::string Text);

    private:
      struct FormatPart
      {
          const Value *ValueObject = nullptr;
          std::string Text;
      };

      std::string Result;
      std::vector<FormatPart> Pending;
  };
} // namespace ink::ir

#endif
