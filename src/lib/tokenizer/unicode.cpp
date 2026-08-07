#include "unicode.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

namespace ink::tokenizer::unicode {
namespace {

struct CodePointRange {
  char32_t lower;
  char32_t upper;
};

struct DecompositionEntry {
  char32_t codepoint;
  std::uint32_t offset;
  std::uint8_t length;
};

struct CombiningClassEntry {
  char32_t codepoint;
  std::uint8_t value;
};

struct CompositionEntry {
  std::uint64_t key;
  char32_t codepoint;
};

#include "unicode_data.inc"

template <typename Range, std::size_t Size>
bool contains(const Range (&ranges)[Size], char32_t value) noexcept {
  std::size_t first = 0;
  std::size_t count = Size;
  while (count != 0) {
    const std::size_t step = count / 2;
    const std::size_t index = first + step;
    if (ranges[index].upper < value) {
      first = index + 1;
      count -= step + 1;
    } else {
      count = step;
    }
  }
  return first != Size && ranges[first].lower <= value;
}

std::uint8_t combining_class(char32_t value) noexcept {
  const auto* begin = std::begin(kCombiningClasses);
  const auto* end = std::end(kCombiningClasses);
  const auto* result = std::lower_bound(begin, end, value, [](const CombiningClassEntry& entry, char32_t candidate) { return entry.codepoint < candidate; });
  return result != end && result->codepoint == value ? result->value : 0;
}

const DecompositionEntry* decomposition(char32_t value) noexcept {
  const auto* begin = std::begin(kCanonicalDecompositions);
  const auto* end = std::end(kCanonicalDecompositions);
  const auto* result = std::lower_bound(begin, end, value, [](const DecompositionEntry& entry, char32_t candidate) { return entry.codepoint < candidate; });
  return result != end && result->codepoint == value ? result : nullptr;
}

void decompose(char32_t value, std::vector<char32_t>& output) {
  constexpr char32_t hangul_s_base = 0xAC00;
  constexpr char32_t hangul_l_base = 0x1100;
  constexpr char32_t hangul_v_base = 0x1161;
  constexpr char32_t hangul_t_base = 0x11A7;
  constexpr int hangul_l_count = 19;
  constexpr int hangul_v_count = 21;
  constexpr int hangul_t_count = 28;
  constexpr int hangul_n_count = hangul_v_count * hangul_t_count;
  constexpr int hangul_s_count = hangul_l_count * hangul_n_count;

  if (value >= hangul_s_base && value < hangul_s_base + hangul_s_count) {
    const int index = static_cast<int>(value - hangul_s_base);
    output.push_back(hangul_l_base + index / hangul_n_count);
    output.push_back(hangul_v_base + (index % hangul_n_count) / hangul_t_count);
    if (index % hangul_t_count != 0) {
      output.push_back(hangul_t_base + index % hangul_t_count);
    }
    return;
  }

  const DecompositionEntry* entry = decomposition(value);
  if (entry == nullptr) {
    output.push_back(value);
    return;
  }
  for (std::size_t index = 0; index < entry->length; ++index) {
    decompose(kCanonicalDecompositionValues[entry->offset + index], output);
  }
}

char32_t compose_pair(char32_t first, char32_t second) noexcept {
  constexpr char32_t hangul_s_base = 0xAC00;
  constexpr char32_t hangul_l_base = 0x1100;
  constexpr char32_t hangul_v_base = 0x1161;
  constexpr char32_t hangul_t_base = 0x11A7;
  constexpr int hangul_l_count = 19;
  constexpr int hangul_v_count = 21;
  constexpr int hangul_t_count = 28;
  constexpr int hangul_n_count = hangul_v_count * hangul_t_count;
  constexpr int hangul_s_count = hangul_l_count * hangul_n_count;

  if (first >= hangul_l_base && first < hangul_l_base + hangul_l_count && second >= hangul_v_base && second < hangul_v_base + hangul_v_count) {
    return hangul_s_base + ((first - hangul_l_base) * hangul_v_count + (second - hangul_v_base)) * hangul_t_count;
  }
  if (first >= hangul_s_base && first < hangul_s_base + hangul_s_count && (first - hangul_s_base) % hangul_t_count == 0 && second > hangul_t_base && second < hangul_t_base + hangul_t_count) {
    return first + (second - hangul_t_base);
  }

  const std::uint64_t key = (static_cast<std::uint64_t>(first) << 21U) | static_cast<std::uint64_t>(second);
  const auto* begin = std::begin(kCanonicalCompositions);
  const auto* end = std::end(kCanonicalCompositions);
  const auto* result = std::lower_bound(begin, end, key, [](const CompositionEntry& entry, std::uint64_t candidate) { return entry.key < candidate; });
  return result != end && result->key == key ? result->codepoint : 0;
}

std::vector<char32_t> normalize_nfc(const std::vector<char32_t>& input) {
  std::vector<char32_t> decomposed;
  decomposed.reserve(input.size());
  for (char32_t value : input) {
    const std::size_t insertion_start = decomposed.size();
    decompose(value, decomposed);
    for (std::size_t index = insertion_start; index < decomposed.size(); ++index) {
      const std::uint8_t current_class = combining_class(decomposed[index]);
      if (current_class == 0) {
        continue;
      }
      std::size_t destination = index;
      while (destination > 0) {
        const std::uint8_t previous_class = combining_class(decomposed[destination - 1]);
        if (previous_class == 0 || previous_class <= current_class) {
          break;
        }
        std::swap(decomposed[destination], decomposed[destination - 1]);
        --destination;
      }
    }
  }

  if (decomposed.empty()) {
    return decomposed;
  }
  std::vector<char32_t> composed;
  composed.reserve(decomposed.size());
  composed.push_back(decomposed.front());
  std::size_t starter_index = 0;
  char32_t starter = decomposed.front();
  std::uint8_t last_class = combining_class(decomposed.front());
  for (std::size_t index = 1; index < decomposed.size(); ++index) {
    const char32_t value = decomposed[index];
    const std::uint8_t current_class = combining_class(value);
    const char32_t composite = compose_pair(starter, value);
    if (composite != 0 && (last_class == 0 || last_class < current_class)) {
      composed[starter_index] = composite;
      starter = composite;
      continue;
    }
    if (current_class == 0) {
      starter_index = composed.size();
      starter = value;
    }
    composed.push_back(value);
    last_class = current_class;
  }
  return composed;
}

bool is_continuation(unsigned char value) noexcept { return value >= 0x80 && value <= 0xBF; }

std::size_t invalid_sequence_length(std::string_view source, std::size_t offset, std::size_t expected) noexcept {
  std::size_t length = 1;
  while (length < expected && offset + length < source.size() && is_continuation(static_cast<unsigned char>(source[offset + length]))) {
    ++length;
  }
  return length;
}

}  // namespace

DecodeResult decode(std::string_view source, std::size_t offset) noexcept {
  if (offset >= source.size()) {
    return {};
  }
  const auto first = static_cast<unsigned char>(source[offset]);
  if (first <= 0x7F) {
    return {first, 1, true};
  }

  std::size_t length = 0;
  char32_t value = 0;
  if (first >= 0xC2 && first <= 0xDF) {
    length = 2;
    value = first & 0x1F;
  } else if (first >= 0xE0 && first <= 0xEF) {
    length = 3;
    value = first & 0x0F;
  } else if (first >= 0xF0 && first <= 0xF4) {
    length = 4;
    value = first & 0x07;
  } else if (first == 0xC0 || first == 0xC1) {
    return {0, invalid_sequence_length(source, offset, 2), false};
  } else if (first >= 0xF5 && first <= 0xF7) {
    return {0, invalid_sequence_length(source, offset, 4), false};
  } else {
    return {0, 1, false};
  }

  if (offset + length > source.size()) {
    return {0, invalid_sequence_length(source, offset, length), false};
  }
  for (std::size_t index = 1; index < length; ++index) {
    const auto byte = static_cast<unsigned char>(source[offset + index]);
    if (!is_continuation(byte)) {
      return {0, index, false};
    }
    value = static_cast<char32_t>((value << 6U) | (byte & 0x3F));
  }

  const auto second = static_cast<unsigned char>(source[offset + 1]);
  if ((first == 0xE0 && second < 0xA0) || (first == 0xED && second > 0x9F) || (first == 0xF0 && second < 0x90) || (first == 0xF4 && second > 0x8F)) {
    return {0, length, false};
  }
  return {value, length, true};
}

bool is_xid_start(char32_t value) noexcept { return contains(kXidStartRanges, value); }

bool is_xid_continue(char32_t value) noexcept { return contains(kXidContinueRanges, value); }

bool is_nfc(std::string_view source) {
  std::vector<char32_t> values;
  for (std::size_t offset = 0; offset < source.size();) {
    const DecodeResult decoded = decode(source, offset);
    if (!decoded.valid) {
      return false;
    }
    values.push_back(decoded.value);
    offset += decoded.length;
  }
  return normalize_nfc(values) == values;
}

bool is_default_ignorable(char32_t value) noexcept {
  static constexpr CodePointRange ranges[] = {{0x00AD, 0x00AD}, {0x034F, 0x034F}, {0x061C, 0x061C}, {0x115F, 0x1160}, {0x17B4, 0x17B5}, {0x180B, 0x180F}, {0x200B, 0x200F}, {0x202A, 0x202E}, {0x2060, 0x206F}, {0x3164, 0x3164}, {0xFE00, 0xFE0F}, {0xFEFF, 0xFEFF}, {0xFFA0, 0xFFA0}, {0xFFF0, 0xFFF8}, {0x1BCA0, 0x1BCAF}, {0x1D173, 0x1D17A}, {0xE0000, 0xE0FFF}};
  return contains(ranges, value);
}

bool is_unicode_whitespace(char32_t value) noexcept {
  static constexpr CodePointRange ranges[] = {{0x0085, 0x0085}, {0x00A0, 0x00A0}, {0x1680, 0x1680}, {0x2000, 0x200A}, {0x2028, 0x2029}, {0x202F, 0x202F}, {0x205F, 0x205F}, {0x3000, 0x3000}, {0xFEFF, 0xFEFF}};
  return contains(ranges, value);
}

void append_utf8(std::string& output, char32_t value) {
  if (value <= 0x7F) {
    output.push_back(static_cast<char>(value));
  } else if (value <= 0x7FF) {
    output.push_back(static_cast<char>(0xC0 | (value >> 6U)));
    output.push_back(static_cast<char>(0x80 | (value & 0x3F)));
  } else if (value <= 0xFFFF) {
    output.push_back(static_cast<char>(0xE0 | (value >> 12U)));
    output.push_back(static_cast<char>(0x80 | ((value >> 6U) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | (value & 0x3F)));
  } else {
    output.push_back(static_cast<char>(0xF0 | (value >> 18U)));
    output.push_back(static_cast<char>(0x80 | ((value >> 12U) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | ((value >> 6U) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | (value & 0x3F)));
  }
}

}  // namespace ink::tokenizer::unicode
