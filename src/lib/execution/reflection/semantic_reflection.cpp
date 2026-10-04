#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/class_type.h"

namespace ink::execution
{
  void SemanticValueBridge::retainReflectionTypes()
  {
    std::vector<const ir::Module *> Modules;
    for (const auto &Module : Context.modules())
    {
      Modules.push_back(Module.get());
    }
    for (std::size_t Index = 0; Index < Modules.size(); ++Index)
    {
      for (const auto *Class : Modules[Index]->classTypes())
      {
        if (Class->isComplete())
        {
          lowerType(*Class);
        }
      }
      for (const auto &Value : Modules[Index]->entryBlock().values())
      {
        if (ir::Module::classof(Value.get()))
        {
          Modules.push_back(static_cast<const ir::Module *>(Value.get()));
        }
      }
    }
    synchronizeReflection();
  }

  void SemanticValueBridge::synchronizeReflection()
  {
    if (Reflecting || (ReflectionRevision == Context.revision() && ReflectionTypeCount == SourceTypes.size()))
    {
      return;
    }
    Reflecting = true;
    for (std::size_t Index = 0; Index < SourceTypes.size(); ++Index)
    {
      const auto *Source = SourceTypes[Index];
      const auto *Layout = Types->get(static_cast<RuntimeTypeId>(Index));
      if (!Source || !ir::ClassType::classof(Source) || !Layout || Layout->Kind != RuntimeKind::Class)
      {
        continue;
      }
      const auto &Class = static_cast<const ir::ClassType &>(*Source);
      auto Description = std::make_shared<ClassDesc>(Layout->classDesc());
      for (std::size_t Field = 0; Field < Class.fields().size(); ++Field)
      {
        Description->Fields[Field].Initializer = InvalidFunction;
        if (const auto *Initializer = Class.fields()[Field].Initializer)
        {
          Description->Fields[Field].Initializer = lowerFunction(*Initializer);
        }
      }
      Description->Methods.clear();
      for (const ir::Function *Method : Class.methods())
      {
        const FunctionId Function = lowerFunction(*Method);
        const auto Name = Context.namePool().text(Method->name());
        const auto Position = Name.rfind('.');
        const auto &Receiver = static_cast<const ir::PointerType &>(*Method->functionType().parameterTypes().front());
        Description->Methods.push_back({std::string(Position == std::string_view::npos ? Name : Name.substr(Position + 1)), lowerType(Method->type()), Function, Method->visibility() == ir::VisibilityKind::Private ? MemberVisibility::Private : MemberVisibility::Public, Receiver.access() == ir::AccessKind::ReadWrite});
      }
      Types->updateClass(static_cast<RuntimeTypeId>(Index), std::move(Description));
    }
    ReflectionRevision = Context.revision();
    ReflectionTypeCount = SourceTypes.size();
    Reflecting = false;
  }
} // namespace ink::execution
