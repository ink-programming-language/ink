class Box
{
  field Value: i32;

  func __init__(InitialValue: i32): void
  {
    this.Value = InitialValue;
  }
};

func index(): i32
{
  return 2;
}

func main(): i32
{
  var Objects = [Box(42)];
  return Objects[index()].Value;
}
