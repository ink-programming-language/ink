class Counter
{
  field Value: i32;
  field Step: i32 = 2;

  func advance(): i32
  {
    this.Value = this.Value + this.Step;
    return this.Value;
  }
};

func copy(Value: Counter): Counter
{
  var Local = Value;
  Local.advance();
  return Local;
}

func main(): i32
{
  var Original = Counter(38);
  var Changed = copy(Original);
  if (Original.Value != 38)
  {
    return 10;
  }
  if (Changed.Value != 40)
  {
    return 11;
  }
  return Changed.advance();
}
