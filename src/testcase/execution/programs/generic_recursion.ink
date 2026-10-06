import "C" func puts(Text: *u8): i32;

func left[T: type](Count: i32, Value: T): T
{
  if (Count == 0)
  {
    return Value;
  }
  return right::[T](Count + -1, Value);
}

func right[T: type](Count: i32, Value: T): T
{
  return left::[T](Count, Value);
}

func choose[T: type](Value: i32): i32
{
  return Value;
}

func choose[T: type](Value: bool): i32
{
  return InvalidUnselectedOverload;
}

func choose(Value: i32): i32
{
  return Value + 1;
}

func nested[T: type](Value: T): T
{
  func inner[U: type](Item: U): U
  {
    return Item;
  }
  return inner::[T](Value);
}

func unused[T: type](): void
{
  UnknownUnusedFunction();
}

func main(): i32
{
  puts("Generic.recursion");
  if (left::[i32](5, 42) != 42 || comptime(left::[i32](3, 42)) != 42)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.overloads");
  if (choose(20) + choose::[bool](21) != 42)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.nested");
  if (nested::[i32](42) != 42 || !nested::[bool](true))
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");
  return 0;
}
