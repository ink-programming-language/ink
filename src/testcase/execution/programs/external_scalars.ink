// Exercise real C integer arguments and returns through the default main entry.
extern "C" func puts(Text: *u8): i32;
extern "C" func abs(Value: i32): i32;
extern "C" func llabs(Value: i64): i64;
extern "C" func atoi(Text: *u8): i32;

func check(Result: bool): bool
{
  if (Result)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  // Signed i32 arguments and returns preserve negative, zero, and positive input.
  puts("External.abs_negative");
  if (!check(abs(-37) == 37)) return 1;

  puts("External.abs_zero");
  if (!check(abs(0) == 0)) return 1;

  puts("External.abs_positive");
  if (!check(abs(91) == 91)) return 1;

  // Feed an external return directly into another C call.
  puts("External.abs_nested");
  if (!check(abs(abs(-19)) == 19)) return 1;

  // Avoid INT_MIN, whose absolute value is not representable in C.
  puts("External.abs_boundary");
  if (!check(abs(-2147483647) == 2147483647)) return 1;

  // The i64 argument and return retain significant bits above the i32 range.
  puts("External.llabs_wide");
  if (!check(llabs(-4294967311) == 4294967311)) return 1;

  // C string conversion returns a signed integer after whitespace and a sign.
  puts("External.atoi_signed");
  if (!check(atoi("  -2048") == -2048)) return 1;

  // A native return can be consumed directly as another native argument.
  puts("External.atoi_nested");
  if (!check(abs(atoi("-73tail")) == 73)) return 1;

  // An empty C string is terminated and produces atoi's defined no-conversion result.
  puts("External.atoi_empty");
  if (!check(atoi("") == 0)) return 1;

  return 0;
}
