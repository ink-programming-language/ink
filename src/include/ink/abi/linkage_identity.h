#ifndef INK_ABI_LINKAGE_IDENTITY_H
#define INK_ABI_LINKAGE_IDENTITY_H

#include <string>
#include <utility>
#include <vector>

namespace ink::abi
{
  // Resolved build identity, independent of source paths and optimization settings.
  // The default is local to one link unit; separately distributed packages must supply an identity.
  struct PackageIdentity
  {
      std::string Authority = "local";
      std::vector<std::string> Name{"anonymous"};
      std::string Revision = "0";
      std::vector<std::pair<std::string, std::string>> Variant;

      bool operator==(const PackageIdentity &) const = default;
  };

  struct ModuleIdentity
  {
      PackageIdentity Package;
      std::vector<std::string> Path;

      bool operator==(const ModuleIdentity &) const = default;
  };

  // A wire record, not a second semantic type system. Type records are produced from IR types.
  struct Record
  {
      char Tag = 0;
      std::string Payload;

      bool operator==(const Record &) const = default;
  };
} // namespace ink::abi

#endif
