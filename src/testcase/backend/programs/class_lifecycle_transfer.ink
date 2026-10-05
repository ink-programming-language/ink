class Item
{
  field Count: *i32;
  field Weight: i32;

  func __init__(Count: *i32, Weight: i32): void
  {
    this.Count = Count;
    this.Weight = Weight;
  }

  func __del__(): void
  {
    *this.Count = *this.Count + this.Weight;
  }

  func ready(): bool
  {
    return true;
  }
};

class Pair
{
  field Items: [Item; 2];

  func __init__(Count: *i32): void
  {
    this.Items = [Item(Count, 1), Item(Count, 2)];
  }
};

func make(Count: *i32): Item
{
  return Item(Count, 3);
}

func discard(Count: *i32, Enabled: bool): void
{
  Enabled && Item(Count, 5).ready();
}

func main(): i32
{
  var Count = 0;
  {
    var A = make(&Count);
    A = make(&Count);
    var B = Pair(&Count);
  }
  if (Count != 9)
  {
    return 1;
  }
  discard(&Count, false);
  discard(&Count, true);
  for (var I = 0; I < 3; I++)
  {
    I == 1 && Item(&Count, 7).ready();
  }
  if (Count != 21)
  {
    return 2;
  }
  return 42;
}
