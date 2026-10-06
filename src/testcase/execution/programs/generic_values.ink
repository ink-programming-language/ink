import "C" func puts(Text: *u8): i32;

func add[T: type = i32, N: T = 2](Value: T): T
{
  return Value + N;
}

func value[N: i32](): i32
{
  return N;
}

func ordered[A: i32, B: i32](): bool
{
  return A == 2 && B == 1;
}

func selected[B: bool](): i32
{
  comptime if (B)
  {
    return 21;
  }
  else
  {
    return InvalidDiscardedBranch;
  }
}

comptime func sum[T: type](Left: T, Right: T): T
{
  return Left + Right;
}

func main(): i32
{
  puts("Generic.defaults");
  if (add::[](18) + add::[N = 2, T = i32](20) != 42)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.frozen_values");
  comptime var Counter: i32 = 20;
  var Before = value::[Counter]();
  comptime { Counter = 22; }
  if (Before + value::[Counter]() != 42 || value::[20]() != Before)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.argument_order");
  comptime var Order: i32 = 0;
  if (!ordered::[B = ++Order, A = ++Order]())
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.comptime");
  if (comptime(sum::[i32](selected::[true](), selected::[true]())) != 42)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");
  return 0;
}
