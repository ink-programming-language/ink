#include "ink/tokenizer/tokenizer.h"

#include "unicode.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::tokenizer {
namespace {

using unicode::DecodeResult;

struct KeywordEntry {
  std::string_view spelling;
  KeywordKind kind;
};

struct BuiltinTypeEntry {
  std::string_view spelling;
  BuiltinTypeKind kind;
};

struct SuffixEntry {
  std::string_view spelling;
  NumericSuffix suffix;
  bool floating;
};

constexpr KeywordEntry kKeywords[] = {{"as", KeywordKind::As}, {"async", KeywordKind::Async}, {"await", KeywordKind::Await}, {"break", KeywordKind::Break}, {"catch", KeywordKind::Catch}, {"class", KeywordKind::Class}, {"comptime", KeywordKind::Comptime}, {"const", KeywordKind::Const}, {"continue", KeywordKind::Continue}, {"constructor", KeywordKind::Constructor}, {"decorator", KeywordKind::Decorator}, {"defer", KeywordKind::Defer}, {"destructor", KeywordKind::Destructor}, {"else", KeywordKind::Else}, {"enum", KeywordKind::Enum}, {"extern", KeywordKind::Extern}, {"for", KeywordKind::For}, {"from", KeywordKind::From}, {"func", KeywordKind::Func}, {"if", KeywordKind::If}, {"implicit", KeywordKind::Implicit}, {"import", KeywordKind::Import}, {"in", KeywordKind::In}, {"interface", KeywordKind::Interface}, {"let", KeywordKind::Let}, {"match", KeywordKind::Match}, {"override", KeywordKind::Override}, {"private", KeywordKind::Private}, {"return", KeywordKind::Return}, {"this", KeywordKind::This}, {"throw", KeywordKind::Throw}, {"try", KeywordKind::Try}, {"var", KeywordKind::Var}, {"virtual", KeywordKind::Virtual}, {"while", KeywordKind::While}};

constexpr BuiltinTypeEntry kBuiltinTypes[] = {{"i8", BuiltinTypeKind::I8}, {"i16", BuiltinTypeKind::I16}, {"i32", BuiltinTypeKind::I32}, {"i64", BuiltinTypeKind::I64}, {"i128", BuiltinTypeKind::I128}, {"u8", BuiltinTypeKind::U8}, {"u16", BuiltinTypeKind::U16}, {"u32", BuiltinTypeKind::U32}, {"u64", BuiltinTypeKind::U64}, {"u128", BuiltinTypeKind::U128}, {"int", BuiltinTypeKind::Int}, {"uint", BuiltinTypeKind::UInt}, {"ptrsize", BuiltinTypeKind::PtrSize}, {"f16", BuiltinTypeKind::F16}, {"f32", BuiltinTypeKind::F32}, {"f64", BuiltinTypeKind::F64}, {"bool", BuiltinTypeKind::Bool}, {"byte", BuiltinTypeKind::Byte}, {"void", BuiltinTypeKind::Void}, {"never", BuiltinTypeKind::Never}, {"type", BuiltinTypeKind::Type}};

constexpr SuffixEntry kSuffixes[] = {{"i8", NumericSuffix::I8, false}, {"i16", NumericSuffix::I16, false}, {"i32", NumericSuffix::I32, false}, {"i64", NumericSuffix::I64, false}, {"i128", NumericSuffix::I128, false}, {"u8", NumericSuffix::U8, false}, {"u16", NumericSuffix::U16, false}, {"u32", NumericSuffix::U32, false}, {"u64", NumericSuffix::U64, false}, {"u128", NumericSuffix::U128, false}, {"int", NumericSuffix::Int, false}, {"uint", NumericSuffix::UInt, false}, {"ptrsize", NumericSuffix::PtrSize, false}, {"byte", NumericSuffix::Byte, false}, {"f16", NumericSuffix::F16, true}, {"f32", NumericSuffix::F32, true}, {"f64", NumericSuffix::F64, true}};

constexpr std::string_view kSymbols = "(){}[],;:.@+-*/%=!&|^~<>";

bool starts_with(std::string_view source, std::size_t offset, std::string_view expected) noexcept {
  return offset <= source.size() && expected.size() <= source.size() - offset && source.compare(offset, expected.size(), expected) == 0;
}

bool is_ascii_digit(char value) noexcept { return value >= '0' && value <= '9'; }

int digit_value(char value) noexcept {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return value - 'a' + 10;
  }
  if (value >= 'A' && value <= 'F') {
    return value - 'A' + 10;
  }
  return -1;
}

bool is_hex_digit(char value) noexcept { return digit_value(value) >= 0; }

bool is_scalar_value(char32_t value) noexcept { return value <= 0x10FFFF && !(value >= 0xD800 && value <= 0xDFFF); }

bool is_forbidden_control(char32_t value) noexcept { return (value <= 0x1F && value != U'\t' && value != U'\n' && value != U'\r') || value == 0x7F; }

std::string diagnostic_message(DiagnosticKind kind) {
  return diagnostic_kind_name(kind);
}

std::string code_point_name(char32_t value) {
  constexpr char digits[] = "0123456789ABCDEF";
  std::string result = "U+";
  std::string hexadecimal;
  do {
    hexadecimal.push_back(digits[value & 0xFU]);
    value >>= 4U;
  } while (value != 0);
  while (hexadecimal.size() < 4) {
    hexadecimal.push_back('0');
  }
  result.append(hexadecimal.rbegin(), hexadecimal.rend());
  return result;
}

class Scanner {
 public:
  Scanner(const std::string& source, std::vector<Token>& tokens, std::vector<Diagnostic>& diagnostics, TokenizerOptions options) : source_(source), tokens_(tokens), diagnostics_(diagnostics), options_(options) {}

