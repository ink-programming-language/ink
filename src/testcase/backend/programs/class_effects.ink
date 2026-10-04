import "C" func puts(Text: *u8): i32;

func initial(): i32
{
  puts("default");
  return 2;
}

class Counter
{
  field Value: i32;
  field Step: i32 = initial();

  func add(Amount: i32): i32
  {
    puts("method");
    this.Value = this.Value + Amount;
    return this.Value;
  }

  func __init__(InitialValue: i32): void
  {
    this.Value = InitialValue;
  }

  func __init__(InitialValue: i32, InitialStep: i32): void
  {
    this.Value = InitialValue;
    this.Step = InitialStep;
  }
};

func receiver(): Counter
{
  puts("receiver");
  return Counter(38);
}

func argument(): i32
{
  puts("argument");
  return 2;
}

func main(): i32
{
  var First = Counter(1);
  var Second = Counter(2);
  var Target = receiver();
  return Target.add(argument()) + 2;
}
