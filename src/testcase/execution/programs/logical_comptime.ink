// Run every case from main and assert its result before reporting PASS on stdout.
extern "C" func puts(Text: *u8): i32;

func missing(): bool;

comptime const Minimum: i8 = -128;

func ordinary(Left: i32, Right: i32): bool
{
  return Left < Right && Left != Right || Left == Right;
}

comptime func logical(Left: bool, Right: bool): bool
{
  return !Left || (Left && Right);
}

comptime func compare(Left: i32, Right: i32): bool
{
  return Left < Right && Left <= Right && Right > Left && Right >= Left && Left != Right && !(Left == Right);
}

comptime func skip(Left: bool): bool
{
  return Left || missing();
}

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func logical_true(): i32
{
  return encode(comptime logical(false, false));
}

func logical_false(): i32
{
  return encode(comptime logical(true, false));
}

func comparisons(): i32
{
  return encode(comptime compare(-3, 5));
}

func ordinary_call(): i32
{
  return encode(comptime ordinary(3, 5));
}

func skip_call(): i32
{
  return encode(comptime skip(true));
}

func direct_expression(): i32
{
  return encode(comptime (1 + 2 < 4 && !(5 == 6) && -(128) == Minimum || false));
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
  puts("Comptime.logical_true");
  if (!check(logical_true(), 17)) return 1;

  puts("Comptime.logical_false");
  if (!check(logical_false(), 23)) return 1;

  puts("Comptime.comparisons");
  if (!check(comparisons(), 17)) return 1;

  puts("Comptime.ordinary_call");
  if (!check(ordinary_call(), 17)) return 1;

  puts("Comptime.skip_call");
  if (!check(skip_call(), 17)) return 1;

  puts("Comptime.direct_expression");
  if (!check(direct_expression(), 17)) return 1;

  return 0;
}