  void run() {
    if (starts_with(source_, 0, "\xEF\xBB\xBF")) {
      tokens_.push_back(make_token(TokenKind::Utf8Bom, 0, 3));
      position_ = 3;
    }
    while (position_ < source_.size()) {
      const std::size_t before = position_;
      Token token = scan_token();
      if (position_ <= before) {
        add_diagnostic(DiagnosticKind::InvalidCharacter, {before, std::min(before + 1, source_.size())});
        position_ = std::min(before + 1, source_.size());
        token = make_token(TokenKind::InvalidCharacter, before, position_);
      }
      tokens_.push_back(std::move(token));
    }
    tokens_.push_back(make_token(TokenKind::EndOfFile, source_.size(), source_.size()));
  }

 private:
  struct LiteralLine {
    std::size_t start;
    std::size_t end;
  };

  Token scan_token() {
    const std::size_t start = position_;
    const DecodeResult decoded = unicode::decode(source_, position_);
    if (!decoded.valid) {
      position_ += std::max<std::size_t>(decoded.length, 1);
      add_diagnostic(DiagnosticKind::InvalidUtf8, {start, position_});
      return make_token(TokenKind::InvalidEncoding, start, position_);
    }

    if (decoded.value == U'\r') {
      if (starts_with(source_, position_, "\r\n")) {
        position_ += 2;
        return make_token(TokenKind::LineBreak, start, position_);
      }
      position_ += decoded.length;
      add_diagnostic(DiagnosticKind::LoneCarriageReturn, {start, position_});
      return make_token(TokenKind::InvalidCharacter, start, position_);
    }
    if (decoded.value == U'\n') {
      ++position_;
      return make_token(TokenKind::LineBreak, start, position_);
    }
    if (decoded.value == U' ' || decoded.value == U'\t') {
      do {
        ++position_;
      } while (position_ < source_.size() && (source_[position_] == ' ' || source_[position_] == '\t'));
      return make_token(TokenKind::SpacesAndTabs, start, position_);
    }
    if (decoded.value == 0xFEFF) {
      position_ += decoded.length;
      add_diagnostic(DiagnosticKind::UnexpectedBom, {start, position_});
      return make_token(TokenKind::InvalidCharacter, start, position_);
    }
    if (is_forbidden_control(decoded.value)) {
      position_ += decoded.length;
      add_diagnostic(DiagnosticKind::ForbiddenControlCharacter, {start, position_});
      return make_token(TokenKind::InvalidCharacter, start, position_);
    }
    if (unicode::is_unicode_whitespace(decoded.value)) {
      position_ += decoded.length;
      add_diagnostic(DiagnosticKind::NonAsciiWhitespace, {start, position_});
      return make_token(TokenKind::InvalidCharacter, start, position_);
    }
    if (starts_with(source_, position_, "//")) {
      return scan_line_comment();
    }
    if (starts_with(source_, position_, "/*")) {
      return scan_block_comment();
    }
    if (starts_with(source_, position_, "r\"\"\"")) {
      return scan_multiline_string(true);
    }
    if (starts_with(source_, position_, "r\"")) {
      return scan_single_line_string(true);
    }
    if (starts_with(source_, position_, "\"\"\"")) {
      return scan_multiline_string(false);
    }
    if (decoded.value == U'"') {
      return scan_single_line_string(false);
    }
    if (decoded.value == U'\'') {
      return scan_scalar_literal();
    }
    if (decoded.value >= U'0' && decoded.value <= U'9') {
      return scan_number();
    }
    if (decoded.value == U'_' || unicode::is_xid_start(decoded.value)) {
      return scan_identifier();
    }
    if (decoded.value <= 0x7F && kSymbols.find(static_cast<char>(decoded.value)) != std::string_view::npos) {
      position_ += decoded.length;
      return make_token(TokenKind::Symbol, start, position_, static_cast<char>(decoded.value));
    }

    position_ += decoded.length;
    if (unicode::is_default_ignorable(decoded.value)) {
      add_invisible_diagnostic(start, decoded, previous_visible_scalar(start), next_visible_scalar(position_), false);
    } else {
      add_diagnostic(DiagnosticKind::InvalidCharacter, {start, position_});
    }
    return make_token(TokenKind::InvalidCharacter, start, position_);
  }

  Token scan_line_comment() {
    const std::size_t start = position_;
    position_ += 2;
    while (position_ < source_.size() && source_[position_] != '\n' && !(source_[position_] == '\r' && position_ + 1 < source_.size() && source_[position_ + 1] == '\n')) {
      ++position_;
    }
    const TokenKind validation = validate_raw_range(start, position_, true);
    return make_token(validation == TokenKind::Identifier ? TokenKind::LineComment : validation, start, position_);
  }

