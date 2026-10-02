// Run every case from main and assert its result before reporting PASS on stdout.
extern "C" func puts(Text: *u8): i32;

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func logicalNot(Value: bool): bool
{
  return !Value;
}

func logicalAnd(Left: bool, Right: bool): bool
{
  return Left && Right;
}

func logicalOr(Left: bool, Right: bool): bool
{
  return Left || Right;
}

func equal(Left: bool, Right: bool): bool
{
  return Left == Right;
}

func notEqual(Left: bool, Right: bool): bool
{
  return Left != Right;
}

func not_false(): i32
{
  return encode(logicalNot(false));
}

func not_true(): i32
{
  return encode(logicalNot(true));
}

func logicalAnd_false_false(): i32
{
  return encode(logicalAnd(false, false));
}

func logicalAnd_false_true(): i32
{
  return encode(logicalAnd(false, true));
}

func logicalAnd_true_false(): i32
{
  return encode(logicalAnd(true, false));
}

func logicalAnd_true_true(): i32
{
  return encode(logicalAnd(true, true));
}

func logicalOr_false_false(): i32
{
  return encode(logicalOr(false, false));
}

func logicalOr_false_true(): i32
{
  return encode(logicalOr(false, true));
}

func logicalOr_true_false(): i32
{
  return encode(logicalOr(true, false));
}

func logicalOr_true_true(): i32
{
  return encode(logicalOr(true, true));
}

func equal_false_false(): i32
{
  return encode(equal(false, false));
}

func equal_false_true(): i32
{
  return encode(equal(false, true));
}

func equal_true_false(): i32
{
  return encode(equal(true, false));
}

func equal_true_true(): i32
{
  return encode(equal(true, true));
}

func notEqual_false_false(): i32
{
  return encode(notEqual(false, false));
}

func notEqual_false_true(): i32
{
  return encode(notEqual(false, true));
}

func notEqual_true_false(): i32
{
  return encode(notEqual(true, false));
}

func notEqual_true_true(): i32
{
  return encode(notEqual(true, true));
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
  puts("Not.false");
  if (!check(not_false(), 17)) return 1;

  puts("Not.true");
  if (!check(not_true(), 23)) return 1;

  puts("And.false.false");
  if (!check(logicalAnd_false_false(), 23)) return 1;

  puts("And.false.true");
  if (!check(logicalAnd_false_true(), 23)) return 1;

  puts("And.true.false");
  if (!check(logicalAnd_true_false(), 23)) return 1;

  puts("And.true.true");
  if (!check(logicalAnd_true_true(), 17)) return 1;

  puts("Or.false.false");
  if (!check(logicalOr_false_false(), 23)) return 1;

  puts("Or.false.true");
  if (!check(logicalOr_false_true(), 17)) return 1;

  puts("Or.true.false");
  if (!check(logicalOr_true_false(), 17)) return 1;

  puts("Or.true.true");
  if (!check(logicalOr_true_true(), 17)) return 1;

  puts("EqualBool.false.false");
  if (!check(equal_false_false(), 17)) return 1;

  puts("EqualBool.false.true");
  if (!check(equal_false_true(), 23)) return 1;

  puts("EqualBool.true.false");
  if (!check(equal_true_false(), 23)) return 1;

  puts("EqualBool.true.true");
  if (!check(equal_true_true(), 17)) return 1;

  puts("NotEqualBool.false.false");
  if (!check(notEqual_false_false(), 23)) return 1;

  puts("NotEqualBool.false.true");
  if (!check(notEqual_false_true(), 17)) return 1;

  puts("NotEqualBool.true.false");
  if (!check(notEqual_true_false(), 17)) return 1;

  puts("NotEqualBool.true.true");
  if (!check(notEqual_true_true(), 23)) return 1;

  return 0;
}
