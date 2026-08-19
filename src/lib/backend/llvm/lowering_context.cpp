#include "lowering_context.h"

#include <utility>

namespace ink::backend::llvm
{
  LoweringContext::LoweringContext(::llvm::LLVMContext &Context, const ir::Module &SourceModule)
      : Context(Context),
        SourceModule(SourceModule),
        TargetModule(std::make_unique<::llvm::Module>(moduleName(), Context))
  {
    TargetModule->setDataLayout(dataLayoutString());
  }

  bool LoweringContext::lower()
  {
    return declareStructTypes() && lowerByteConstants() && lowerGlobals() && declareFunctions() && lowerFunctions() && lowerLifecycleFunctions();
  }

  std::unique_ptr<::llvm::Module> LoweringContext::takeModule() noexcept
  {
    return std::move(TargetModule);
  }

  std::string LoweringContext::valueName(ir::ValueId Value) const
  {
    return "v" + std::to_string(Value.value());
  }

  std::string LoweringContext::moduleName() const
  {
    return SourceModule.Name.has_value() ? SourceModule.Name->str() : "ink.module";
  }

  std::string LoweringContext::dataLayoutString() const
  {
    const core::TargetContext &Target = SourceModule.context().compilationContext().targetContext();
    const char Endianness = Target.byteOrder() == core::ByteOrder::LittleEndian ? 'e' : 'E';
    const std::size_t PointerBits = static_cast<std::size_t>(Target.pointerWidth());
    return std::string(1, Endianness) + "-p:" + std::to_string(PointerBits) + ":" + std::to_string(PointerBits) + "-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f16:16:16-f32:32:32-f64:64:64";
  }
} // namespace ink::backend::llvm