  Token scan_block_comment() {
    const std::size_t start = position_;
    std::size_t depth = 1;
    std::vector<std::size_t> opening_positions = {start};
    bool nesting_limit_exceeded = options_.max_block_comment_depth < 1;
    if (nesting_limit_exceeded) {
      add_diagnostic(DiagnosticKind::BlockCommentNestingLimit, {start, start + 2});
    }
    position_ += 2;
    while (position_ < source_.size() && depth != 0) {
      if (starts_with(source_, position_, "/*")) {
        ++depth;
        opening_positions.push_back(position_);
        if (depth > options_.max_block_comment_depth && !nesting_limit_exceeded) {
          nesting_limit_exceeded = true;
          add_diagnostic(DiagnosticKind::BlockCommentNestingLimit, {position_, position_ + 2});
        }
        position_ += 2;
      } else if (starts_with(source_, position_, "*/")) {
        --depth;
        opening_positions.pop_back();
        position_ += 2;
      } else {
        ++position_;
      }
    }
    if (depth != 0) {
      validate_raw_range(start, position_, true);
      diagnostics_.push_back({DiagnosticKind::UnterminatedBlockComment, {start, std::min(start + 2, source_.size())}, std::string("block comment is not terminated; outermost opening byte: ") + std::to_string(start) + "; remaining nesting depth: " + std::to_string(depth) + "; most recent unclosed opening byte: " + std::to_string(opening_positions.back())});
      return make_token(TokenKind::UnterminatedBlockComment, start, position_);
    }
    const TokenKind validation = validate_raw_range(start, position_, true);
    if (nesting_limit_exceeded && validation == TokenKind::Identifier) {
      return make_token(TokenKind::InvalidCharacter, start, position_);
    }
    return make_token(validation == TokenKind::Identifier ? TokenKind::BlockComment : validation, start, position_);
  }

  Token scan_identifier() {
    const std::size_t start = position_;
    bool invisible = false;
    std::size_t previous_visible = std::string::npos;
    while (position_ < source_.size()) {
      const DecodeResult decoded = unicode::decode(source_, position_);
      if (!decoded.valid || (decoded.value != U'_' && !unicode::is_xid_continue(decoded.value))) {
        break;
      }
      if (unicode::is_default_ignorable(decoded.value) || unicode::is_unicode_whitespace(decoded.value)) {
        invisible = true;
        const std::size_t run_start = position_;
        std::size_t run_end = position_;
        while (run_end < source_.size()) {
          const DecodeResult member = unicode::decode(source_, run_end);
          if (!member.valid || (member.value != U'_' && !unicode::is_xid_continue(member.value)) || (!unicode::is_default_ignorable(member.value) && !unicode::is_unicode_whitespace(member.value))) {
            break;
          }
          run_end += member.length;
        }
        const DecodeResult next = unicode::decode(source_, run_end);
        const std::size_t next_visible = run_end < source_.size() && next.valid && (next.value == U'_' || unicode::is_xid_continue(next.value)) ? run_end : std::string::npos;
        for (std::size_t member_offset = run_start; member_offset < run_end;) {
          const DecodeResult member = unicode::decode(source_, member_offset);
          add_invisible_diagnostic(member_offset, member, previous_visible, next_visible, true);
          member_offset += member.length;
        }
        position_ = run_end;
        continue;
      } else {
        previous_visible = position_;
      }
      position_ += decoded.length;
    }
    const std::string_view spelling(source_.data() + start, position_ - start);
    if (invisible) {
      return make_token(TokenKind::InvalidIdentifier, start, position_);
    }
    if (!unicode::is_nfc(spelling)) {
      add_diagnostic(DiagnosticKind::IdentifierNotNfc, {start, position_});
      return make_token(TokenKind::InvalidIdentifier, start, position_);
    }
    for (const KeywordEntry& entry : kKeywords) {
      if (entry.spelling == spelling) {
        return make_token(TokenKind::Keyword, start, position_, entry.kind);
      }
    }
    if (spelling == "true") {
      return make_token(TokenKind::BoolLiteral, start, position_, true);
    }
    if (spelling == "false") {
      return make_token(TokenKind::BoolLiteral, start, position_, false);
    }
    if (spelling == "null") {
      return make_token(TokenKind::NullLiteral, start, position_);
    }
    for (const BuiltinTypeEntry& entry : kBuiltinTypes) {
      if (entry.spelling == spelling) {
        return make_token(TokenKind::BuiltinType, start, position_, entry.kind);
      }
    }
    return make_token(TokenKind::Identifier, start, position_);
  }

