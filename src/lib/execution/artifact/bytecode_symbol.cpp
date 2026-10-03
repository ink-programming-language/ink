#include "bytecode_internal.h"

#include <algorithm>
#include <bit>
#include <limits>

namespace ink::execution
{
  namespace
  {
    void appendField(std::string &Text, std::string_view Field)
    {
      Text += std::to_string(Field.size());
      Text += ':';
      Text += Field;
    }

    bool appendIdentityField(std::string &Text, std::string_view Field, std::size_t Maximum)
    {
      const std::string Length = std::to_string(Field.size());
      if (Text.size() > Maximum || Length.size() + 1 > Maximum - Text.size() || Field.size() > Maximum - Text.size() - Length.size() - 1)
      {
        return false;
      }
      Text += Length;
      Text += ':';
      Text += Field;
      return true;
    }

    std::size_t childCount(const StorageLayout &Layout)
    {
      return Layout.Kind == RuntimeKind::Pointer || Layout.Kind == RuntimeKind::Array ? 1 : Layout.Kind == RuntimeKind::Function ? Layout.Parameters.size() + 1 : 0;
    }

    RuntimeTypeId childAt(const StorageLayout &Layout, std::size_t Index)
    {
      return Layout.Kind == RuntimeKind::Array ? Layout.ElementType : Layout.Kind == RuntimeKind::Pointer ? Layout.Pointee : Index == 0 ? Layout.ReturnType : Layout.Parameters[Index - 1];
    }

    std::string layoutPrefix(const StorageLayout &Layout)
    {
      // These identity tags are versioned independently of RuntimeKind's enum order.
      unsigned Kind = 0;
      switch (Layout.Kind)
      {
      case RuntimeKind::Void:
        Kind = 1;
        break;
      case RuntimeKind::Boolean:
        Kind = 2;
        break;
      case RuntimeKind::Integer:
        Kind = 3;
        break;
      case RuntimeKind::Float:
        Kind = 4;
        break;
      case RuntimeKind::String:
        Kind = 5;
        break;
      case RuntimeKind::Pointer:
        Kind = 6;
        break;
      case RuntimeKind::Function:
        Kind = 7;
        break;
      case RuntimeKind::Array:
        Kind = 8;
        break;
      case RuntimeKind::Invalid:
        break;
      }
      return "t(" + std::to_string(Kind) + "," + std::to_string(Layout.BitWidth) + "," + std::to_string(Layout.Signed) + "," + std::to_string(Layout.Size) + "," + std::to_string(Layout.Alignment) + "," + std::to_string(Layout.Native) + "," + std::to_string(Layout.Writable) + "," + std::to_string(childCount(Layout)) + (Layout.Kind == RuntimeKind::Array ? "," + std::to_string(Layout.ElementCount) : "") + ")";
    }
  } // namespace

  std::string nativeBytecodeTarget()
  {
    std::string Result;
#if defined(_WIN32)
    Result = "windows";
#elif defined(__APPLE__)
    Result = "darwin";
#elif defined(__linux__)
    Result = "linux";
#else
    return {};
#endif
#if defined(_M_ARM64EC)
    Result += "-aarch64ec";
#elif defined(_M_X64) || defined(__x86_64__)
    Result += "-x86_64";
#elif defined(_M_IX86) || defined(__i386__)
    Result += "-x86";
#elif defined(_M_ARM64) || defined(__aarch64__)
    Result += "-aarch64";
#elif defined(_M_ARM) || defined(__arm__)
    Result += "-arm";
#if defined(__ARM_PCS_VFP) || (defined(_M_ARM) && defined(_WIN32))
    Result += "-float-hard";
#elif defined(__SOFTFP__) || defined(__ARM_PCS)
    Result += "-float-soft";
#else
    return {};
#endif
#elif defined(__riscv)
#if defined(__riscv_xlen)
    Result += "-riscv";
    Result += std::to_string(__riscv_xlen);
#else
    return {};
#endif
#if defined(__riscv_float_abi_quad)
    Result += "-float-quad";
#elif defined(__riscv_float_abi_double)
    Result += "-float-double";
#elif defined(__riscv_float_abi_single)
    Result += "-float-single";
#elif defined(__riscv_float_abi_soft)
    Result += "-float-soft";
#else
    return {};
#endif
#else
    return {};
#endif
    Result += std::endian::native == std::endian::little ? "-le" : std::endian::native == std::endian::big ? "-be" : "-mixed";
    Result += "-p" + std::to_string(sizeof(void *) * 8);
    return Result;
  }

  std::string bytecodeTypeIdentity(const RuntimeTypeTable &Types, RuntimeTypeId Type, std::size_t MaxDepth)
  {
    if (!Types.get(Type))
    {
      return {};
    }
    BytecodeLimits Limits;
    Limits.MaxTypeDepth = MaxDepth;
    std::vector<std::string> Identities;
    return artifact_detail::typeIdentities(Types, Limits, Identities) ? std::move(Identities[Type]) : std::string{};
  }

