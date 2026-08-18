#include "ink/ir/serialization.h"

#include "ink/ir/analysis/verifier.h"
#include "text/lexer.h"
#include "text/module_draft.h"
#include "text/parser.h"
#include "text/printer.h"
#include "text/resolver.h"

#include <string_view>
#include <utility>
#include <vector>

namespace ink::ir
{
  SerializeResult printText(IRContext &Context, const Module &ModuleValue)
  {
    SerializeResult Result;
    const VerificationResult Verification = verify(Context, ModuleValue);
    if (!Verification.succeeded())
    {
      Result.Diagnostics = Verification.diagnostics();
      return Result;
    }
    Result.Text = text::printModule(ModuleValue);
    return Result;
  }

  SerializeResult printText(const Module &ModuleValue)
  {
    return printText(ModuleValue.context(), ModuleValue);
  }

  DeserializeResult parseText(IRContext &Context, std::string_view Text)
  {
    DeserializeResult Result;
    std::vector<text::Token> Tokens;
    core::Diagnostic Error;
    if (!text::tokenize(Text, Tokens, Error))
    {
      Context.diagnosticEngine().report(Error);
      Result.Diagnostics.push_back(std::move(Error));
      return Result;
    }

    text::ModuleDraft Draft(Context);
    if (!text::parse(Draft, std::move(Tokens), Error) || !text::resolveReferences(Draft, Error))
    {
      Context.diagnosticEngine().report(Error);
      Result.Diagnostics.push_back(std::move(Error));
      return Result;
    }

    Module ModuleValue = Draft.Builder.takeModule();
    const VerificationResult Verification = verify(Context, ModuleValue, core::DiagnosticClass::User);
    if (!Verification.succeeded())
    {
      Result.Diagnostics = Verification.diagnostics();
      return Result;
    }
    Result.Value = std::move(ModuleValue);
    return Result;
  }

  SerializeResult serialize(IRContext &Context, const Module &ModuleValue)
  {
    return printText(Context, ModuleValue);
  }

  SerializeResult serialize(const Module &ModuleValue)
  {
    return printText(ModuleValue);
  }

  DeserializeResult deserialize(IRContext &Context, std::string_view Text)
  {
    return parseText(Context, Text);
  }
} // namespace ink::ir
