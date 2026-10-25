#include "load_transaction.h"

#include "ink/ir/context.h"

#include <unordered_set>

namespace ink::ir::detail
{
  namespace
  {
    template <typename MapT, typename SetT>
    void eraseNewValues(MapT &Map, const SetT &NewValues)
    {
      std::erase_if(Map, [&NewValues](const auto &Entry)
                    {
                      return NewValues.contains(Entry.second);
                    });
    }

    template <typename MapT, typename SetT>
    void eraseNewNestedValues(MapT &Map, const SetT &NewValues)
    {
      for (auto Iterator = Map.begin(); Iterator != Map.end();)
      {
        eraseNewValues(Iterator->second, NewValues);
        if (Iterator->second.empty())
        {
          Iterator = Map.erase(Iterator);
        }
        else
        {
          ++Iterator;
        }
      }
    }
  } // namespace

  IRLoadTransaction::IRLoadTransaction(IRContext &Context) noexcept
      : Context(Context),
        TypeCount(Context.typePool().Storage.size()),
        ConstantCount(Context.constantPool().Storage.size())
  {
  }

  IRLoadTransaction::~IRLoadTransaction() noexcept
  {
    if (Committed)
    {
      return;
    }

    auto &Constants = Context.constantPool();
    if (Constants.Storage.size() > ConstantCount)
    {
      std::unordered_set<const Constant *> NewConstants;
      NewConstants.reserve(Constants.Storage.size() - ConstantCount);
      for (std::size_t Index = ConstantCount; Index < Constants.Storage.size(); ++Index)
      {
        NewConstants.insert(Constants.Storage[Index].get());
      }
      eraseNewNestedValues(Constants.IntegerConstants, NewConstants);
      eraseNewNestedValues(Constants.FloatConstants, NewConstants);
      eraseNewValues(Constants.ArrayConstants, NewConstants);
      // String cache keys borrow storage bytes, so erase them before destruction.
      eraseNewValues(Constants.StringConstants, NewConstants);
      for (const Constant *Value : NewConstants)
      {
        Constants.OwnedConstants.erase(Value);
      }
      Constants.Storage.resize(ConstantCount);
    }

    auto &Types = Context.typePool();
    if (Types.Storage.size() > TypeCount)
    {
      std::unordered_set<const Type *> NewTypes;
      NewTypes.reserve(Types.Storage.size() - TypeCount);
      for (std::size_t Index = TypeCount; Index < Types.Storage.size(); ++Index)
      {
        NewTypes.insert(Types.Storage[Index].get());
      }
      eraseNewValues(Types.IntegerTypes, NewTypes);
      eraseNewValues(Types.FloatTypes, NewTypes);
      eraseNewValues(Types.PointerTypes, NewTypes);
      eraseNewValues(Types.ReferenceTypes, NewTypes);
      eraseNewValues(Types.ArrayTypes, NewTypes);
      eraseNewValues(Types.FunctionTypes, NewTypes);
      eraseNewValues(Types.ConstNominalTypes, NewTypes);
      for (const Type *Value : NewTypes)
      {
        Types.OwnedTypes.erase(Value);
      }
      Types.Storage.resize(TypeCount);
    }
  }

  void IRLoadTransaction::commit() noexcept
  {
    Committed = true;
  }
} // namespace ink::ir::detail