  std::string bytecodeSymbolKey(const BytecodeSymbolIdentity &Identity)
  {
    std::string Result;
    appendField(Result, Identity.Module);
    appendField(Result, Identity.Name);
    appendField(Result, Identity.Signature);
    Result += std::to_string(Identity.GenericArguments.size()) + ':';
    for (const BytecodeGenericArgument &Argument : Identity.GenericArguments)
    {
      Result += Argument.Kind == BytecodeGenericArgumentKind::Type ? 'T' : 'C';
      appendField(Result, Argument.Type);
      appendField(Result, Argument.Value);
    }
    return Result;
  }

  namespace artifact_detail
  {
    bool accountSymbolKey(const BytecodeSymbolIdentity &Identity, Budget &Usage)
    {
      constexpr std::size_t Prefix = std::numeric_limits<std::size_t>::digits10 + 2;
      if (!Usage.allocation(Prefix * 4) || !Usage.allocation(Identity.Module.size()) || !Usage.allocation(Identity.Name.size()) || !Usage.allocation(Identity.Signature.size()))
      {
        return false;
      }
      for (const BytecodeGenericArgument &Argument : Identity.GenericArguments)
      {
        if (!Usage.allocation(Prefix * 2 + 1) || !Usage.allocation(Argument.Type.size()) || !Usage.allocation(Argument.Value.size()))
        {
          return false;
        }
      }
      return true;
    }

    BytecodeResult typeIdentities(const RuntimeTypeTable &Types, BytecodeLimits Limits, std::vector<std::string> &Identities, Budget *SharedUsage)
    {
      Budget LocalUsage(Limits);
      Budget &Usage = SharedUsage ? *SharedUsage : LocalUsage;
      if (Types.size() >= InvalidRuntimeType || Types.size() > Limits.MaxRecords || !Usage.allocation(Types.size(), sizeof(std::string) + sizeof(std::size_t) * 3 + sizeof(std::uint8_t)) || Limits.MaxTypeDepth == 0)
      {
        return {BytecodeStatus::LimitExceeded, "Bytecode type table exceeds the configured limits"};
      }
      Identities.resize(Types.size());
      std::vector<std::uint8_t> States(Types.size());
      std::vector<std::size_t> Depths(Types.size());
      struct Visit
      {
          RuntimeTypeId Type;
          std::size_t Child = 0;
      };
      std::vector<Visit> Stack;
      std::size_t IdentityBytes = 0;
      for (std::size_t Root = 0; Root < Types.size(); ++Root)
      {
        if (States[Root] == 2)
        {
          continue;
        }
        States[Root] = 1;
        Stack.push_back({static_cast<RuntimeTypeId>(Root)});
        while (!Stack.empty())
        {
          Visit &Current = Stack.back();
          const StorageLayout &Layout = *Types.get(Current.Type);
          if (Layout.Kind <= RuntimeKind::Invalid || Layout.Kind > RuntimeKind::Array || Layout.Parameters.size() >= Limits.MaxRecords)
          {
            return {BytecodeStatus::InvalidImage, "Bytecode type graph contains an unsupported kind or invalid parameter count"};
          }
          if (Current.Child < childCount(Layout))
          {
            const RuntimeTypeId Child = childAt(Layout, Current.Child++);
            if (!Types.get(Child) || States[Child] == 1)
            {
              return {BytecodeStatus::InvalidImage, "Bytecode type graph contains a missing type or a cycle"};
            }
            if (States[Child] == 0)
            {
              if (Stack.size() >= Limits.MaxTypeDepth)
              {
                return {BytecodeStatus::LimitExceeded, "Bytecode type graph exceeds the nesting limit"};
              }
              States[Child] = 1;
              Stack.push_back({Child});
            }
            continue;
          }
          std::string Identity = layoutPrefix(Layout);
          if (!Usage.allocation(Identity.size() * 2))
          {
            return {BytecodeStatus::LimitExceeded, "Bytecode type identity exceeds the allocation limit"};
          }
          std::size_t Depth = 1;
          for (std::size_t Index = 0; Index < childCount(Layout); ++Index)
          {
            const RuntimeTypeId Child = childAt(Layout, Index);
            if (Depths[Child] >= Limits.MaxTypeDepth || !Usage.allocation(Identities[Child].size(), 2) || !Usage.allocation((std::numeric_limits<std::size_t>::digits10 + 2) * 2) || !appendIdentityField(Identity, Identities[Child], Limits.MaxStringBytes))
            {
              return {BytecodeStatus::LimitExceeded, "Bytecode type identity exceeds the nesting or string limit"};
            }
            Depth = std::max(Depth, Depths[Child] + 1);
          }
          if (IdentityBytes > Limits.MaxStringBytes || Identity.size() > Limits.MaxStringBytes - IdentityBytes || IdentityBytes > Limits.MaxAllocationBytes || Identity.size() > Limits.MaxAllocationBytes - IdentityBytes)
          {
            return {BytecodeStatus::LimitExceeded, "Bytecode type identities exceed the configured storage limit"};
          }
          IdentityBytes += Identity.size();
          Identities[Current.Type] = std::move(Identity);
          Depths[Current.Type] = Depth;
          States[Current.Type] = 2;
          Stack.pop_back();
        }
      }
      return {};
    }
  } // namespace artifact_detail
} // namespace ink::execution