  Token scan_number() {
    const std::size_t start = position_;
    unsigned base = 10;
    bool explicit_base = false;
    bool has_fraction = false;
    bool has_exponent = false;
    bool invalid = false;
    NumericSuffix suffix = NumericSuffix::None;

    if (position_ + 1 < source_.size() && source_[position_] == '0' && (source_[position_ + 1] == 'b' || source_[position_ + 1] == 'o' || source_[position_ + 1] == 'x')) {
      explicit_base = true;
      base = source_[position_ + 1] == 'b' ? 2U : source_[position_ + 1] == 'o' ? 8U : 16U;
      position_ += 2;
    }

    bool saw_digit = false;
    bool previous_digit = false;
    while (position_ < source_.size()) {
      const char current = source_[position_];
      const int value = digit_value(current);
      if (value >= 0 && (is_ascii_digit(current) || base == 16)) {
        if (static_cast<unsigned>(value) >= base) {
          invalid = true;
          add_diagnostic(DiagnosticKind::DigitOutOfRange, {position_, position_ + 1});
        } else {
          saw_digit = true;
        }
        previous_digit = static_cast<unsigned>(value) < base;
        ++position_;
        continue;
      }
      if (current == '_') {
        const bool next_is_digit = position_ + 1 < source_.size() && digit_value(source_[position_ + 1]) >= 0 && static_cast<unsigned>(digit_value(source_[position_ + 1])) < base;
        if (!previous_digit || !next_is_digit) {
          invalid = true;
          add_diagnostic(DiagnosticKind::MisplacedNumericSeparator, {position_, position_ + 1});
        }
        previous_digit = false;
        ++position_;
        continue;
      }
      break;
    }

    if (explicit_base && !saw_digit) {
      invalid = true;
      add_diagnostic(DiagnosticKind::MissingBaseDigits, {start, position_});
    }

    if (!explicit_base && position_ < source_.size() && source_[position_] == '.' && position_ + 1 < source_.size() && is_ascii_digit(source_[position_ + 1])) {
      has_fraction = true;
      ++position_;
      scan_decimal_component(invalid);
    } else if (!explicit_base && position_ + 2 < source_.size() && source_[position_] == '.' && source_[position_ + 1] == '_' && is_ascii_digit(source_[position_ + 2])) {
      has_fraction = true;
      ++position_;
      scan_decimal_component(invalid);
    } else if (explicit_base && position_ < source_.size() && source_[position_] == '.' && position_ + 1 < source_.size() && (is_ascii_digit(source_[position_ + 1]) || (base == 16 && is_hex_digit(source_[position_ + 1])))) {
      invalid = true;
      add_diagnostic(DiagnosticKind::UnsupportedNonDecimalFloat, {position_, position_ + 1});
      ++position_;
      while (position_ < source_.size() && (is_ascii_digit(source_[position_]) || (base == 16 && is_hex_digit(source_[position_])) || source_[position_] == '_')) {
        ++position_;
      }
    }

    if (!explicit_base && position_ < source_.size() && (source_[position_] == 'e' || source_[position_] == 'E')) {
      has_exponent = true;
      const std::size_t exponent_start = position_++;
      if (position_ < source_.size() && (source_[position_] == '+' || source_[position_] == '-')) {
        ++position_;
      }
      const std::size_t digits_start = position_;
      const bool exponent_has_digits = scan_decimal_component(invalid);
      if (!exponent_has_digits) {
        invalid = true;
        add_diagnostic(DiagnosticKind::MissingExponentDigits, {exponent_start, std::max(position_, digits_start)});
      }
    }

    const std::size_t suffix_start = position_;
    if (position_ < source_.size()) {
      const DecodeResult decoded = unicode::decode(source_, position_);
      if (decoded.valid && (decoded.value == U'_' || unicode::is_xid_start(decoded.value))) {
        if (unicode::is_default_ignorable(decoded.value)) {
          invalid = true;
          add_diagnostic(DiagnosticKind::InvisibleCharacter, {position_, position_ + decoded.length});
        }
        position_ += decoded.length;
        while (position_ < source_.size()) {
          const DecodeResult continued = unicode::decode(source_, position_);
          if (!continued.valid || (continued.value != U'_' && !unicode::is_xid_continue(continued.value))) {
            break;
          }
          if (unicode::is_default_ignorable(continued.value)) {
            invalid = true;
            add_diagnostic(DiagnosticKind::InvisibleCharacter, {position_, position_ + continued.length});
          }
          position_ += continued.length;
        }
        const std::string_view spelling(source_.data() + suffix_start, position_ - suffix_start);
        const SuffixEntry* entry = nullptr;
        for (const SuffixEntry& candidate : kSuffixes) {
          if (candidate.spelling == spelling) {
            entry = &candidate;
            break;
          }
        }
        if (entry == nullptr) {
          invalid = true;
          const bool looks_like_non_decimal_exponent = explicit_base && !spelling.empty() && (spelling.front() == 'e' || spelling.front() == 'E' || spelling.front() == 'p' || spelling.front() == 'P');
          add_diagnostic(looks_like_non_decimal_exponent ? DiagnosticKind::UnsupportedNonDecimalFloat : DiagnosticKind::UnknownNumericSuffix, {suffix_start, position_});
        } else if (explicit_base && entry->floating) {
          invalid = true;
          add_diagnostic(DiagnosticKind::InvalidNumericSuffix, {suffix_start, position_});
        } else if ((has_fraction || has_exponent) && !entry->floating) {
          invalid = true;
          add_diagnostic(DiagnosticKind::InvalidNumericSuffix, {suffix_start, position_});
        } else {
          suffix = entry->suffix;
          if (entry->floating) {
            has_fraction = true;
          }
        }
      }
    }

    if (invalid) {
      return make_token(TokenKind::InvalidNumber, start, position_);
    }
    const TokenKind kind = has_fraction || has_exponent ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral;
    return make_token(kind, start, position_, NumericInfo{base, suffix});
  }

  bool scan_decimal_component(bool& invalid) {
    bool saw_digit = false;
    bool previous_digit = false;
    while (position_ < source_.size()) {
      if (is_ascii_digit(source_[position_])) {
        saw_digit = true;
        previous_digit = true;
        ++position_;
      } else if (source_[position_] == '_') {
        const bool next_is_digit = position_ + 1 < source_.size() && is_ascii_digit(source_[position_ + 1]);
        if (!previous_digit || !next_is_digit) {
          invalid = true;
          add_diagnostic(DiagnosticKind::MisplacedNumericSeparator, {position_, position_ + 1});
        }
        previous_digit = false;
        ++position_;
      } else {
        break;
      }
    }
    return saw_digit;
  }

