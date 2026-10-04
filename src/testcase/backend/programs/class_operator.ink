import "C" func puts(Text: *u8): i32;

class Number
{
  field Value: i32;

  func __add__(Other: Number): Number
  {
    puts("add");
    this.Value = this.Value + Other.Value;
    return Number(this.Value);
  }

  func __eq__(Other: Number): bool
  {
    puts("equal");
    return this.Value == Other.Value;
  }

  func __neg__(): i32
  {
    puts("negative");
    return this.Value;
  }
};

func left(Value: *Number): *Number
{
  puts("left");
  return Value;
}

func right(Value: *Number): Number
{
  puts("right");
  Value->Value = 7;
  return Number(22);
}

func main(): i32
{
  var Original = Number(20);
  const Result = *left(&Original) + right(&Original);
  if (Original.Value != 7)
  {
    return 10;
  }
  const Fixed = Number(5);
  const FixedResult = Fixed + Number(1);
  if (Fixed.Value != 5 || FixedResult.Value != 6)
  {
    return 12;
  }
  if (Result == Number(42))
  {
    return -Result;
  }
  return 11;
}
