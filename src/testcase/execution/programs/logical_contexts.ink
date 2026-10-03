// Run every case from main and assert its result before reporting PASS on stdout.
import "C" func puts(Text: *u8): i32;

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func between(Value: i32, Lower: i32, Upper: i32): bool
{
  return Lower <= Value && Value < Upper;
}

func precedence(): i32
{
  return encode(true || false && false);
}

func parentheses(): i32
{
  return encode((true || false) && false);
}

func repeated_not(): i32
{
  return encode(!!!false);
}

func bool_comparison_precedence(): i32
{
  return encode(false == !true && true != false || false);
}

func local_assignment(): i32
{
  var Result: bool = 1 + 2 < 4;
  Result = Result && between(5, 1, 8);
  Result = !Result || 7 >= 8;
  return encode(Result);
}

func nested_condition(): i32
{
  var Value: i32 = 7;
  if (between(Value, 1, 8) && !(Value == 3 || Value == 5))
  {
    if (Value > 6 || Value < 0) return 31;
    return 32;
  }
  else if (Value == 8 && Value != 9)
  {
    return 33;
  }
  return 34;
}

func call_arguments(): i32
{
  return encode(between(4 + 1, 1 + 1, 3 + 3) == (!false && true));
}

func nested_merge(): i32
{
  var Result: bool = (false || true) && (false || (true && true));
  if (Result)
  {
    Result = (true && false) || (false || true);
  }
  return encode(Result);
}

func inferred_local(): i32
{
  var First = 1 < 2;
  var Second = !First;
  var Third = First && !Second;
  return encode(Third);
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
  puts("Context.precedence");
  if (!check(precedence(), 17)) return 1;

  puts("Context.parentheses");
  if (!check(parentheses(), 23)) return 1;

  puts("Context.repeated_not");
  if (!check(repeated_not(), 17)) return 1;

  puts("Context.bool_comparison_precedence");
  if (!check(bool_comparison_precedence(), 17)) return 1;

  puts("Context.local_assignment");
  if (!check(local_assignment(), 23)) return 1;

  puts("Context.nested_condition");
  if (!check(nested_condition(), 31)) return 1;

  puts("Context.call_arguments");
  if (!check(call_arguments(), 17)) return 1;

  puts("Context.nested_merge");
  if (!check(nested_merge(), 17)) return 1;

  puts("Context.inferred_local");
  if (!check(inferred_local(), 17)) return 1;

  return 0;
}
