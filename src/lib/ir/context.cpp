#include "ink/ir/context.h"

#include <cassert>
#include <utility>

namespace ink::ir
{
  IRContext::IRContext(core::CompilationContext &Compilation)
      : Compilation(Compilation)
  {
#define INK_IR_TYPE(Name, Spelling) PrimitiveTypes[static_cast<std::size_t>(TypeKind::Name)] = std::unique_ptr<Type>(new Type(TypeKind::Name));
#include "ink/ir/ir.def"
  }

  IRContext::~IRContext() = default;

  const Type &IRContext::getType(TypeKind Kind) const noexcept
  {
    const std::size_t Index = static_cast<std::size_t>(Kind);
    assert(Index < PrimitiveTypes.size());
    assert(PrimitiveTypes[Index]);
    return *PrimitiveTypes[Index];
  }

  const StructType &IRContext::createStructType(std::string Name, std::vector<const Type *> FieldTypes)
  {
    auto Result = std::unique_ptr<StructType>(new StructType(std::move(Name), std::move(FieldTypes)));
    const StructType &Reference = *Result;
    StructTypes.push_back(std::move(Result));
    return Reference;
  }
} // namespace ink::ir
