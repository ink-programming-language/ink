#ifndef INK_TESTCASE_TOKENIZER_TOKENIZER_TEST_SUPPORT_H
#define INK_TESTCASE_TOKENIZER_TOKENIZER_TEST_SUPPORT_H

#include "ink/core/source_file_id.h"

namespace ink::tokenizer
{
  inline constexpr core::SourceFileId TestSourceFileId = core::SourceFileId::fromValue(0);
} // namespace ink::tokenizer

#endif
