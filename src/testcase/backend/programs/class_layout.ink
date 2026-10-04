class Empty
{
};

class Wide
{
  field Tag: u8;
  field Bits: u128;
  field Marker: Empty;
  field Tail: i32;

  func __init__(InitialTag: u8, InitialBits: u128, InitialMarker: Empty, InitialTail: i32): void
  {
    this.Tag = InitialTag;
    this.Bits = InitialBits;
    this.Marker = InitialMarker;
    this.Tail = InitialTail;
  }
};

func main(): i32
{
  var EmptyArray: [Empty; 0] = [];
  var Original = Wide(7, 340282366920938463463374607431768211455, Empty(), 40);
  var Saved = &Original.Tail;
  Original.Bits = Original.Bits + 1;
  var Copy = Original;
  Copy.Tail = 0;
  if (Original.Bits != 0 || Original.Tag != 7)
  {
    return 10;
  }
  *Saved = *Saved + 2;
  return Original.Tail;
}
