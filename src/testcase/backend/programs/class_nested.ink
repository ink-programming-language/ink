class Pair
{
  field Left: i32;
  field Right: i32;

  func __init__(InitialLeft: i32, InitialRight: i32): void
  {
    this.Left = InitialLeft;
    this.Right = InitialRight;
  }
};

class Box
{
  field Tag: u8;
  field Value: Pair;

  func __init__(InitialTag: u8, InitialValue: Pair): void
  {
    this.Tag = InitialTag;
    this.Value = InitialValue;
  }
};

func main(): i32
{
  var Data = [Box(1, Pair(10, 20)), Box(2, Pair(30, 40))];
  var Alias = &Data[1].Value.Left;
  Data[1] = Box(3, Pair(39, 1));
  *Alias = *Alias + 2;
  var Copy = Data;
  Copy[1].Value.Left = 0;
  return Data[1].Value.Left + Data[1].Value.Right;
}
