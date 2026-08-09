#include "module_loader.h"

#include "ink/core/source_file_id.h"
#include "ink/tokenizer/tokenizer.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::tools::inkc
{
  namespace
  {
    struct ImportDeclaration
    {
      std::vector<std::string> Components;
      std::string Alias;
      std::size_t Start = 0;
      std::size_t End = 0;
    };

    struct FunctionDeclaration
    {
      std::string Name;
      std::string LoweredName;
      bool Exported = true;
      bool External = false;
    };

    struct Replacement
    {
      std::size_t Start = 0;
      std::size_t End = 0;
      std::string Text;
      unsigned Priority = 0;
    };

    struct ScannedSource
    {
      tokenizer::TokenizedBuffer Tokens;
      std::vector<std::size_t> Significant;
      std::vector<unsigned> Depths;
      std::vector<ImportDeclaration> Imports;
      std::vector<FunctionDeclaration> Functions;
    };

    enum class FunctionExportKind
    {
      InkFunction,
      StandardOutput,
      UnsupportedStandardInput,
    };

    struct FunctionExport
    {
      std::string LoweredName;
      FunctionExportKind Kind = FunctionExportKind::InkFunction;
    };

    using FunctionExports = std::map<std::string, FunctionExport>;

    struct ModuleRecord
    {
      enum class State
      {
        Loading,
        Loaded,
      };

      State LoadingState = State::Loading;
      std::filesystem::path Path;
      std::string TransformedSource;
      FunctionExports Exports;
      std::vector<StaticOutputCall> StaticOutputCalls;
    };

    struct TransformResult
    {
      std::string Source;
      FunctionExports Exports;
      std::vector<StaticOutputCall> StaticOutputCalls;
    };

    bool readFile(const std::filesystem::path &Path, std::string &Contents)
    {
      std::ifstream Input(Path, std::ios::binary);
      if (!Input)
      {
        return false;
      }
      Contents.assign(std::istreambuf_iterator<char>(Input), std::istreambuf_iterator<char>());
      return !Input.bad();
    }

    bool isKeyword(const tokenizer::Token &Token, tokenizer::KeywordKind Kind)
    {
      const auto *Keyword = std::get_if<tokenizer::KeywordKind>(&Token.Payload);
      return Token.Kind == tokenizer::TokenKind::Keyword && Keyword != nullptr && *Keyword == Kind;
    }

    bool isSymbol(const tokenizer::TokenizedBuffer &Tokens, const tokenizer::Token &Token, std::string_view Symbol)
    {
      return Token.Kind == tokenizer::TokenKind::Symbol && Tokens.raw(Token) == Symbol;
    }

    bool isIdentifier(const tokenizer::TokenizedBuffer &Tokens, const tokenizer::Token &Token, std::string_view Name)
    {
      return Token.Kind == tokenizer::TokenKind::Identifier && Tokens.raw(Token) == Name;
    }

    std::string pathKey(const std::filesystem::path &Path)
    {
      std::error_code Error;
      const std::filesystem::path Canonical = std::filesystem::weakly_canonical(Path, Error);
      return (Error ? Path.lexically_normal() : Canonical).generic_u8string();
    }

    std::string modulePrefix(const std::filesystem::path &Path)
    {
      const std::string Key = pathKey(Path);
      std::uint64_t Hash = 1469598103934665603ULL;
      for (const unsigned char Byte : Key)
      {
        Hash ^= Byte;
        Hash *= 1099511628211ULL;
      }
      std::ostringstream Stream;
      Stream << "ink_module_" << std::hex << std::setw(16) << std::setfill('0') << Hash << '_';
      return Stream.str();
    }

    std::optional<ScannedSource> scanSource(const std::filesystem::path &Path, std::string Source, bool MangleFunctions, ModuleLoadError &Error)
    {
      ScannedSource Result{tokenizer::tokenize(core::SourceFileId::fromValue(0), std::move(Source))};
      if (!Result.Tokens.succeeded())
      {
        Error = {Path, "source contains lexical errors while scanning module imports", true};
        return std::nullopt;
      }

      const std::vector<tokenizer::Token> &AllTokens = Result.Tokens.tokens();
      unsigned Depth = 0;
      for (std::size_t Index = 0; Index < AllTokens.size(); ++Index)
      {
        const tokenizer::Token &Token = AllTokens[Index];
        if (Token.isTrivia() || Token.Kind == tokenizer::TokenKind::Utf8Bom || Token.Kind == tokenizer::TokenKind::EndOfFile)
        {
          continue;
        }
        Result.Significant.push_back(Index);
        Result.Depths.push_back(Depth);
        if (isSymbol(Result.Tokens, Token, "{"))
        {
          ++Depth;
        }
        else if (isSymbol(Result.Tokens, Token, "}"))
        {
          if (Depth == 0)
          {
            Error = {Path, "unbalanced closing brace while scanning module imports", true};
            return std::nullopt;
          }
          --Depth;
        }
      }

      for (std::size_t Position = 0; Position < Result.Significant.size(); ++Position)
      {
        const tokenizer::Token &Token = AllTokens[Result.Significant[Position]];
        if (Result.Depths[Position] != 0 || !isKeyword(Token, tokenizer::KeywordKind::Import))
        {
          continue;
        }
        ImportDeclaration Import;
        Import.Start = Token.Span.Start;
        std::size_t Cursor = Position + 1;
        bool ExpectComponent = true;
        while (Cursor < Result.Significant.size())
        {
          const tokenizer::Token &Current = AllTokens[Result.Significant[Cursor]];
          if (isKeyword(Current, tokenizer::KeywordKind::As) || isSymbol(Result.Tokens, Current, ";"))
          {
            break;
          }
          if (ExpectComponent)
          {
            if (Current.Kind != tokenizer::TokenKind::Identifier && Current.Kind != tokenizer::TokenKind::BuiltinType)
            {
              Error = {Path, "inkc currently requires module imports of the form 'import path.to.module as name;'", true};
              return std::nullopt;
            }
            Import.Components.emplace_back(Result.Tokens.raw(Current));
          }
          else if (!isSymbol(Result.Tokens, Current, "."))
          {
            Error = {Path, "invalid module path in import declaration", true};
            return std::nullopt;
          }
          ExpectComponent = !ExpectComponent;
          ++Cursor;
        }
        if (Import.Components.empty() || ExpectComponent || Cursor >= Result.Significant.size())
        {
          Error = {Path, "invalid module path in import declaration", true};
          return std::nullopt;
        }
        const tokenizer::Token &Separator = AllTokens[Result.Significant[Cursor]];
        if (isKeyword(Separator, tokenizer::KeywordKind::As))
        {
          ++Cursor;
          if (Cursor >= Result.Significant.size() || AllTokens[Result.Significant[Cursor]].Kind != tokenizer::TokenKind::Identifier)
          {
            Error = {Path, "module import alias must be an identifier", true};
            return std::nullopt;
          }
          Import.Alias = std::string(Result.Tokens.raw(AllTokens[Result.Significant[Cursor]]));
          ++Cursor;
        }
        else
        {
          Import.Alias = Import.Components.back();
        }
        if (Cursor >= Result.Significant.size() || !isSymbol(Result.Tokens, AllTokens[Result.Significant[Cursor]], ";"))
        {
          Error = {Path, "module import declaration must end with ';'", true};
          return std::nullopt;
        }
        Import.End = AllTokens[Result.Significant[Cursor]].Span.End;
        Result.Imports.push_back(std::move(Import));
        Position = Cursor;
      }

      const std::string Prefix = modulePrefix(Path);
      for (std::size_t Position = 0; Position < Result.Significant.size(); ++Position)
      {
        const tokenizer::Token &Token = AllTokens[Result.Significant[Position]];
        if (Result.Depths[Position] != 0 || !isKeyword(Token, tokenizer::KeywordKind::Func))
        {
          continue;
        }
        if (Position + 1 >= Result.Significant.size())
        {
          continue;
        }
        const tokenizer::Token &NameToken = AllTokens[Result.Significant[Position + 1]];
        if (NameToken.Kind != tokenizer::TokenKind::Identifier)
        {
          continue;
        }
        FunctionDeclaration Function;
        Function.Name = std::string(Result.Tokens.raw(NameToken));
        for (std::size_t Back = Position; Back > 0; --Back)
        {
          const tokenizer::Token &Previous = AllTokens[Result.Significant[Back - 1]];
          if (isSymbol(Result.Tokens, Previous, ";") || isSymbol(Result.Tokens, Previous, "{") || isSymbol(Result.Tokens, Previous, "}"))
          {
            break;
          }
          if (isKeyword(Previous, tokenizer::KeywordKind::Private) || isKeyword(Previous, tokenizer::KeywordKind::Protected))
          {
            Function.Exported = false;
          }
          else if (isKeyword(Previous, tokenizer::KeywordKind::Public))
          {
            Function.Exported = true;
          }
          else if (isKeyword(Previous, tokenizer::KeywordKind::Extern))
          {
            Function.External = true;
          }
        }
        Function.LoweredName = MangleFunctions && !Function.External ? Prefix + Function.Name : Function.Name;
        Result.Functions.push_back(std::move(Function));
      }
      return Result;
    }

    std::string blank(std::string_view Text)
    {
      std::string Result(Text);
      for (char &Character : Result)
      {
        if (Character != '\r' && Character != '\n')
        {
          Character = ' ';
        }
      }
      return Result;
    }

    bool overlaps(const Replacement &Left, const Replacement &Right)
    {
      return Left.Start < Right.End && Right.Start < Left.End;
    }

    void addReplacement(std::vector<Replacement> &Replacements, Replacement Candidate)
    {
      for (auto Iterator = Replacements.begin(); Iterator != Replacements.end();)
      {
        if (!overlaps(*Iterator, Candidate))
        {
          ++Iterator;
          continue;
        }
        if (Iterator->Priority >= Candidate.Priority)
        {
          return;
        }
        Iterator = Replacements.erase(Iterator);
      }
      Replacements.push_back(std::move(Candidate));
    }

    std::string applyReplacements(const std::string &Source, std::vector<Replacement> Replacements)
    {
      std::sort(Replacements.begin(), Replacements.end(), [](const Replacement &Left, const Replacement &Right)
      {
        return Left.Start < Right.Start;
      });
      std::string Result;
      std::size_t Cursor = 0;
      for (const Replacement &ReplacementValue : Replacements)
      {
        Result.append(Source, Cursor, ReplacementValue.Start - Cursor);
        Result += ReplacementValue.Text;
        Cursor = ReplacementValue.End;
      }
      Result.append(Source, Cursor, std::string::npos);
      return Result;
    }

    class LoaderImpl
    {
    public:
      explicit LoaderImpl(std::filesystem::path StandardLibraryRoot) : StandardLibraryRoot(std::move(StandardLibraryRoot))
      {
      }

      ModuleLoadResult load(const std::filesystem::path &EntryPath)
      {
        std::string EntrySource;
        if (!readFile(EntryPath, EntrySource))
        {
          return failure(EntryPath, "cannot read entry source file");
        }
        ModuleLoadError ScanError;
        std::optional<ScannedSource> Scanned = scanSource(EntryPath, EntrySource, false, ScanError);
        if (!Scanned)
        {
          return failure(std::move(ScanError));
        }
        std::map<std::string, FunctionExports> Imports;
        if (!loadImports(EntryPath, Scanned->Imports, Imports))
        {
          return failure(*Error);
        }
        std::optional<TransformResult> Entry = transform(EntryPath, EntrySource, *Scanned, Imports);
        if (!Entry)
        {
          return failure(*Error);
        }

        LoadedProgram Program;
        for (const std::string &Key : ModuleOrder)
        {
          const ModuleRecord &Module = Modules.at(Key);
          const std::size_t Start = Program.Source.size();
          Program.Source += Module.TransformedSource;
          Program.Source += '\n';
          Program.Sections.push_back({Module.Path, Start, Program.Source.size()});
          Program.Dependencies.push_back(Module.Path);
          Program.StaticOutputCalls.insert(Program.StaticOutputCalls.end(), Module.StaticOutputCalls.begin(), Module.StaticOutputCalls.end());
        }
        const std::size_t EntryStart = Program.Source.size();
        Program.Source += Entry->Source;
        Program.Sections.push_back({EntryPath, EntryStart, Program.Source.size()});
        Program.StaticOutputCalls.insert(Program.StaticOutputCalls.end(), Entry->StaticOutputCalls.begin(), Entry->StaticOutputCalls.end());
        return {std::move(Program), std::nullopt};
      }

    private:
      ModuleLoadResult failure(ModuleLoadError LoadError)
      {
        return {std::nullopt, std::move(LoadError)};
      }

      ModuleLoadResult failure(const std::filesystem::path &Path, std::string Message)
      {
        return failure(ModuleLoadError{Path, std::move(Message)});
      }

      std::filesystem::path resolveImport(const std::filesystem::path &Importer, const ImportDeclaration &Import) const
      {
        std::filesystem::path Result;
        std::size_t Begin = 0;
        if (!Import.Components.empty() && Import.Components.front() == "std")
        {
          Result = StandardLibraryRoot;
          Begin = 1;
        }
        else
        {
          Result = Importer.parent_path();
        }
        for (std::size_t Index = Begin; Index < Import.Components.size(); ++Index)
        {
          Result /= std::filesystem::u8path(Import.Components[Index]);
        }
        Result += ".ink";
        return Result;
      }

      bool loadImports(const std::filesystem::path &Importer, const std::vector<ImportDeclaration> &Declarations, std::map<std::string, FunctionExports> &Imports)
      {
        for (const ImportDeclaration &Import : Declarations)
        {
          if (Imports.find(Import.Alias) != Imports.end())
          {
            Error = ModuleLoadError{Importer, "duplicate module import alias '" + Import.Alias + "'", true};
            return false;
          }
          const std::filesystem::path Resolved = resolveImport(Importer, Import);
          ModuleRecord *Module = loadModule(Resolved);
          if (Module == nullptr)
          {
            return false;
          }
          Imports.emplace(Import.Alias, Module->Exports);
        }
        return true;
      }

      ModuleRecord *loadModule(const std::filesystem::path &Path)
      {
        const std::string Key = pathKey(Path);
        const auto Existing = Modules.find(Key);
        if (Existing != Modules.end())
        {
          if (Existing->second.LoadingState == ModuleRecord::State::Loading)
          {
            Error = ModuleLoadError{Path, "module import cycle detected", true};
            return nullptr;
          }
          return &Existing->second;
        }

        ModuleRecord &Record = Modules[Key];
        Record.Path = Path;
        std::string Source;
        if (!readFile(Path, Source))
        {
          Error = ModuleLoadError{Path, "cannot read imported module"};
          Modules.erase(Key);
          return nullptr;
        }
        ModuleLoadError ScanError;
        std::optional<ScannedSource> Scanned = scanSource(Path, Source, true, ScanError);
        if (!Scanned)
        {
          Error = std::move(ScanError);
          Modules.erase(Key);
          return nullptr;
        }
        if (Key == pathKey(StandardLibraryRoot / "io.ink"))
        {
          for (const FunctionDeclaration &Function : Scanned->Functions)
          {
            if (!Function.Exported)
            {
              continue;
            }
            if (Function.Name == "output")
            {
              Record.Exports.emplace(Function.Name, FunctionExport{"", FunctionExportKind::StandardOutput});
            }
            else if (Function.Name == "input")
            {
              Record.Exports.emplace(Function.Name, FunctionExport{"", FunctionExportKind::UnsupportedStandardInput});
            }
          }
          Record.LoadingState = ModuleRecord::State::Loaded;
          ModuleOrder.push_back(Key);
          return &Record;
        }
        std::map<std::string, FunctionExports> Imports;
        if (!loadImports(Path, Scanned->Imports, Imports))
        {
          Modules.erase(Key);
          return nullptr;
        }
        std::optional<TransformResult> Transformed = transform(Path, Source, *Scanned, Imports);
        if (!Transformed)
        {
          Modules.erase(Key);
          return nullptr;
        }
        Record.TransformedSource = std::move(Transformed->Source);
        Record.Exports = std::move(Transformed->Exports);
        Record.StaticOutputCalls = std::move(Transformed->StaticOutputCalls);
        Record.LoadingState = ModuleRecord::State::Loaded;
        ModuleOrder.push_back(Key);
        return &Record;
      }

      std::optional<TransformResult> transform(const std::filesystem::path &Path, const std::string &Source, const ScannedSource &Scanned, const std::map<std::string, FunctionExports> &Imports)
      {
        std::vector<Replacement> Replacements;
        std::string RuntimeDeclarations;
        std::vector<StaticOutputCall> StaticOutputCalls;
        for (const ImportDeclaration &Import : Scanned.Imports)
        {
          addReplacement(Replacements, {Import.Start, Import.End, blank(std::string_view(Source).substr(Import.Start, Import.End - Import.Start)), 100});
        }

        const std::vector<tokenizer::Token> &Tokens = Scanned.Tokens.tokens();
        for (std::size_t Position = 0; Position + 2 < Scanned.Significant.size(); ++Position)
        {
          const tokenizer::Token &AliasToken = Tokens[Scanned.Significant[Position]];
          const tokenizer::Token &DotToken = Tokens[Scanned.Significant[Position + 1]];
          const tokenizer::Token &MemberToken = Tokens[Scanned.Significant[Position + 2]];
          if (AliasToken.Kind != tokenizer::TokenKind::Identifier || !isSymbol(Scanned.Tokens, DotToken, ".") || MemberToken.Kind != tokenizer::TokenKind::Identifier)
          {
            continue;
          }
          const std::string Alias(Scanned.Tokens.raw(AliasToken));
          const auto Import = Imports.find(Alias);
          if (Import == Imports.end())
          {
            continue;
          }
          if (Position + 3 >= Scanned.Significant.size() || !isSymbol(Scanned.Tokens, Tokens[Scanned.Significant[Position + 3]], "("))
          {
            continue;
          }
          const std::string Member(Scanned.Tokens.raw(MemberToken));
          const auto Export = Import->second.find(Member);
          if (Export == Import->second.end())
          {
            Error = ModuleLoadError{Path, "module alias '" + Alias + "' has no public function named '" + Member + "'", true};
            return std::nullopt;
          }
          if (Export->second.Kind == FunctionExportKind::UnsupportedStandardInput)
          {
            Error = ModuleLoadError{Path, "std.io.input is not supported by the current inkc language slice", true};
            return std::nullopt;
          }
          if (Export->second.Kind == FunctionExportKind::StandardOutput)
          {
            if (Position + 5 >= Scanned.Significant.size())
            {
              Error = ModuleLoadError{Path, "std.io.output requires exactly one string literal argument", true};
              return std::nullopt;
            }
            const tokenizer::Token &ArgumentToken = Tokens[Scanned.Significant[Position + 4]];
            const tokenizer::Token &CloseToken = Tokens[Scanned.Significant[Position + 5]];
            const tokenizer::StringInfo *String = std::get_if<tokenizer::StringInfo>(&ArgumentToken.Payload);
            if (ArgumentToken.Kind != tokenizer::TokenKind::StringLiteral || String == nullptr || !isSymbol(Scanned.Tokens, CloseToken, ")"))
            {
              Error = ModuleLoadError{Path, "std.io.output requires exactly one string literal argument", true};
              return std::nullopt;
            }
            const std::string FunctionName = "ink_compiler_runtime_stdout_" + std::to_string(NextStaticOutputCall++);
            addReplacement(Replacements, {AliasToken.Span.Start, CloseToken.Span.End, FunctionName + "()", 95});
            RuntimeDeclarations += "func " + FunctionName + "() -> i32;\n";
            StaticOutputCalls.push_back({FunctionName, String->Decoded});
            Position += 5;
            continue;
          }
          addReplacement(Replacements, {AliasToken.Span.Start, MemberToken.Span.End, Export->second.LoweredName, 90});
        }

        for (std::size_t Position = 0; Position < Scanned.Significant.size(); ++Position)
        {
          if (Scanned.Depths[Position] != 0)
          {
            continue;
          }
          const tokenizer::Token &Token = Tokens[Scanned.Significant[Position]];
          if (isKeyword(Token, tokenizer::KeywordKind::Public) || isKeyword(Token, tokenizer::KeywordKind::Protected) || isKeyword(Token, tokenizer::KeywordKind::Private))
          {
            addReplacement(Replacements, {Token.Span.Start, Token.Span.End, blank(Scanned.Tokens.raw(Token)), 70});
          }
          else if (isKeyword(Token, tokenizer::KeywordKind::Extern))
          {
            std::size_t End = Token.Span.End;
            if (Position + 1 < Scanned.Significant.size() && Tokens[Scanned.Significant[Position + 1]].Kind == tokenizer::TokenKind::StringLiteral)
            {
              End = Tokens[Scanned.Significant[Position + 1]].Span.End;
            }
            addReplacement(Replacements, {Token.Span.Start, End, blank(std::string_view(Source).substr(Token.Span.Start, End - Token.Span.Start)), 70});
          }
        }

        TransformResult Result;
        for (const FunctionDeclaration &Function : Scanned.Functions)
        {
          if (Function.Exported)
          {
            Result.Exports.emplace(Function.Name, FunctionExport{Function.LoweredName, FunctionExportKind::InkFunction});
          }
          if (Function.Name == Function.LoweredName)
          {
            continue;
          }
          for (std::size_t Position = 0; Position < Scanned.Significant.size(); ++Position)
          {
            const tokenizer::Token &Token = Tokens[Scanned.Significant[Position]];
            if (!isIdentifier(Scanned.Tokens, Token, Function.Name))
            {
              continue;
            }
            const bool DeclarationName = Position > 0 && isKeyword(Tokens[Scanned.Significant[Position - 1]], tokenizer::KeywordKind::Func);
            const bool DirectCall = Position + 1 < Scanned.Significant.size() && isSymbol(Scanned.Tokens, Tokens[Scanned.Significant[Position + 1]], "(");
            const bool GroupedCall = Position > 0 && Position + 2 < Scanned.Significant.size() && isSymbol(Scanned.Tokens, Tokens[Scanned.Significant[Position - 1]], "(") && isSymbol(Scanned.Tokens, Tokens[Scanned.Significant[Position + 1]], ")") && isSymbol(Scanned.Tokens, Tokens[Scanned.Significant[Position + 2]], "(");
            if (DeclarationName || DirectCall || GroupedCall)
            {
              addReplacement(Replacements, {Token.Span.Start, Token.Span.End, Function.LoweredName, 50});
            }
          }
        }
        Result.Source = applyReplacements(Source, std::move(Replacements));
        if (!RuntimeDeclarations.empty())
        {
          Result.Source += '\n';
          Result.Source += RuntimeDeclarations;
        }
        Result.StaticOutputCalls = std::move(StaticOutputCalls);
        return Result;
      }

      std::filesystem::path StandardLibraryRoot;
      std::map<std::string, ModuleRecord> Modules;
      std::vector<std::string> ModuleOrder;
      std::optional<ModuleLoadError> Error;
      std::uint64_t NextStaticOutputCall = 0;
    };
  } // namespace

  const SourceSection *LoadedProgram::sectionAt(std::size_t Offset) const noexcept
  {
    for (const SourceSection &Section : Sections)
    {
      if (Offset >= Section.Start && Offset < Section.End)
      {
        return &Section;
      }
    }
    return nullptr;
  }

  bool ModuleLoadResult::succeeded() const noexcept
  {
    return Program.has_value() && !Error.has_value();
  }

  ModuleLoader::ModuleLoader(std::filesystem::path StandardLibraryRoot) : StandardLibraryRoot(std::move(StandardLibraryRoot))
  {
  }

  ModuleLoadResult ModuleLoader::load(const std::filesystem::path &EntryPath)
  {
    return LoaderImpl(StandardLibraryRoot).load(EntryPath);
  }
} // namespace ink::tools::inkc