  Token scan_scalar_literal() {
    const std::size_t start = position_++;
    std::vector<char32_t> values;
    bool invalid = false;
    bool closed = false;
    while (position_ < source_.size()) {
      if (source_[position_] == '\'') {
        ++position_;
        closed = true;
        break;
      }
      if (source_[position_] == '\n' || (source_[position_] == '\r' && position_ + 1 < source_.size() && source_[position_ + 1] == '\n')) {
        invalid = true;
        add_diagnostic(DiagnosticKind::UnterminatedScalarLiteral, {start, position_});
        break;
      }
      if (source_[position_] == '\\') {
        char32_t value = 0;
        bool produced = false;
        if (!scan_escape(position_, source_.size(), value, produced)) {
          invalid = true;
        }
        if (produced) {
          values.push_back(value);
        }
        continue;
      }
      const DecodeResult decoded = unicode::decode(source_, position_);
      if (!decoded.valid) {
        invalid = true;
        add_diagnostic(DiagnosticKind::InvalidUtf8, {position_, position_ + std::max<std::size_t>(decoded.length, 1)});
        position_ += std::max<std::size_t>(decoded.length, 1);
        continue;
      }
      if (is_forbidden_control(decoded.value) || decoded.value == U'\r') {
        invalid = true;
        add_diagnostic(decoded.value == U'\r' ? DiagnosticKind::LoneCarriageReturn : DiagnosticKind::ForbiddenControlCharacter, {position_, position_ + decoded.length});
      }
      if (unicode::is_default_ignorable(decoded.value)) {
        invalid = true;
        add_diagnostic(DiagnosticKind::InvisibleCharacter, {position_, position_ + decoded.length});
      }
      values.push_back(decoded.value);
      position_ += decoded.length;
    }
    if (!closed && position_ == source_.size()) {
      invalid = true;
      add_diagnostic(DiagnosticKind::UnterminatedScalarLiteral, {start, position_});
    }
    if (values.empty() && closed) {
      invalid = true;
      add_diagnostic(DiagnosticKind::EmptyScalarLiteral, {start, position_});
    } else if (values.size() > 1) {
      invalid = true;
      add_diagnostic(DiagnosticKind::MultipleScalarValues, {start, position_});
    }
    if (invalid || !closed || values.size() != 1) {
      return make_token(TokenKind::InvalidScalarLiteral, start, position_);
    }
    return make_token(TokenKind::ScalarLiteral, start, position_, values.front());
  }

  Token scan_single_line_string(bool raw_mode) {
    const std::size_t start = position_;
    position_ += raw_mode ? 2 : 1;
    std::string decoded_value;
    bool invalid = false;
    bool closed = false;
    while (position_ < source_.size()) {
      if (source_[position_] == '"') {
        ++position_;
        closed = true;
        break;
      }
      if (source_[position_] == '\n' || (source_[position_] == '\r' && position_ + 1 < source_.size() && source_[position_ + 1] == '\n')) {
        invalid = true;
        add_diagnostic(DiagnosticKind::UnterminatedStringLiteral, {start, position_});
        break;
      }
      if (!raw_mode && source_[position_] == '\\') {
        char32_t value = 0;
        bool produced = false;
        if (!scan_escape(position_, source_.size(), value, produced)) {
          invalid = true;
        }
        if (produced) {
          unicode::append_utf8(decoded_value, value);
        }
        continue;
      }
      if (!scan_literal_scalar(position_, source_.size(), decoded_value)) {
        invalid = true;
      }
    }
    if (!closed && position_ == source_.size()) {
      invalid = true;
      add_diagnostic(DiagnosticKind::UnterminatedStringLiteral, {start, position_});
    }
    if (invalid || !closed) {
      return make_token(TokenKind::InvalidStringLiteral, start, position_);
    }
    const StringMode mode = raw_mode ? StringMode::RawSingleLine : StringMode::EscapedSingleLine;
    return make_token(TokenKind::StringLiteral, start, position_, StringInfo{mode, std::move(decoded_value)});
  }

  Token scan_multiline_string(bool raw_mode) {
    const std::size_t start = position_;
    position_ += raw_mode ? 4 : 3;
    std::size_t opening_line_break_length = logical_line_break_length(position_);
    if (opening_line_break_length == 0) {
      add_diagnostic(DiagnosticKind::MultilineOpeningLineBreakRequired, {start, position_});
      std::size_t closing = std::string::npos;
      for (std::size_t line_start = position_; line_start < source_.size();) {
        const std::size_t line_break = source_.find('\n', line_start);
        if (line_break == std::string::npos) {
          break;
        }
        std::size_t candidate = line_break + 1;
        while (candidate < source_.size() && (source_[candidate] == ' ' || source_[candidate] == '\t')) {
          ++candidate;
        }
        if (starts_with(source_, candidate, "\"\"\"")) {
          closing = candidate;
          break;
        }
        line_start = line_break + 1;
      }
      if (closing == std::string::npos) {
        closing = source_.find("\"\"\"", position_);
      }
      position_ = closing == std::string::npos ? source_.size() : closing + 3;
      validate_raw_range(start, position_, false);
      return make_token(TokenKind::InvalidStringLiteral, start, position_);
    }
    position_ += opening_line_break_length;

    std::vector<LiteralLine> lines;
    std::size_t closing_start = std::string::npos;
    std::size_t closing_line_start = std::string::npos;
    while (position_ < source_.size()) {
      const std::size_t line_start = position_;
      std::size_t line_end = position_;
      while (line_end < source_.size() && source_[line_end] != '\n' && !(source_[line_end] == '\r' && line_end + 1 < source_.size() && source_[line_end + 1] == '\n')) {
        ++line_end;
      }
      std::size_t first_content = line_start;
      while (first_content < line_end && (source_[first_content] == ' ' || source_[first_content] == '\t')) {
        ++first_content;
      }
      if (starts_with(source_, first_content, "\"\"\"")) {
        closing_start = first_content;
        closing_line_start = line_start;
        position_ = first_content + 3;
        break;
      }
      lines.push_back({line_start, line_end});
      if (line_end == source_.size()) {
        position_ = line_end;
        break;
      }
      position_ = line_end + logical_line_break_length(line_end);
    }

    if (closing_start == std::string::npos) {
      position_ = source_.size();
      validate_raw_range(start, position_, false);
      add_diagnostic(DiagnosticKind::UnterminatedMultilineStringLiteral, {start, position_});
      return make_token(TokenKind::InvalidStringLiteral, start, position_);
    }

    const std::string_view indentation(source_.data() + closing_line_start, closing_start - closing_line_start);
    std::string decoded_value;
    bool invalid = false;
    for (std::size_t line_index = 0; line_index < lines.size(); ++line_index) {
      const LiteralLine line = lines[line_index];
      std::size_t content_start = line.start;
      const std::string_view raw_line(source_.data() + line.start, line.end - line.start);
      const bool whitespace_only = std::all_of(raw_line.begin(), raw_line.end(), [](char value) { return value == ' ' || value == '\t'; });
      if (raw_line.size() >= indentation.size() && raw_line.substr(0, indentation.size()) == indentation) {
        content_start += indentation.size();
      } else if (whitespace_only && indentation.size() >= raw_line.size() && indentation.substr(0, raw_line.size()) == raw_line) {
        content_start = line.end;
      } else {
        invalid = true;
        add_diagnostic(DiagnosticKind::InvalidMultilineIndentation, {line.start, line.end});
      }

      std::size_t cursor = content_start;
      while (cursor < line.end) {
        if (!raw_mode && source_[cursor] == '\\') {
          char32_t value = 0;
          bool produced = false;
          if (!scan_escape(cursor, line.end, value, produced)) {
            invalid = true;
          }
          if (produced) {
            unicode::append_utf8(decoded_value, value);
          }
        } else if (!scan_literal_scalar(cursor, line.end, decoded_value)) {
          invalid = true;
        }
      }
      if (line_index + 1 < lines.size()) {
        decoded_value.push_back('\n');
      }
    }

    if (invalid) {
      return make_token(TokenKind::InvalidStringLiteral, start, position_);
    }
    const StringMode mode = raw_mode ? StringMode::RawMultiline : StringMode::EscapedMultiline;
    return make_token(TokenKind::StringLiteral, start, position_, StringInfo{mode, std::move(decoded_value)});
  }

