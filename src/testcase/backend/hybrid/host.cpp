#include "ink/execution/hybrid/runtime.h"
#include "ink/execution/hybrid/archive.h"

#include <spdlog/spdlog.h>

#include <string>
#include <thread>

extern "C" const InkHybridModule *ink_hybrid_module();
extern "C" int32_t hybridValue(int32_t Value);
extern "C" int32_t hybridLifecycle();
extern "C" int32_t hybridArray();
extern "C" uint32_t hybridBusy();

namespace
{
  bool require(bool Condition, const char *Message)
  {
    if (!Condition)
    {
      spdlog::error("{}: {}", Message, ink_hybrid_last_error());
    }
    return Condition;
  }

} // namespace

int main(int ArgumentCount, char **Arguments)
{
  if (ArgumentCount != 2)
  {
    return 1;
  }
  const auto *Module = ink_hybrid_module();
  const std::string Directory = Arguments[1];
  if (!require(ink_hybrid_register(Module) == InkHybridSuccess, "registration") || !require(hybridValue(2) == 23 && hybridLifecycle() == 9 && hybridArray() == 42, "native bodies") || !require(hybridBusy() == InkHybridBusy, "active AOT patch removal"))
  {
    return 2;
  }
  if (!require(ink_hybrid_apply((Directory + "/first.patch").c_str()) == InkHybridSuccess, "first patch") || !require(hybridValue(2) == 122, "VM to AOT to VM recursion") || !require(hybridLifecycle() == 10 && hybridArray() == 43, "aggregate copying and destruction") || !require(hybridBusy() == InkHybridBusy, "VM calls statically linked native import"))
  {
    return 3;
  }
  bool OtherThreadNative = false;
  std::thread Other([&]()
  {
    OtherThreadNative = hybridValue(2) == 23;
  });
  Other.join();
  if (!require(OtherThreadNative, "thread local patch ownership") || !require(ink_hybrid_apply((Directory + "/bounce.patch").c_str()) == InkHybridSuccess && hybridValue(2) == 142, "existing VM caller resolves new target") || !require(ink_hybrid_apply((Directory + "/second.patch").c_str()) == InkHybridSuccess && hybridValue(2) == 242, "replace active patch image"))
  {
    return 6;
  }
  for (int Index = 0; Index < 100; ++Index)
  {
    if (!require(hybridValue(4) == 284, "repeat hosted VM invocation"))
    {
      return 7;
    }
  }
  auto WrongBuild = ink::execution::readHybridFile(std::filesystem::path(Directory) / "second.patch");
  if (!require(static_cast<bool>(WrongBuild), "read rejection fixture"))
  {
    return 9;
  }
  auto WrongBytes = ink::execution::serializeHybridArtifact(ink::execution::HybridArtifactKind::Patch, std::string(64, '0'), *WrongBuild.Artifact.Bytecode);
  if (!require(static_cast<bool>(WrongBytes) && ink_hybrid_apply_bytes(WrongBytes.Bytes.data(), WrongBytes.Bytes.size()) == InkHybridIncompatible && hybridValue(2) == 242, "wrong native build preserves active patch"))
  {
    return 10;
  }
  auto &Object = *WrongBuild.Artifact.Bytecode;
  std::vector<ink::execution::TypeDesc> Layouts;
  for (size_t Index = 0; Index < Object.Image.Layouts->size(); ++Index)
  {
    auto Type = *Object.Image.Layouts->get(static_cast<ink::execution::RuntimeTypeId>(Index));
    if (Type.Kind == ink::execution::RuntimeKind::Class && !Type.classDesc().Fields.empty())
    {
      Type.editClass().Fields.front().Name = "ChangedField";
    }
    Layouts.push_back(std::move(Type));
  }
  auto ChangedTypes = std::make_shared<ink::execution::RuntimeTypeTable>();
  if (!require(ChangedTypes->defineAll(std::move(Layouts)), "build incompatible type fixture"))
  {
    return 11;
  }
  Object.Image.Layouts = ChangedTypes;
  for (auto &[Id, Body] : Object.Image.Functions)
  {
    Body->Layouts = ChangedTypes;
  }
  auto WrongTypes = ink::execution::serializeHybridArtifact(ink::execution::HybridArtifactKind::Patch, WrongBuild.Artifact.BuildId, Object);
  if (!require(static_cast<bool>(WrongTypes) && ink_hybrid_apply_bytes(WrongTypes.Bytes.data(), WrongTypes.Bytes.size()) == InkHybridIncompatible && hybridValue(2) == 242, "incompatible type preserves active patch"))
  {
    return 12;
  }
  if (!require(ink_hybrid_apply_bytes("broken", 6) == InkHybridInvalidArtifact && hybridValue(2) == 242, "invalid patch preserves current version") || !require(ink_hybrid_clear() == InkHybridSuccess && hybridValue(2) == 23 && hybridLifecycle() == 9, "restore original native bodies"))
  {
    return 8;
  }
  return 0;
}
