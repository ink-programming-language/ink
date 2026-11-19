#include "ink/ir/module/module_path.h"

#include "ink/core/name/name_pool.h"

#include <string>

namespace ink::ir
{
    namespace
    {
        std::size_t combineHash(std::size_t First, std::size_t Second) noexcept
        {
            return First ^ (Second + std::size_t{0x9e3779b9} + (First << 6) + (First >> 2));
        }
    } // namespace

    ModulePath::ModulePath(std::span<const core::Name> Components)
        : Components(Components.begin(), Components.end())
    {
        if (this->Components.empty())
        {
            return;
        }
        core::NamePool& Names = core::NamePool::instance();
        std::string Joined;
        for (std::size_t Index = 0; Index < this->Components.size(); ++Index)
        {
            const std::string_view Spelling = Names.text(this->Components[Index]);
            if (Spelling.empty() || Spelling.find('.') != std::string_view::npos)
            {
                return;
            }
            if (Index != 0)
            {
                Joined.push_back('.');
            }
            Joined.append(Spelling);
        }
        QualifiedName = Names.intern(Joined);
    }

    ModulePath::ModulePath(std::initializer_list<core::Name> Components)
        : ModulePath(std::span<const core::Name>(Components.begin(), Components.size()))
    {
    }

    ModulePath ModulePath::parse(std::string_view QualifiedText)
    {
        if (QualifiedText.empty())
        {
            return {};
        }
        core::NamePool& Names = core::NamePool::instance();
        std::vector<core::Name> ParsedComponents;
        std::size_t Start = 0;
        while (Start <= QualifiedText.size())
        {
            const std::size_t Dot = QualifiedText.find('.', Start);
            const std::size_t End = Dot == std::string_view::npos ? QualifiedText.size() : Dot;
            const std::string_view Segment = QualifiedText.substr(Start, End - Start);
            if (Segment.empty())
            {
                return {};
            }
            const core::Name Interned = Names.intern(Segment);
            if (!Interned.valid())
            {
                return {};
            }
            ParsedComponents.push_back(Interned);
            if (Dot == std::string_view::npos)
            {
                break;
            }
            Start = Dot + 1;
        }
        return ModulePath(ParsedComponents);
    }

    core::Name ModulePath::qualifiedSymbolName(core::Name SymbolName) const
    {
        if (!valid())
        {
            return core::NAME_None;
        }
        core::NamePool& Names = core::NamePool::instance();
        const std::string_view ModuleSpelling = Names.text(QualifiedName);
        const std::string_view SymbolSpelling = Names.text(SymbolName);
        if (ModuleSpelling.empty() || SymbolSpelling.empty())
        {
            return core::NAME_None;
        }
        std::string Combined;
        Combined.reserve(ModuleSpelling.size() + 2 + SymbolSpelling.size());
        Combined.append(ModuleSpelling);
        Combined.append("::");
        Combined.append(SymbolSpelling);
        return Names.intern(Combined);
    }

    std::size_t ModulePath::hash() const noexcept
    {
        std::size_t Value = combineHash(std::hash<core::Name>{}(QualifiedName), std::hash<std::size_t>{}(Components.size()));
        for (const core::Name Component : Components)
        {
            Value = combineHash(Value, std::hash<core::Name>{}(Component));
        }
        return Value;
    }
} // namespace ink::ir