  bool scan_escape(std::size_t& cursor, std::size_t limit, char32_t& value, bool& produced) {
    const std::size_t start = cursor;
    produced = false;
    if (cursor + 1 >= limit) {
      cursor = limit;
      add_diagnostic(DiagnosticKind::UnknownEscape, {start, cursor});
      return false;
    }
    const DecodeResult escaped_character = unicode::decode(source_, cursor + 1);
    if (!escaped_character.valid) {
      const std::size_t invalid_length = std::max<std::size_t>(escaped_character.length, 1);
      cursor = std::min(cursor + 1 + invalid_length, limit);
      add_diagnostic(DiagnosticKind::InvalidUtf8, {start + 1, cursor});
      return false;
    }
    if (is_forbidden_control(escaped_character.value)) {
      add_diagnostic(DiagnosticKind::ForbiddenControlCharacter, {cursor + 1, cursor + 1 + escaped_character.length});
    }
    if (unicode::is_default_ignorable(escaped_character.value)) {
      add_diagnostic(DiagnosticKind::InvisibleCharacter, {cursor + 1, cursor + 1 + escaped_character.length});
    }
    const char kind = source_[cursor + 1];
    if (kind == '\n' || kind == '\r') {
      cursor = start + 1;
      add_diagnostic(DiagnosticKind::UnknownEscape, {start, cursor});
      return false;
    }
    switch (kind) {
      case '\\': value = U'\\'; cursor += 2; produced = true; return true;
      case '\'': value = U'\''; cursor += 2; produced = true; return true;
      case '"': value = U'"'; cursor += 2; produced = true; return true;
      case '0': value = U'\0'; cursor += 2; produced = true; return true;
      case 'n': value = U'\n'; cursor += 2; produced = true; return true;
      case 'r': value = U'\r'; cursor += 2; produced = true; return true;
      case 't': value = U'\t'; cursor += 2; produced = true; return true;
      case 'x': {
        if (cursor + 3 >= limit || !is_hex_digit(source_[cursor + 2]) || !is_hex_digit(source_[cursor + 3])) {
          cursor += 2;
          while (cursor < limit && cursor < start + 4 && is_hex_digit(source_[cursor])) {
            ++cursor;
          }
          add_diagnostic(DiagnosticKind::InvalidHexEscape, {start, cursor});
          return false;
        }
        value = static_cast<char32_t>(digit_value(source_[cursor + 2]) * 16 + digit_value(source_[cursor + 3]));
        cursor += 4;
        produced = true;
        return true;
      }
      case 'u': {
        if (cursor + 2 >= limit || source_[cursor + 2] != '{') {
          cursor += 2;
          add_diagnostic(DiagnosticKind::InvalidUnicodeEscape, {start, cursor});
          return false;
        }
        std::size_t index = cursor + 3;
        std::size_t digit_count = 0;
        char32_t scalar = 0;
        bool valid_digits = true;
        while (index < limit && source_[index] != '}' && source_[index] != '"' && source_[index] != '\'' && source_[index] != '\n' && source_[index] != '\r' && source_[index] != '\\') {
          const DecodeResult escaped_digit = unicode::decode(source_, index);
          if (!escaped_digit.valid) {
            const std::size_t invalid_length = std::max<std::size_t>(escaped_digit.length, 1);
            add_diagnostic(DiagnosticKind::InvalidUtf8, {index, std::min(index + invalid_length, limit)});
            valid_digits = false;
            ++digit_count;
            index = std::min(index + invalid_length, limit);
            continue;
          }
          if (is_forbidden_control(escaped_digit.value) || escaped_digit.value == U'\r') {
            add_diagnostic(escaped_digit.value == U'\r' ? DiagnosticKind::LoneCarriageReturn : DiagnosticKind::ForbiddenControlCharacter, {index, index + escaped_digit.length});
            valid_digits = false;
          }
          if (unicode::is_default_ignorable(escaped_digit.value)) {
            add_diagnostic(DiagnosticKind::InvisibleCharacter, {index, index + escaped_digit.length});
            valid_digits = false;
          }
          const int digit = digit_value(source_[index]);
          if (digit < 0) {
            valid_digits = false;
          } else if (digit_count < 7) {
            scalar = static_cast<char32_t>((scalar << 4U) | digit);
          }
          ++digit_count;
          index += escaped_digit.length;
        }
        if (index >= limit || source_[index] != '}') {
          cursor = index;
          add_diagnostic(DiagnosticKind::InvalidUnicodeEscape, {start, cursor});
          return false;
        }
        cursor = index + 1;
        if (!valid_digits || digit_count == 0 || digit_count > 6) {
          add_diagnostic(DiagnosticKind::InvalidUnicodeEscape, {start, cursor});
          return false;
        }
        if (!is_scalar_value(scalar)) {
          add_diagnostic(DiagnosticKind::InvalidUnicodeScalar, {start, cursor});
          return false;
        }
        value = scalar;
        produced = true;
        return true;
      }
      default: {
        const DecodeResult escaped = unicode::decode(source_, cursor + 1);
        cursor += 1 + std::max<std::size_t>(escaped.length, 1);
        add_diagnostic(DiagnosticKind::UnknownEscape, {start, cursor});
        return false;
      }
    }
  }

