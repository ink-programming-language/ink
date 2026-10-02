// Subtraction, multiplication, division, remainder and bitwise operators currently execute in comptime.
extern "C" func puts(Text: *u8): i32;

// Compound assignments feed the updated value into each subsequent arithmetic operation.
func compoundArithmetic(): i32
{
  comptime var Value: i32 = 10;
  comptime
  {
    Value += 7;
    Value -= 2;
    Value *= 4;
    Value /= 3;
    Value %= 6;
  }
  return comptime Value;
}

// Compound bitwise operations and shifts preserve the same variable throughout the chain.
func compoundBitwise(): i32
{
  comptime var Value: i32 = 5;
  comptime
  {
    Value <<= 3;
    Value |= 3;
    Value ^= 9;
    Value &= 62;
    Value >>= 1;
  }
  return comptime Value;
}

// Prefix updates expose the new value while postfix updates expose the previous value.
func updateValues(): bool
{
  comptime var Value: i32 = 4;
  comptime var BeforeIncrement: i32 = Value++;
  comptime var AfterIncrement: i32 = ++Value;
  comptime var BeforeDecrement: i32 = Value--;
  comptime var AfterDecrement: i32 = --Value;
  return comptime (BeforeIncrement == 4 && AfterIncrement == 6 && BeforeDecrement == 6 && AfterDecrement == 4 && Value == 4);
}

// Eight-bit unary negation and inversion preserve the selected width at its signed extrema.
func signedByte(): bool
{
  comptime var Minimum: i8 = -128;
  comptime var Maximum: i8 = 127;
  return comptime (-Minimum == Minimum && ~Minimum == Maximum && Minimum - 1 == Maximum && Maximum * 2 == -2);
}

// Unsigned right shift fills with zeros, while inversion and subtraction wrap within one byte.
func unsignedByte(): bool
{
  comptime var High: u8 = 128;
  comptime var Zero: u8 = 0;
  return comptime (High >> 7 == 1 && ~Zero == 255 && Zero - 1 == 255 && High * 2 == Zero);
}

// Signed right shift propagates the sign bit for negative operands.
func signedShift(): bool
{
  comptime var Negative: i32 = -16;
  comptime var Minimum: i32 = -2147483648;
  return comptime (Negative >> 2 == -4 && Minimum >> 31 == -1 && Negative >> 0 == Negative);
}

// Wide multiplication and division use both words of a 128-bit integer.
func wideArithmetic(): bool
{
  comptime var High: u128 = 18446744073709551616;
  comptime var Product: u128 = High * 3;
  return comptime (Product == 55340232221128654848 && Product / 3 == High && (Product + 2) % 3 == 2 && (High << 63) >> 63 == High);
}

// A compile-time initializer is frozen into runtime storage, then participates in runtime addition.
func runtimeInitializer(): i32
{
  var Value: i32 = comptime (6 * 7);
  Value = Value + 1;
  return Value;
}

func check(Actual: bool): bool
{
  if (Actual)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  // Addition, subtraction and multiplication compose according to arithmetic precedence.
  puts("ComptimeArithmetic.precedence");
  if (!check(comptime (2 + 3 * 4 - 5 == 9))) return 1;

  // Parentheses change which sum is multiplied.
  puts("ComptimeArithmetic.parentheses");
  if (!check(comptime ((2 + 3) * (9 - 5) == 20))) return 1;

  // Subtraction associates left-to-right unless explicitly grouped.
  puts("ComptimeArithmetic.subtraction_association");
  if (!check(comptime (20 - 7 - 3 == 10 && 20 - (7 - 3) == 16))) return 1;

  // Division associates left-to-right and discards the fractional remainder.
  puts("ComptimeArithmetic.division_association");
  if (!check(comptime (100 / 5 / 2 == 10 && 100 / (5 / 2) == 50 && 17 / 5 == 3))) return 1;

  // Signed division truncates toward zero for every combination of operand signs.
  puts("ComptimeArithmetic.signed_division");
  if (!check(comptime (-17 / 5 == -3 && 17 / -5 == -3 && -17 / -5 == 3))) return 1;

  // The remainder has the dividend's sign, independently of the divisor's sign.
  puts("ComptimeArithmetic.signed_remainder");
  if (!check(comptime (-17 % 5 == -2 && 17 % -5 == 2 && -17 % -5 == -2 && 20 % 5 == 0))) return 1;

  // Multiplying by zero or by a negative value produces the expected signed result.
  puts("ComptimeArithmetic.multiply");
  if (!check(comptime (123 * 0 == 0 && -7 * 6 == -42 && -7 * -6 == 42))) return 1;

  // Unary plus, unary minus and complement bind before binary arithmetic.
  puts("ComptimeArithmetic.unary");
  if (!check(comptime (-(2 + 3) == -5 && +(2 + 3) == 5 && ~0 == -1 && ~5 == -6))) return 1;

  // AND preserves only bits shared by both operands.
  puts("ComptimeArithmetic.bitwise_and");
  if (!check(comptime ((12 & 10) == 8 && (12 & 0) == 0))) return 1;

  // OR preserves bits present in either operand.
  puts("ComptimeArithmetic.bitwise_or");
  if (!check(comptime ((12 | 10) == 14 && (12 | 0) == 12))) return 1;

  // XOR clears shared bits and preserves differing bits.
  puts("ComptimeArithmetic.bitwise_xor");
  if (!check(comptime ((12 ^ 10) == 6 && (12 ^ 12) == 0))) return 1;

  // AND binds more tightly than XOR, which binds more tightly than OR.
  puts("ComptimeArithmetic.bitwise_precedence");
  if (!check(comptime ((8 | 6 ^ 3 & 1) == 15))) return 1;

  // Addition binds more tightly than shifts; zero and maximum valid shifts are accepted.
  puts("ComptimeArithmetic.shift_boundaries");
  if (!check(comptime ((1 << 2 + 1) == 8 && (7 << 0) == 7 && (1 << 31) == -2147483648 && (16 >> 2) == 4))) return 1;

  puts("ComptimeArithmetic.compound_arithmetic");
  if (!check(comptime (compoundArithmetic() == 2))) return 1;

  puts("ComptimeArithmetic.compound_bitwise");
  if (!check(comptime (compoundBitwise() == 17))) return 1;

  puts("ComptimeArithmetic.update_values");
  if (!check(comptime updateValues())) return 1;

  puts("ComptimeArithmetic.signed_byte");
  if (!check(comptime signedByte())) return 1;

  puts("ComptimeArithmetic.unsigned_byte");
  if (!check(comptime unsignedByte())) return 1;

  puts("ComptimeArithmetic.signed_shift");
  if (!check(comptime signedShift())) return 1;

  puts("ComptimeArithmetic.wide_arithmetic");
  if (!check(comptime wideArithmetic())) return 1;

  puts("ComptimeArithmetic.runtime_initializer");
  if (!check(runtimeInitializer() == 43)) return 1;

  return 0;
}
