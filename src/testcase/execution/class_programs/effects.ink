// The receiver precedes explicit arguments and is evaluated exactly once.
import "C" func puts(Text: *u8): i32;

class Counter
{
  field Value: i32;

  func add(N: i32): i32
  {
    puts("method");
    this.Value = this.Value + N;
    return this.Value;
  }

  func __init__(InitialValue: i32): void
  {
    this.Value = InitialValue;
  }
};

func receiver(Count: *i32, Value: *Counter): *Counter
{
  puts("receiver");
  *Count = *Count + 1;
  return Value;
}

func argument(Count: *i32): i32
{
  puts("argument");
  *Count = *Count + 1;
  return *Count;
}

func main(): i32
{
  puts("Class.method_order");
  var Count = 0;
  var Value = Counter(38);
  const Result = receiver(&Count, &Value)->add(argument(&Count));
  if (Result + Count != 42 || Value.Value != 40)
  {
    return 1;
  }
  puts("PASS");
  return 0;
}
