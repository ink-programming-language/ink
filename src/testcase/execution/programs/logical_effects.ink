// Run every case from main and assert its result before reporting PASS on stdout.
import "C" func puts(Text: *u8): i32;

func left(Value: bool): bool
{
  puts("left");
  return Value;
}

func right(Value: bool): bool
{
  puts("right");
  return Value;
}

func tail(Value: bool): bool
{
  puts("tail");
  return Value;
}

func first(Value: i32): i32
{
  puts("first");
  return Value;
}

func second(Value: i32): i32
{
  puts("second");
  return Value;
}

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func skip_and(): i32
{
  return encode(left(false) && right(true));
}

func skip_or(): i32
{
  return encode(left(true) || right(false));
}

func reach_and(): i32
{
  return encode(left(true) && right(false));
}

func reach_or(): i32
{
  return encode(left(false) || right(true));
}

func nested(): i32
{
  var Value: bool = (left(false) || right(true)) && tail(true);
  return encode(Value == Value);
}

func reuse_result(): i32
{
  var Value: bool = left(true) && right(true);
  return encode(Value && Value && Value);
}

func compare_order(): i32
{
  return encode(first(3) < second(5));
}

func not_once(): i32
{
  return encode(!left(false));
}

func precedence_skip(): i32
{
  return encode(left(true) || right(true) && tail(false));
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
  puts("Effects.skip_and");
  if (!check(skip_and(), 23)) return 1;

  puts("Effects.skip_or");
  if (!check(skip_or(), 17)) return 1;

  puts("Effects.reach_and");
  if (!check(reach_and(), 23)) return 1;

  puts("Effects.reach_or");
  if (!check(reach_or(), 17)) return 1;

  puts("Effects.nested");
  if (!check(nested(), 17)) return 1;

  puts("Effects.reuse_result");
  if (!check(reuse_result(), 17)) return 1;

  puts("Effects.compare_order");
  if (!check(compare_order(), 17)) return 1;

  puts("Effects.not_once");
  if (!check(not_once(), 17)) return 1;

  puts("Effects.precedence_skip");
  if (!check(precedence_skip(), 17)) return 1;

  return 0;
}
