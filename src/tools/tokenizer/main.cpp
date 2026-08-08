#include "ink/tokenizer/tokenizer.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

int main(int argc, char **argv)
{
  std::string Source;
  if (argc == 1)
  {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    Source.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
  }
  else if (argc == 2)
  {
    std::ifstream Input(argv[1], std::ios::binary);
    if (!Input)
    {
      std::cerr << "cannot open " << argv[1] << '\n';
      return 2;
    }
    Source.assign(std::istreambuf_iterator<char>(Input), std::istreambuf_iterator<char>());
  }
  else
  {
    std::cerr << "usage: ink_tokenize [source-file]\n";
    return 2;
  }

  const ink::tokenizer::TokenizedBuffer Result = ink::tokenizer::tokenize(std::move(Source));
  for (const ink::tokenizer::Token &Token : Result.tokens())
  {
    std::cout << ink::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.Start << ", " << Token.Span.End << ")\n";
  }
  for (const ink::core::Diagnostic &Diagnostic : Result.diagnostics())
  {
    std::cerr << Diagnostic.Message << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ")\n";
  }
  return Result.succeeded() ? 0 : 1;
}
