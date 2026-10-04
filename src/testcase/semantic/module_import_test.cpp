#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/module_import.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    class ModuleAnalysis final
    {
      public:
        explicit ModuleAnalysis(std::initializer_list<std::pair<std::string_view, std::string_view>> Sources)
            : Frontend(Compilation),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
          for (const auto &[Name, Source] : Sources)
          {
            Names.emplace_back(Name);
            Parsed.push_back(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source))));
          }
        }

        bool parsed() const
        {
          return std::all_of(Parsed.begin(), Parsed.end(), [](const parser::ParseResult &Input)
          {
            return Input.succeeded();
          });
        }

        ir::Module *analyze(std::string_view Entry = "app")
        {
          std::vector<Analyzer::ModuleInput> Inputs;
          for (std::size_t Index = 0; Index < Parsed.size(); ++Index)
          {
            Inputs.push_back({Names[Index], &Parsed[Index]});
          }
          return Analyzer{}.analyzeModules(Context, Inputs, Entry);
        }

        ir::Module *module(std::string_view Name)
        {
          for (const auto &Module : Context.modules())
          {
            if (Context.namePool().text(Module->name()) == Name)
            {
              return Module.get();
            }
          }
          return nullptr;
        }

        const Binding<ir::Value *> *lookup(ir::Module &Module, std::string_view Name)
        {
          return NameResolver(Context).lookupMember(Module, Context.namePool().find(Name));
        }

        const ir::Function *function(ir::Module &Module, std::string_view Name)
        {
          const auto *Binding = lookup(Module, Name);
          return Binding && Binding->targets().size() == 1 && ir::Function::classof(Binding->targets().front()) ? static_cast<const ir::Function *>(Binding->targets().front()) : nullptr;
        }

        void expectResult(ir::Module &Module, std::uint64_t Expected)
        {
          const ir::Function *Entry = function(Module, "main");
          ASSERT_NE(Entry, nullptr);
          execution::ExecutionEngine Engine(Context.irContext());
          const auto Result = Engine.execute(*Entry);
          ASSERT_TRUE(Result);
          ASSERT_EQ(Result.Value.kind(), execution::ExecutionValueKind::Integer);
          EXPECT_EQ(Result.Value.integer().bits().words().front(), Expected);
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        std::vector<std::string> Names;
        std::vector<parser::ParseResult> Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };
  } // namespace

  // Function aliases retain their original module identity, visibility and actual IR definition.
  TEST(SemanticModuleImportTest, ImportsRealFunctionsWithAliases)
  {
    ModuleAnalysis Input({
        {"app", "from library import answer as read; func main(): i32 { return read(); }"},
        {"library", "func answer(): i32 { return 42; }"},
    });
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    ir::Module *Library = Input.module("library");
    ASSERT_NE(Library, nullptr);
    const ir::Function *Original = Input.function(*Library, "answer");
    ASSERT_NE(Original, nullptr);
    EXPECT_EQ(Original->visibility(), ir::VisibilityKind::Public);
    EXPECT_EQ(Input.function(*Entry, "read"), Original);
    EXPECT_EQ(Original->outer(), &Library->entryBlock());
    ASSERT_EQ(Input.Context.moduleImports(*Entry).size(), 1U);
    EXPECT_EQ(Input.Context.moduleImports(*Entry).front(), Original);
    EXPECT_TRUE(Input.Context.moduleImports(*Library).empty());
    EXPECT_EQ(Entry->entryBlock().values().size(), 1U);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectResult(*Entry, 42);
  }

  // Qualified calls support explicit aliases and the last path component as the default alias.
  TEST(SemanticModuleImportTest, ResolvesQualifiedModuleCalls)
  {
    ModuleAnalysis Input({
        {"app", "import package.left as L; import package.right; func main(): i32 { return L.value() + right.value(); }"},
        {"package.left", "public func value(): i32 { return 19; }"},
        {"package.right", "func value(): i32 { return 23; }"},
    });
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    EXPECT_EQ(Input.Context.moduleImports(*Entry).size(), 2U);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectResult(*Entry, 42);
  }

  // Top-level signatures are bound across the graph before mutually recursive bodies are analyzed.
  TEST(SemanticModuleImportTest, ResolvesMutualRecursionRegardlessOfInputOrder)
  {
    for (bool Reverse : {false, true})
    {
      SCOPED_TRACE(Reverse);
      const std::pair<std::string_view, std::string_view> App{"app", "from peer import next; func main(): i32 { return bounce(3); } func bounce(n: i32): i32 { if (n == 3) { return next(2); } return 42; }"};
      const std::pair<std::string_view, std::string_view> Peer{"peer", "from app import bounce; func next(n: i32): i32 { return bounce(n); }"};
      ModuleAnalysis Input(Reverse ? std::initializer_list{Peer, App} : std::initializer_list{App, Peer});
      ASSERT_TRUE(Input.parsed());
      ir::Module *Entry = Input.analyze();
      ASSERT_NE(Entry, nullptr);
      EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
      Input.expectResult(*Entry, 42);
    }
  }

  // Imported ordinary and comptime functions lower on demand, including transitive calls and module state.
  TEST(SemanticModuleImportTest, PreparesImportedComptimeDependenciesBeforeExecution)
  {
    for (bool Reverse : {false, true})
    {
      SCOPED_TRACE(Reverse);
      const std::pair<std::string_view, std::string_view> App{"app", "from adapter import answer; import provider as P; func main(): i32 { return comptime answer() + comptime P.folded(); }"};
      const std::pair<std::string_view, std::string_view> Adapter{"adapter", "from provider import seed; func answer(): i32 { return seed(); }"};
      const std::pair<std::string_view, std::string_view> Provider{"provider", "comptime var initial: i32 = 19; func seed(): i32 { return comptime initial; } comptime func folded(): i32 { return 23; }"};
      ModuleAnalysis Input(Reverse ? std::initializer_list{Provider, Adapter, App} : std::initializer_list{App, Adapter, Provider});
      ASSERT_TRUE(Input.parsed());
      ir::Module *Entry = Input.analyze();
      ASSERT_NE(Entry, nullptr);
      EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
      Input.expectResult(*Entry, 42);
    }
  }

  // Stored definition-time constants remain source ordered when an importer demands a later function first.
  TEST(SemanticModuleImportTest, PreservesDefinitionSnapshotsAcrossModuleOrder)
  {
    for (bool Reverse : {false, true})
    {
      const std::pair<std::string_view, std::string_view> App{"app", "from provider import read; func main(): i32 { return comptime read(); }"};
      const std::pair<std::string_view, std::string_view> Provider{"provider", "comptime var counter: i32 = 1; func snapshot(): i32 { return counter; } comptime { counter = 10; } func read(): i32 { return snapshot() + counter; }"};
      ModuleAnalysis Input(Reverse ? std::initializer_list{Provider, App} : std::initializer_list{App, Provider});
      ASSERT_TRUE(Input.parsed());
      ir::Module *Entry = Input.analyze();
      ASSERT_NE(Entry, nullptr);
      EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
      EXPECT_EQ(Input.Context.comptimeState().Bindings.size(), 1U);
      Input.expectResult(*Entry, 11);
    }
  }

  // A source module resolves the same later function definition when compiled alone or as an imported dependency.
  TEST(SemanticModuleImportTest, SupportsForwardDefinitionsWithoutImports)
  {
    ModuleAnalysis Input({{"app", "func main(): i32 { return later(); } func later(): i32 { return 42; }"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectResult(*Entry, 42);
  }

  // Preparing a compile-time call follows normal recursive references without treating them as a body cycle.
  TEST(SemanticModuleImportTest, ExecutesComptimeAcrossRecursiveModuleGraph)
  {
    ModuleAnalysis Input({
        {"app", "from left import bounce; func main(): i32 { return comptime bounce(3); }"},
        {"left", "from right import next; func bounce(n: i32): i32 { if (n == 3) { return next(2); } return 42; }"},
        {"right", "from left import bounce; func next(n: i32): i32 { return bounce(n); }"},
    });
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectResult(*Entry, 42);
  }

  // Compile-time evaluation never executes a function whose IR body is still being generated.
  TEST(SemanticModuleImportTest, RejectsCyclicComptimeBodyDependency)
  {
    ModuleAnalysis Input({
        {"app", "from peer import answer; func main(): i32 { return comptime answer(); }"},
        {"peer", "from app import main; func answer(): i32 { return main(); }"},
    });
    ASSERT_TRUE(Input.parsed());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticComptimeBodyDependency);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
  }

  // Nested definitions receive the same incomplete-body guard as top-level compile-time targets.
  TEST(SemanticModuleImportTest, RejectsExecutingIncompleteLocalFunction)
  {
    ModuleAnalysis Input({{"app", "func main(): void { func local(): i32 { return comptime local(); } }"}});
    ASSERT_TRUE(Input.parsed());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticComptimeBodyDependency);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
  }

  // Forward compile-time evaluation cannot move mutable module state past an unfinished earlier definition.
  TEST(SemanticModuleImportTest, RejectsAdvancingInitializationPastActiveDefinition)
  {
    for (bool CrossModule : {false, true})
    {
      SCOPED_TRACE(CrossModule);
      const std::string_view App = CrossModule ? "from peer import bridge; comptime var counter: i32 = 1; func main(): i32 { return comptime bridge() + counter; } comptime { counter = 10; } func later(): i32 { return counter; }" : "comptime var counter: i32 = 1; func main(): i32 { return comptime later() + counter; } comptime { counter = 10; } func later(): i32 { return counter; }";
      ModuleAnalysis Input({{"app", App}, {"peer", "from app import later; func bridge(): i32 { return later(); }"}});
      ASSERT_TRUE(Input.parsed());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticComptimeModuleDependency);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
    }
  }

  // Demanded top-level functions keep their declaration scope instead of capturing an active comptime block.
  TEST(SemanticModuleImportTest, PreservesScopeDuringTopLevelComptimeReentry)
  {
    ModuleAnalysis Valid({{"app", "comptime { var hidden: i32 = 7; main(); } func main(): i32 { return 42; }"}});
    ASSERT_TRUE(Valid.parsed());
    ir::Module *Entry = Valid.analyze();
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Valid.Diagnostics.diagnostics().empty());
    Valid.expectResult(*Entry, 42);
    ModuleAnalysis Invalid({{"app", "comptime { var hidden: i32 = 42; later(); } func later(): i32 { return hidden; }"}});
    ASSERT_TRUE(Invalid.parsed());
    EXPECT_EQ(Invalid.analyze(), nullptr);
    ASSERT_EQ(Invalid.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Invalid.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticUnknownName);
  }

  // An unfinished module initializer cannot recursively execute a later initializer through a forward function.
  TEST(SemanticModuleImportTest, RejectsAdvancingInitializationPastActiveStatement)
  {
    ModuleAnalysis Input({{"app", "comptime var counter: i32 = 1; comptime { later(); } comptime { counter = 10; } func later(): i32 { return counter; }"}});
    ASSERT_TRUE(Input.parsed());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticComptimeStatementDependency);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
  }

  // Self-import aliases retain access to private definitions without introducing external function dependencies.
  TEST(SemanticModuleImportTest, AllowsPrivateAliasesWithinDefiningModule)
  {
    ModuleAnalysis Input({{"app", "import app as Self; from app import hidden as read; private func hidden(): i32 { return 21; } func main(): i32 { return read() + Self.hidden(); }"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    EXPECT_TRUE(Input.Context.moduleImports(*Entry).empty());
    Input.expectResult(*Entry, 42);
  }

  // Relative level one starts in the importing file's package and higher levels ascend its parents.
  TEST(SemanticModuleImportTest, ResolvesRelativeImportsWithinSourceRoot)
  {
    const parser::NameToken Path[] = {{parser::InvalidTokenId, "helper", {}}};
    EXPECT_EQ(resolveImportModuleName("package.child.main", Path, 1), "package.child.helper");
    EXPECT_EQ(resolveImportModuleName("package.child.main", Path, 2), "package.helper");
    EXPECT_TRUE(resolveImportModuleName("package.child.main", Path, 3).empty());
    EXPECT_TRUE(resolveImportModuleName("main", Path, 1).empty());
    EXPECT_TRUE(resolveImportModuleName("package..main", Path, 1).empty());
    EXPECT_TRUE(resolveImportModuleName(".main", Path, 1).empty());
    EXPECT_EQ(resolveImportModuleName("ignored", Path), "helper");
    const parser::NameToken Invalid[] = {{parser::InvalidTokenId, "../escape", {}}};
    EXPECT_TRUE(resolveImportModuleName("package.main", Invalid, 1).empty());
    ModuleAnalysis Input({
        {"package.child.app", "from ..helper import answer; func main(): i32 { return answer(); }"},
        {"package.helper", "func answer(): i32 { return 42; }"},
    });
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze("package.child.app");
    ASSERT_NE(Entry, nullptr);
    Input.expectResult(*Entry, 42);
  }

  // Imported overload sets contain public definitions only, even when a private overload has the best parameter match.
  TEST(SemanticModuleImportTest, FiltersPrivateOverloadsBeforeResolution)
  {
    const std::string_view Library = "func choose(x: i32): i32 { return 42; } private func choose(x: u8): i32 { return 7; }";
    for (std::string_view App : {"from library import choose; func main(): i32 { return choose(1); }", "import library as L; func main(): i32 { return L.choose(1); }"})
    {
      ModuleAnalysis Input({{"app", App}, {"library", Library}});
      ASSERT_TRUE(Input.parsed());
      ir::Module *Entry = Input.analyze();
      ASSERT_NE(Entry, nullptr);
      ASSERT_EQ(Input.Context.moduleImports(*Entry).size(), 1U);
      EXPECT_EQ(Input.Context.moduleImports(*Entry).front()->visibility(), ir::VisibilityKind::Public);
      Input.expectResult(*Entry, 42);
    }
    for (std::string_view App : {"from library import choose; func main(): i32 { var x: u8 = 1; return choose(x); }", "import library as L; func main(): i32 { var x: u8 = 1; return L.choose(x); }"})
    {
      ModuleAnalysis Input({{"app", App}, {"library", Library}});
      ASSERT_TRUE(Input.parsed());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // Missing modules, private definitions, unsupported value imports and alias collisions produce recoverable source errors.
  TEST(SemanticModuleImportTest, ReportsInvalidImportsAsUserErrors)
  {
    struct Case
    {
        std::string_view App;
        std::string_view Library;
        core::DiagnosticKind Expected;
    };
    const Case Cases[] = {
        {"from absent import answer;", "", core::DiagnosticKind::SemanticModuleNotFound},
        {"from library import missing;", "func answer(): i32 { return 42; }", core::DiagnosticKind::SemanticImportNotFound},
        {"from library import answer;", "private func answer(): i32 { return 42; }", core::DiagnosticKind::SemanticPrivateImport},
        {"import library; func main(): i32 { return library.answer(); }", "private func answer(): i32 { return 42; }", core::DiagnosticKind::SemanticPrivateImport},
        {"from library import item;", "var item: i32 = 42;", core::DiagnosticKind::SemanticUnsupportedImport},
        {"from library import Item;", "private class Item {};", core::DiagnosticKind::SemanticInvalidMember},
        {"import library; func main(): i32 { return library.item(); }", "var item: i32 = 42;", core::DiagnosticKind::SemanticUnsupportedImport},
        {"import library; func main(): i32 { return library.Item(); }", "class Item {};", core::DiagnosticKind::SemanticTypeMismatch},
        {"from library import answer as main; func main(): i32 { return 0; }", "func answer(): i32 { return 42; }", core::DiagnosticKind::SemanticDuplicateName},
        {"func main(): i32 { return answer(); }", "func answer(): i32 { return 42; }", core::DiagnosticKind::SemanticUnknownName},
        {"from .library import answer;", "func answer(): i32 { return 42; }", core::DiagnosticKind::SemanticInvalidModulePath},
        {"comptime import library;", "", core::DiagnosticKind::SemanticImportRequiresTopLevel},
        {"func main(): void { import library; }", "", core::DiagnosticKind::SemanticImportRequiresTopLevel},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.App);
      ModuleAnalysis Input({{"app", Entry.App}, {"library", Entry.Library}});
      ASSERT_TRUE(Input.parsed());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, Entry.Expected);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
    }
  }

  // Demand-lowered imported class defaults observe earlier compile-time bindings at their definition, independently of input order.
  TEST(SemanticModuleImportTest, PreservesClassDefaultDefinitionSnapshot)
  {
    constexpr std::string_view App = "from library import P; func main(): i32 { var Value = P(); return Value.X + 2; }";
    constexpr std::string_view Library = "comptime var Seed = 40; class P { field X: i32 = comptime Seed; }; comptime { Seed = 9; }";
    for (bool LibraryFirst : {false, true})
    {
      ModuleAnalysis Input(LibraryFirst ? std::initializer_list<std::pair<std::string_view, std::string_view>>{{"library", Library}, {"app", App}} : std::initializer_list<std::pair<std::string_view, std::string_view>>{{"app", App}, {"library", Library}});
      ASSERT_TRUE(Input.parsed());
      ir::Module *Entry = Input.analyze();
      ASSERT_NE(Entry, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      Input.expectResult(*Entry, 42);
    }
  }

  // Modules with duplicate canonical identities cannot accidentally share a scope or overload set.
  TEST(SemanticModuleImportTest, RejectsDuplicateModuleIdentities)
  {
    ModuleAnalysis Input({{"app", "func main(): i32 { return 1; }"}, {"app", "func main(): i32 { return 2; }"}});
    ASSERT_TRUE(Input.parsed());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticDuplicateModule);
  }

  // Top-level default visibility is public, explicit private is retained, and nested functions are always private.
  TEST(SemanticModuleImportTest, StoresFunctionVisibilityInIr)
  {
    ModuleAnalysis Input({{"app", "func main(): i32 { func inner(): i32 { return 42; } return inner(); } private func hidden(): void {} public func shown(): void {}"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    const ir::Function *Main = Input.function(*Entry, "main");
    ASSERT_NE(Main, nullptr);
    EXPECT_EQ(Main->visibility(), ir::VisibilityKind::Public);
    EXPECT_EQ(Input.function(*Entry, "hidden")->visibility(), ir::VisibilityKind::Private);
    EXPECT_EQ(Input.function(*Entry, "shown")->visibility(), ir::VisibilityKind::Public);
    const auto *Inner = static_cast<const ir::Function *>(Main->entryBlock()->values().front().get());
    EXPECT_EQ(Inner->visibility(), ir::VisibilityKind::Private);
    Input.expectResult(*Entry, 42);
  }

  // Export visibility remains a source permission, and imported local C ABI bodies execute in their definition module.
  TEST(SemanticModuleImportTest, ImportsPublicCAbiDefinitionsAndKeepsPrivateExportsHidden)
  {
    ModuleAnalysis Input({{"provider", "[abi(\"C\")] private func local(Value: i32): i32 { return Value + 1; } private export \"C\" func hidden(Value: i32): i32 { return local(Value); } public export \"C\" func exposed(): i32 { return hidden(41); }"}, {"app", "from provider import exposed; func main(): i32 { return exposed(); }"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    Input.expectResult(*Entry, 42);
    ModuleAnalysis Denied({{"provider", "private export \"C\" func hidden(): i32 { return 42; }"}, {"app", "from provider import hidden; func main(): i32 { return hidden(); }"}});
    ASSERT_TRUE(Denied.parsed());
    EXPECT_EQ(Denied.analyze(), nullptr);
    ASSERT_EQ(Denied.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Denied.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticPrivateImport);
  }

  // Public native declarations may be imported through an Ink module without becoming local definitions.
  TEST(SemanticModuleImportTest, ImportsPublicNativeDeclarations)
  {
    ModuleAnalysis Input({{"provider", "public import \"C\" func abs(Value: i32): i32;"}, {"app", "from provider import abs; func main(): i32 { return abs(-42); }"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    Input.expectResult(*Entry, 42);
  }

  // Native imports resolve matching private exports through the native namespace at compile time and runtime.
  TEST(SemanticModuleImportTest, NativeImportsResolveExportedDefinitions)
  {
    ModuleAnalysis Input({{"app", "import \"C\" func nativeAnswer(): i32; comptime var Saved = nativeAnswer(); func main(): i32 { return nativeAnswer() + comptime Saved; }"}, {"provider", "private export \"C\" func nativeAnswer(): i32 { return 21; }"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    Input.expectResult(*Entry, 42);
  }

  // Compile-time C string conversion supplies writable pointer arguments to local, direct-export and native-resolved VM bodies.
  TEST(SemanticModuleImportTest, ConvertsComptimeStringsForCAbiDefinitions)
  {
    ModuleAnalysis Input({{"app", "import \"C\" func nativeFirst(Text: *u8): i32; [abi(\"C\")] func localFirst(Text: *u8): i32 { if (*Text == 65) { *Text = 66; return 14; } return 0; } export \"C\" func directFirst(Text: *u8): i32 { return localFirst(Text); } comptime var Saved = localFirst(\"A\") + directFirst(\"A\") + nativeFirst(\"A\"); func main(): i32 { return comptime Saved; }"}, {"provider", "private export \"C\" func nativeFirst(Text: *u8): i32 { if (*Text == 65) { return 14; } return 0; }"}});
    ASSERT_TRUE(Input.parsed());
    ir::Module *Entry = Input.analyze();
    ASSERT_NE(Entry, nullptr);
    Input.expectResult(*Entry, 42);
  }

  // A native name match with a different signature is diagnosed before body analysis or host symbol fallback.
  TEST(SemanticModuleImportTest, RejectsNativeImportExportSignatureMismatch)
  {
    ModuleAnalysis Input({{"app", "import \"C\" func nativeAnswer(): i64; func main(): i32 { return 0; }"}, {"provider", "private export \"C\" func nativeAnswer(): i32 { return 21; }"}});
    ASSERT_TRUE(Input.parsed());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticNativeImportSignatureMismatch);
  }

  // Native export names must be unique across modules even when both declarations are source-private.
  TEST(SemanticModuleImportTest, RejectsDuplicatePrivateNativeExportsAcrossModules)
  {
    ModuleAnalysis Input({{"provider", "private export \"C\" func entry(): i32 { return 1; }"}, {"app", "private export \"C\" func entry(): i32 { return 2; } func main(): i32 { return 0; }"}});
    ASSERT_TRUE(Input.parsed());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticDuplicateNativeExport);
  }

  // Bodyless Ink prototypes and public local definitions are user errors in both analyzer entry points.
  TEST(SemanticModuleImportTest, RejectsPrototypesAndPublicLocals)
  {
    const std::pair<std::string_view, core::DiagnosticKind> Cases[] = {
        {"func declared(): i32;", core::DiagnosticKind::SemanticFunctionRequiresBody},
        {"func outer(): void { public func inner(): void {} }", core::DiagnosticKind::SemanticPublicLocal},
    };
    for (const auto &[Source, Expected] : Cases)
    {
      for (bool Multiple : {false, true})
      {
        ModuleAnalysis Input({{"app", Source}});
        ASSERT_TRUE(Input.parsed());
        EXPECT_EQ(Multiple ? Input.analyze() : Analyzer{}.analyze(Input.Context, Input.Parsed.front()), nullptr);
        ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
        EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, Expected);
        EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
      }
    }
  }
} // namespace ink::semantic::test