  bool scan_literal_scalar(std::size_t& cursor, std::size_t limit, std::string& decoded_value) {
    const DecodeResult decoded = unicode::decode(source_, cursor);
    if (!decoded.valid || cursor + decoded.length > limit) {
      const std::size_t length = std::max<std::size_t>(decoded.length, 1);
      add_diagnostic(DiagnosticKind::InvalidUtf8, {cursor, std::min(cursor + length, limit)});
      cursor = std::min(cursor + length, limit);
      return false;
    }
    bool valid = true;
    if (is_forbidden_control(decoded.value) || decoded.value == U'\r') {
      valid = false;
      add_diagnostic(decoded.value == U'\r' ? DiagnosticKind::LoneCarriageReturn : DiagnosticKind::ForbiddenControlCharacter, {cursor, cursor + decoded.length});
    }
    if (unicode::is_default_ignorable(decoded.value)) {
      valid = false;
      add_diagnostic(DiagnosticKind::InvisibleCharacter, {cursor, cursor + decoded.length});
    }
    unicode::append_utf8(decoded_value, decoded.value);
    cursor += decoded.length;
    return valid;
  }

  TokenKind validate_raw_range(std::size_t start, std::size_t end, bool allow_invisible) {
    TokenKind result = TokenKind::Identifier;
    for (std::size_t cursor = start; cursor < end;) {
      const DecodeResult decoded = unicode::decode(source_, cursor);
      if (!decoded.valid) {
        const std::size_t length = std::max<std::size_t>(decoded.length, 1);
        add_diagnostic(DiagnosticKind::InvalidUtf8, {cursor, std::min(cursor + length, end)});
        result = TokenKind::InvalidEncoding;
        cursor = std::min(cursor + length, end);
        continue;
      }
      if (is_forbidden_control(decoded.value) || (decoded.value == U'\r' && !(cursor + 1 < end && source_[cursor + 1] == '\n'))) {
        add_diagnostic(decoded.value == U'\r' ? DiagnosticKind::LoneCarriageReturn : DiagnosticKind::ForbiddenControlCharacter, {cursor, cursor + decoded.length});
        if (result != TokenKind::InvalidEncoding) {
          result = TokenKind::InvalidCharacter;
        }
      }
      if (!allow_invisible && unicode::is_default_ignorable(decoded.value)) {
        add_diagnostic(DiagnosticKind::InvisibleCharacter, {cursor, cursor + decoded.length});
        if (result != TokenKind::InvalidEncoding) {
          result = TokenKind::InvalidCharacter;
        }
      }
      cursor += decoded.length;
    }
    return result;
  }

  std::size_t logical_line_break_length(std::size_t offset) const noexcept {
    if (offset < source_.size() && source_[offset] == '\n') {
      return 1;
    }
    if (offset + 1 < source_.size() && source_[offset] == '\r' && source_[offset + 1] == '\n') {
      return 2;
    }
    return 0;
  }

  void add_diagnostic(DiagnosticKind kind, ByteSpan span) { diagnostics_.push_back({kind, span, diagnostic_message(kind)}); }

  std::string describe_scalar(std::size_t offset) const {
    const DecodeResult decoded = unicode::decode(source_, offset);
    if (!decoded.valid) {
      return "invalid UTF-8";
    }
    return code_point_name(decoded.value) + " ('" + std::string(source_.data() + offset, decoded.length) + "')";
  }

  std::size_t previous_visible_scalar(std::size_t offset) const {
    if (offset == 0) {
      return std::string::npos;
    }
    std::size_t candidate = offset - 1;
    while (candidate != 0 && (static_cast<unsigned char>(source_[candidate]) & 0xC0U) == 0x80U) {
      --candidate;
    }
    const DecodeResult decoded = unicode::decode(source_, candidate);
    return decoded.valid && candidate + decoded.length == offset && !unicode::is_default_ignorable(decoded.value) && !is_forbidden_control(decoded.value) && decoded.value != U' ' && decoded.value != U'\t' && decoded.value != U'\r' && decoded.value != U'\n' && !unicode::is_unicode_whitespace(decoded.value) ? candidate : std::string::npos;
  }

  std::size_t next_visible_scalar(std::size_t offset) const {
    const DecodeResult decoded = unicode::decode(source_, offset);
    return offset < source_.size() && decoded.valid && !unicode::is_default_ignorable(decoded.value) && !is_forbidden_control(decoded.value) && decoded.value != U' ' && decoded.value != U'\t' && decoded.value != U'\r' && decoded.value != U'\n' && !unicode::is_unicode_whitespace(decoded.value) ? offset : std::string::npos;
  }

