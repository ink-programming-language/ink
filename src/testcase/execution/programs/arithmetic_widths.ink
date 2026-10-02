// Runtime addition preserves each signed or unsigned integer width, including wraparound.
extern "C" func puts(Text: *u8): i32;

// Signed eight-bit addition covers both wrap boundaries and cancellation.
func signedI8(): bool
{
  var Minimum: i8 = -128;
  var Maximum: i8 = 127;
  var One: i8 = 1;
  return Maximum + One == Minimum && Minimum + -1 == Maximum && Minimum + Maximum == -1 && One + -1 == 0;
}

// Unsigned eight-bit addition retains the high bit and wraps at 256.
func unsignedU8(): bool
{
  var Maximum: u8 = 255;
  var High: u8 = 128;
  return Maximum + 1 == 0 && High + High == 0 && High + 127 == Maximum && Maximum + 0 == Maximum;
}

// Signed sixteen-bit addition carries across the low byte and wraps at the sign bit.
func signedI16(): bool
{
  var Minimum: i16 = -32768;
  var Maximum: i16 = 32767;
  var Low: i16 = 255;
  return Maximum + 1 == Minimum && Minimum + -1 == Maximum && Low + 1 == 256 && Minimum + Maximum == -1;
}

// Unsigned sixteen-bit addition carries across the low byte and wraps at 65536.
func unsignedU16(): bool
{
  var Maximum: u16 = 65535;
  var High: u16 = 32768;
  var Low: u16 = 255;
  return Maximum + 1 == 0 && High + High == 0 && Low + 1 == 256 && High + 32767 == Maximum;
}

// Signed thirty-two-bit addition preserves negative values and both wrap boundaries.
func signedI32(): bool
{
  var Minimum: i32 = -2147483648;
  var Maximum: i32 = 2147483647;
  var Negative: i32 = -123456;
  return Maximum + 1 == Minimum && Minimum + -1 == Maximum && Negative + 123456 == 0 && Minimum + Maximum == -1;
}

// Unsigned thirty-two-bit addition must not interpret the high bit as a sign.
func unsignedU32(): bool
{
  var Maximum: u32 = 4294967295;
  var High: u32 = 2147483648;
  return Maximum + 1 == 0 && High + High == 0 && High + 2147483647 == Maximum && Maximum + 0 == Maximum;
}

// Signed sixty-four-bit addition carries beyond thirty-two bits and wraps at the sign bit.
func signedI64(): bool
{
  var Minimum: i64 = -9223372036854775808;
  var Maximum: i64 = 9223372036854775807;
  var Low: i64 = 4294967295;
  return Maximum + 1 == Minimum && Minimum + -1 == Maximum && Low + 1 == 4294967296 && Minimum + Maximum == -1;
}

// Unsigned sixty-four-bit addition preserves all host-word bits and wraps at the width.
func unsignedU64(): bool
{
  var Maximum: u64 = 18446744073709551615;
  var High: u64 = 9223372036854775808;
  return Maximum + 1 == 0 && High + High == 0 && High + 9223372036854775807 == Maximum && Maximum + 0 == Maximum;
}

// Signed 128-bit addition propagates carry into the upper word and handles both extrema.
func signedI128(): bool
{
  var Minimum: i128 = -170141183460469231731687303715884105728;
  var Maximum: i128 = 170141183460469231731687303715884105727;
  var Low: i128 = 18446744073709551615;
  return Maximum + 1 == Minimum && Minimum + -1 == Maximum && Low + 1 == 18446744073709551616 && Minimum + Maximum == -1;
}

// Unsigned 128-bit addition propagates carry across words and discards carry beyond the width.
func unsignedU128(): bool
{
  var Maximum: u128 = 340282366920938463463374607431768211455;
  var High: u128 = 170141183460469231731687303715884105728;
  var Low: u128 = 18446744073709551615;
  return Maximum + 1 == 0 && High + High == 0 && Low + 1 == 18446744073709551616 && High + 170141183460469231731687303715884105727 == Maximum;
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
  puts("Arithmetic.signed_i8");
  if (!check(signedI8())) return 1;

  puts("Arithmetic.unsigned_u8");
  if (!check(unsignedU8())) return 1;

  puts("Arithmetic.signed_i16");
  if (!check(signedI16())) return 1;

  puts("Arithmetic.unsigned_u16");
  if (!check(unsignedU16())) return 1;

  puts("Arithmetic.signed_i32");
  if (!check(signedI32())) return 1;

  puts("Arithmetic.unsigned_u32");
  if (!check(unsignedU32())) return 1;

  puts("Arithmetic.signed_i64");
  if (!check(signedI64())) return 1;

  puts("Arithmetic.unsigned_u64");
  if (!check(unsignedU64())) return 1;

  puts("Arithmetic.signed_i128");
  if (!check(signedI128())) return 1;

  puts("Arithmetic.unsigned_u128");
  if (!check(unsignedU128())) return 1;

  return 0;
}
