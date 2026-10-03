#include "ink/execution/artifact/bytecode_archive.h"

#include <fstream>
#include <limits>
#include <utility>

namespace ink::execution
{
  BytecodeResult writeBytecodeFile(const std::filesystem::path &Path, const BytecodeArtifact &Artifact, BytecodeLimits Limits)
  {
    BytecodeSerializeResult Serialized = serializeBytecodeArtifact(Artifact, Limits);
    if (!Serialized)
    {
      return {Serialized.Status, std::move(Serialized.Message)};
    }
    if (Serialized.Bytes.size() > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
    {
      return {BytecodeStatus::LimitExceeded, "Bytecode file exceeds the stream size limit"};
    }
    std::ofstream Output(Path, std::ios::binary | std::ios::trunc);
    if (!Output)
    {
      return {BytecodeStatus::IoError, "Cannot open bytecode output file"};
    }
    Output.write(Serialized.Bytes.data(), static_cast<std::streamsize>(Serialized.Bytes.size()));
    Output.flush();
    if (!Output)
    {
      return {BytecodeStatus::IoError, "Cannot write bytecode output file"};
    }
    Output.close();
    if (!Output)
    {
      return {BytecodeStatus::IoError, "Cannot close bytecode output file"};
    }
    return {};
  }

  BytecodeArtifactResult readBytecodeFile(const std::filesystem::path &Path, BytecodeLimits Limits)
  {
    std::ifstream Input(Path, std::ios::binary | std::ios::ate);
    if (!Input)
    {
      return {BytecodeStatus::IoError, "Cannot open bytecode input file", nullptr};
    }
    const std::streamoff Length = Input.tellg();
    if (Length < 0)
    {
      return {BytecodeStatus::IoError, "Cannot determine bytecode input size", nullptr};
    }
    const auto Size = static_cast<std::uintmax_t>(Length);
    if (Size > Limits.MaxBytes || Size > Limits.MaxAllocationBytes || Size > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
    {
      return {BytecodeStatus::LimitExceeded, "Bytecode input exceeds its byte or allocation budget", nullptr};
    }
    std::string Bytes;
    if (Size > Bytes.max_size())
    {
      return {BytecodeStatus::LimitExceeded, "Bytecode input exceeds the string size limit", nullptr};
    }
    Bytes.resize(static_cast<std::size_t>(Size));
    Input.seekg(0, std::ios::beg);
    if (!Input)
    {
      return {BytecodeStatus::IoError, "Cannot seek bytecode input file", nullptr};
    }
    Input.read(Bytes.data(), static_cast<std::streamsize>(Size));
    if (!Input || Input.gcount() != static_cast<std::streamsize>(Size))
    {
      return {BytecodeStatus::IoError, "Cannot read complete bytecode input file", nullptr};
    }
    if (Input.peek() != std::char_traits<char>::eof() || Input.bad())
    {
      return {BytecodeStatus::IoError, "Bytecode input changed while it was being read", nullptr};
    }
    Limits.MaxAllocationBytes -= static_cast<std::size_t>(Size);
    return deserializeBytecodeArtifact(Bytes, Limits);
  }
} // namespace ink::execution
