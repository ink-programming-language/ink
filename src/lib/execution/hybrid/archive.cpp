#include "ink/execution/hybrid/archive.h"

#include <llvm/Support/SHA256.h>
#include <llvm/ADT/ArrayRef.h>

#include <algorithm>
#include <fstream>
#include <unordered_map>

namespace ink::execution
{
  std::string hybridBuildId(std::string_view Bytes)
  {
    const auto Digest = llvm::SHA256::hash(llvm::ArrayRef<std::uint8_t>(reinterpret_cast<const std::uint8_t *>(Bytes.data()), Bytes.size()));
    constexpr char Hex[] = "0123456789abcdef";
    std::string Result;
    for (const auto Byte : Digest)
    {
      Result += Hex[Byte >> 4];
      Result += Hex[Byte & 15];
    }
    return Result;
  }

  BytecodeSerializeResult serializeHybridArtifact(HybridArtifactKind Kind, std::string_view BuildId, const BytecodeArtifact &Artifact)
  {
    if (BuildId.size() != 64 || !std::all_of(BuildId.begin(), BuildId.end(), [](char Value)
    {
      return (Value >= '0' && Value <= '9') || (Value >= 'a' && Value <= 'f');
    }))
    {
      return {BytecodeStatus::InvalidInput, "Invalid hybrid build identity", {}};
    }
    auto Result = serializeBytecodeArtifact(Artifact);
    if (Result)
    {
      Result.Bytes.insert(0, std::string("INKHYB01") + (Kind == HybridArtifactKind::Base ? "B" : "P") + std::string(BuildId));
    }
    return Result;
  }

  HybridArtifactResult deserializeHybridArtifact(std::string_view Bytes)
  {
    if (Bytes.size() < 73 || Bytes.size() > BytecodeLimits{}.MaxBytes + 73 || !Bytes.starts_with("INKHYB01") || (Bytes[8] != 'B' && Bytes[8] != 'P'))
    {
      return {"Invalid hybrid artifact header"};
    }
    const auto Id = Bytes.substr(9, 64);
    if (!std::all_of(Id.begin(), Id.end(), [](char Value)
    {
      return (Value >= '0' && Value <= '9') || (Value >= 'a' && Value <= 'f');
    }))
    {
      return {"Invalid hybrid build identity"};
    }
    auto Read = deserializeBytecodeArtifact(Bytes.substr(73));
    if (!Read)
    {
      return {Read.Message};
    }
    if (Read.Artifact->Kind != BytecodeArtifactKind::Object || (Bytes[8] == 'B' && !Read.Artifact->Image.Functions.empty()))
    {
      return {"Hybrid artifacts require relocatable objects; base manifests cannot contain bytecode bodies"};
    }
    return {{}, {Bytes[8] == 'B' ? HybridArtifactKind::Base : HybridArtifactKind::Patch, std::string(Id), std::move(Read.Artifact)}};
  }

  HybridArtifactResult readHybridFile(const std::filesystem::path &Path)
  {
    std::ifstream Input(Path, std::ios::binary | std::ios::ate);
    if (!Input)
    {
      return {"Cannot open hybrid artifact"};
    }
    const auto Size = Input.tellg();
    if (Size < 0 || static_cast<std::uint64_t>(Size) > BytecodeLimits{}.MaxBytes + 73)
    {
      return {"Hybrid artifact exceeds its byte limit"};
    }
    std::string Bytes(static_cast<std::size_t>(Size), '\0');
    Input.seekg(0);
    if (!Input.read(Bytes.data(), static_cast<std::streamsize>(Bytes.size())))
    {
      return {"Cannot read hybrid artifact"};
    }
    return deserializeHybridArtifact(Bytes);
  }

  BytecodeResult writeHybridFile(const std::filesystem::path &Path, std::string_view Bytes)
  {
    std::ofstream Output(Path, std::ios::binary | std::ios::trunc);
    if (!Output || !Output.write(Bytes.data(), static_cast<std::streamsize>(Bytes.size())))
    {
      return {BytecodeStatus::IoError, "Cannot write hybrid artifact"};
    }
    Output.flush();
    return Output ? BytecodeResult{} : BytecodeResult{BytecodeStatus::IoError, "Cannot flush hybrid artifact"};
  }

  BytecodeResult compatibleHybridTypes(const RuntimeTypeTable &Base, const RuntimeTypeTable &Patch)
  {
    std::unordered_map<std::string, const TypeDesc *> Classes;
    for (std::size_t Index = 0; Index < Base.size(); ++Index)
    {
      const auto &Type = *Base.get(static_cast<RuntimeTypeId>(Index));
      if (Type.Kind == RuntimeKind::Class)
      {
        Classes.emplace(Type.classDesc().NominalIdentity, &Type);
      }
    }
    for (std::size_t Index = 0; Index < Patch.size(); ++Index)
    {
      const auto &Type = *Patch.get(static_cast<RuntimeTypeId>(Index));
      if (Type.Kind != RuntimeKind::Class)
      {
        continue;
      }
      const auto Found = Classes.find(Type.classDesc().NominalIdentity);
      if (Found == Classes.end())
      {
        continue;
      }
      const auto &Original = *Found->second;
      const auto &Left = Original.classDesc();
      const auto &Right = Type.classDesc();
      const auto Mismatch = [&]() -> BytecodeResult
      {
        return {BytecodeStatus::SignatureMismatch, "Hot reload changes the layout or interface of " + Type.Name};
      };
      if (Original.Size != Type.Size || Original.Alignment != Type.Alignment || Left.Fields.size() != Right.Fields.size() || Left.Methods.size() != Right.Methods.size())
      {
        return Mismatch();
      }
      for (std::size_t Field = 0; Field < Left.Fields.size(); ++Field)
      {
        const auto &A = Left.Fields[Field];
        const auto &B = Right.Fields[Field];
        if (A.Name != B.Name || A.Offset != B.Offset || A.Visibility != B.Visibility || bytecodeTypeIdentity(Base, A.Type) != bytecodeTypeIdentity(Patch, B.Type))
        {
          return Mismatch();
        }
      }
      for (std::size_t Method = 0; Method < Left.Methods.size(); ++Method)
      {
        const auto &A = Left.Methods[Method];
        const auto &B = Right.Methods[Method];
        if (A.Name != B.Name || A.Visibility != B.Visibility || A.WritableReceiver != B.WritableReceiver || bytecodeTypeIdentity(Base, A.Signature) != bytecodeTypeIdentity(Patch, B.Signature))
        {
          return Mismatch();
        }
      }
    }
    return {};
  }
} // namespace ink::execution
