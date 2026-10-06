import "C" func puts(Text: *u8): i32;

func identity[T: type](Value: T): T
{
  return Value;
}

func first[T: type, N: u64](Values: [T; N]): T
{
  return Values[0];
}

func read[T: type](Address: *T): T
{
  return *Address;
}

class Box
{
  field Value: i32;
  func __init__(Initial: i32): void
  {
    this.Value = Initial;
  }
};

func main(): i32
{
  puts("Generic.types");
  var Small: i8 = identity::[i8](-128);
  var Wide: u128 = identity::[u128](340282366920938463463374607431768211455);
  if (Small != -128 || Wide != 340282366920938463463374607431768211455 || !identity::[bool](true))
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.arrays_and_pointers");
  var Value: i32 = 20;
  if (read::[i32](&Value) + first::[i32, 2]([22, 0]) != 42)
  {
    puts("FAIL");
    return 1;
  }
  var Empty = identity::[[i32; 0]]([]);
  puts("PASS");

  puts("Generic.class_copy");
  var Original = Box(42);
  var Copy = identity::[Box](Original);
  Original.Value = 1;
  if (Copy.Value != 42)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");

  puts("Generic.function_value");
  var Callback = identity::[i32];
  var BooleanCallback = identity::[bool];
  var ClassCallback = identity::[Box];
  var ArrayCallback = identity::[[i32; 2]];
  var PointerCallback = read::[i32];
  var CallbackCopy = ClassCallback(Original);
  var ArrayCopy = ArrayCallback([20, 22]);
  if (Callback(42) != 42 || BooleanCallback(false) || CallbackCopy.Value != 1 || ArrayCopy[0] + ArrayCopy[1] != 42 || PointerCallback(&Value) != 20)
  {
    puts("FAIL");
    return 1;
  }
  puts("PASS");
  return 0;
}