  void add_invisible_diagnostic(std::size_t offset, DecodeResult decoded, std::size_t previous_visible, std::size_t next_visible, bool identifier_context) {
    std::string message = "invisible format character " + code_point_name(decoded.value);
    if (previous_visible != std::string::npos && next_visible != std::string::npos) {
      message += " appears between " + describe_scalar(previous_visible) + " and " + describe_scalar(next_visible);
    } else if (previous_visible != std::string::npos) {
      message += " appears after " + describe_scalar(previous_visible);
    } else if (next_visible != std::string::npos) {
      message += " appears before " + describe_scalar(next_visible);
    } else {
      message += identifier_context ? " appears in an identifier" : " appears in source text";
    }
    diagnostics_.push_back({DiagnosticKind::InvisibleCharacter, {offset, offset + decoded.length}, std::move(message)});
  }

  Token make_token(TokenKind kind, std::size_t start, std::size_t end, TokenPayload payload = {}) const { return {kind, {start, end}, std::move(payload)}; }

  const std::string& source_;
  std::vector<Token>& tokens_;
  std::vector<Diagnostic>& diagnostics_;
  TokenizerOptions options_;
  std::size_t position_ = 0;
};

}  // namespace

std::string_view LexedFile::raw(const Token& token) const noexcept {
  if (token.span.start > token.span.end || token.span.end > source_.size()) {
    return {};
  }
  return std::string_view(source_.data() + token.span.start, token.span.size());
}

std::size_t LexedFile::line_number(std::size_t byte_offset) const noexcept {
  const std::size_t clamped_offset = std::min(byte_offset, source_.size());
  return static_cast<std::size_t>(std::upper_bound(line_starts_.begin(), line_starts_.end(), clamped_offset) - line_starts_.begin());
}

bool LexedFile::succeeded() const noexcept {
  return diagnostics_.empty() && std::none_of(tokens_.begin(), tokens_.end(), [](const Token& token) { return token.is_error(); });
}

Tokenizer::Tokenizer(TokenizerOptions options) : options_(options) {}

LexedFile Tokenizer::tokenize(std::string source) const {
  LexedFile result;
  result.source_ = std::move(source);
  result.line_starts_.push_back(0);
  for (std::size_t index = 0; index < result.source_.size(); ++index) {
    if (result.source_[index] == '\n') {
      result.line_starts_.push_back(index + 1);
    }
  }
  Scanner scanner(result.source_, result.tokens_, result.diagnostics_, options_);
  scanner.run();
  return result;
}

LexedFile tokenize(std::string source, TokenizerOptions options) { return Tokenizer(options).tokenize(std::move(source)); }

const char* diagnostic_kind_name(DiagnosticKind kind) noexcept {
  switch (kind) {
    case DiagnosticKind::InvalidUtf8: return "invalid UTF-8";
    case DiagnosticKind::UnexpectedBom: return "UTF-8 BOM is only allowed at the start of a file";
    case DiagnosticKind::LoneCarriageReturn: return "carriage return must be followed by line feed";
    case DiagnosticKind::NonAsciiWhitespace: return "only ASCII space and tab are source whitespace";
    case DiagnosticKind::ForbiddenControlCharacter: return "forbidden raw control character";
    case DiagnosticKind::InvalidCharacter: return "character cannot start an Ink token";
    case DiagnosticKind::IdentifierNotNfc: return "identifier is not in Unicode NFC";
    case DiagnosticKind::InvisibleCharacter: return "invisible format character must be written explicitly";
    case DiagnosticKind::MissingBaseDigits: return "base prefix must be followed by a digit";
    case DiagnosticKind::DigitOutOfRange: return "digit does not belong to the literal base";
    case DiagnosticKind::MisplacedNumericSeparator: return "numeric separator must be between two digits";
    case DiagnosticKind::MissingExponentDigits: return "exponent must contain a decimal digit";
    case DiagnosticKind::UnknownNumericSuffix: return "unknown numeric literal suffix";
    case DiagnosticKind::InvalidNumericSuffix: return "numeric suffix is not valid for this literal";
    case DiagnosticKind::UnsupportedNonDecimalFloat: return "non-decimal floating-point literals are not supported";
    case DiagnosticKind::EmptyScalarLiteral: return "scalar literal is empty";
    case DiagnosticKind::MultipleScalarValues: return "scalar literal must contain exactly one Unicode scalar value";
    case DiagnosticKind::UnterminatedScalarLiteral: return "scalar literal is not terminated on this source line";
    case DiagnosticKind::UnknownEscape: return "unknown escape sequence";
    case DiagnosticKind::InvalidHexEscape: return "hex escape requires exactly two hexadecimal digits";
    case DiagnosticKind::InvalidUnicodeEscape: return "Unicode escape requires one to six hexadecimal digits in braces";
    case DiagnosticKind::InvalidUnicodeScalar: return "escape does not designate a Unicode scalar value";
    case DiagnosticKind::UnterminatedStringLiteral: return "single-line string is not terminated on this source line";
    case DiagnosticKind::MultilineOpeningLineBreakRequired: return "multiline string opening delimiter must be followed by a line break";
    case DiagnosticKind::UnterminatedMultilineStringLiteral: return "multiline string has no closing delimiter";
    case DiagnosticKind::InvalidMultilineIndentation: return "multiline string line does not match the closing indentation";
    case DiagnosticKind::UnterminatedBlockComment: return "block comment is not terminated";
    case DiagnosticKind::BlockCommentNestingLimit: return "block comment nesting limit exceeded";
  }
  return "unknown tokenizer diagnostic";
}

}  // namespace ink::tokenizer
