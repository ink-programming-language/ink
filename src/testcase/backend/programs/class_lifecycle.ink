import "C" func puts(Text: *u8): i32;

class Item
{
  field Id: i32;

  func __init__(Id: i32): void
  {
    this.Id = Id;
    puts("init");
  }

  func __del__(): void
  {
    if (this.Id == 1)
    {
      puts("drop1");
    }
    else
    {
      puts("drop2");
    }
  }

  func ready(): bool
  {
    return true;
  }
};

class Pair
{
  field First: Item = Item(1);
  field Second: Item = Item(2);

  func __del__(): void
  {
    puts("pair");
  }
};

func early(): void
{
  var First = Item(1);
  if (true)
  {
    const Second = Item(2);
    return;
  }
}

func main(): i32
{
  {
    var Objects = Pair();
  }
  early();
  false && Item(1).ready();
  true && Item(2).ready();
  {
    var Items = [Item(1), Item(2)];
  }
  return 42;
}
