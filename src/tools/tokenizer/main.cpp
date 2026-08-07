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

int main(int argc, char** argv) {
  std::string source;
  if (argc == 1) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    source.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
  } else if (argc == 2) {
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
      std::cerr << "cannot open " << argv[1] << '\n';
      return 2;
    }
    source.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
  } else {
    std::cerr << "usage: ink_tokenize [source-file]\n";
    return 2;
  }

  const ink::tokenizer::LexedFile result = ink::tokenizer::tokenize(std::move(source));
  for (const ink::tokenizer::Token& token : result.tokens()) {
    std::cout << ink::tokenizer::token_kind_name(token.kind) << " [" << token.span.start << ", " << token.span.end << ")\n";
  }
  for (const ink::tokenizer::Diagnostic& diagnostic : result.diagnostics()) {
    std::cerr << diagnostic.message << " [" << diagnostic.span.start << ", " << diagnostic.span.end << ")\n";
  }
  return result.succeeded() ? 0 : 1;
}
