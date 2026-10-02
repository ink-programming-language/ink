// Run every case from main and assert its result before reporting PASS on stdout.
extern "C" func puts(Text: *u8): i32;

func bit(Value: bool, Weight: i32): i32
{
  if (Value) return Weight;
  return 0;
}

func equal(Left: i32, Right: i32): bool
{
  return Left == Right;
}

func check_equal(): i32
{
  return bit(equal(3, 5), 1) + bit(equal(5, 5), 2) + bit(equal(8, 5), 4);
}

func not_equal(Left: i32, Right: i32): bool
{
  return Left != Right;
}

func check_not_equal(): i32
{
  return bit(not_equal(3, 5), 1) + bit(not_equal(5, 5), 2) + bit(not_equal(8, 5), 4);
}

func less(Left: i32, Right: i32): bool
{
  return Left < Right;
}

func check_less(): i32
{
  return bit(less(3, 5), 1) + bit(less(5, 5), 2) + bit(less(8, 5), 4);
}

func less_equal(Left: i32, Right: i32): bool
{
  return Left <= Right;
}

func check_less_equal(): i32
{
  return bit(less_equal(3, 5), 1) + bit(less_equal(5, 5), 2) + bit(less_equal(8, 5), 4);
}

func greater(Left: i32, Right: i32): bool
{
  return Left > Right;
}

func check_greater(): i32
{
  return bit(greater(3, 5), 1) + bit(greater(5, 5), 2) + bit(greater(8, 5), 4);
}

func greater_equal(Left: i32, Right: i32): bool
{
  return Left >= Right;
}

func check_greater_equal(): i32
{
  return bit(greater_equal(3, 5), 1) + bit(greater_equal(5, 5), 2) + bit(greater_equal(8, 5), 4);
}

func signed_i8(): i32
{
  var Minimum: i8 = -128;
  var Maximum: i8 = 127;
  var Zero: i8 = 0;
  return bit(Minimum < Zero, 1) + bit(Maximum > Zero, 2) + bit(Minimum < Maximum, 4) + bit(-128 == Minimum, 8) + bit(127 >= Maximum, 16);
}

func unsigned_u8(): i32
{
  var Maximum: u8 = 255;
  var HighBit: u8 = 128;
  var Zero: u8 = 0;
  return bit(Maximum > HighBit, 1) + bit(HighBit > Zero, 2) + bit(Maximum >= 255, 4) + bit(0 < HighBit, 8) + bit(Maximum != Zero, 16);
}

func signed_i128(): i32
{
  var Minimum: i128 = -170141183460469231731687303715884105728;
  var Maximum: i128 = 170141183460469231731687303715884105727;
  var Negative: i128 = -18446744073709551616;
  var Zero: i128 = 0;
  return bit(Minimum < Negative, 1) + bit(Negative < Zero, 2) + bit(Zero < Maximum, 4) + bit(Minimum != Maximum, 8) + bit(Minimum <= Minimum, 16) + bit(Maximum >= Maximum, 32);
}

func unsigned_u128(): i32
{
  var Maximum: u128 = 340282366920938463463374607431768211455;
  var High: u128 = 18446744073709551616;
  var Low: u128 = 18446744073709551615;
  return bit(Maximum > High, 1) + bit(High > Low, 2) + bit(Low < Maximum, 4) + bit(High != Low, 8) + bit(High == 18446744073709551616, 16) + bit(18446744073709551615 <= Low, 32);
}

func literal_comparisons(): i32
{
  return bit(1 < 2, 1) + bit(-2 < -1, 2) + bit(0 == -0, 4) + bit(3 + 4 == 7, 8) + bit(7 >= 3 + 4, 16);
}

func check(Actual: i32, Expected: i32): bool
{
  if (Actual == Expected)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  puts("Compare.equal");
  if (!check(check_equal(), 2)) return 1;

  puts("Compare.not_equal");
  if (!check(check_not_equal(), 5)) return 1;

  puts("Compare.less");
  if (!check(check_less(), 1)) return 1;

  puts("Compare.less_equal");
  if (!check(check_less_equal(), 3)) return 1;

  puts("Compare.greater");
  if (!check(check_greater(), 4)) return 1;

  puts("Compare.greater_equal");
  if (!check(check_greater_equal(), 6)) return 1;

  puts("Compare.signed_i8");
  if (!check(signed_i8(), 31)) return 1;

  puts("Compare.unsigned_u8");
  if (!check(unsigned_u8(), 31)) return 1;

  puts("Compare.signed_i128");
  if (!check(signed_i128(), 63)) return 1;

  puts("Compare.unsigned_u128");
  if (!check(unsigned_u128(), 63)) return 1;

  puts("Compare.literal_comparisons");
  if (!check(literal_comparisons(), 31)) return 1;

  return 0;
}
