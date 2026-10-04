class Packet
{
  field Tag: u8;
  field Bits: u128;
  field Address: *i32;
  field Values: [i32; 2];

  func update(): i32
  {
    *this.Address = *this.Address + this.Values[0];
    this.Bits = this.Bits + 1;
    return *this.Address + this.Values[1];
  }

  func __init__(InitialTag: u8, InitialBits: u128, InitialAddress: *i32, InitialValues: [i32; 2]): void
  {
    this.Tag = InitialTag;
    this.Bits = InitialBits;
    this.Address = InitialAddress;
    this.Values = InitialValues;
  }
};

import "C" func malloc(Size: u64): *Packet;
import "C" func free(Address: *Packet): void;

func main(): i32
{
  var Target: i32 = 10;
  var Object = malloc(256);
  *Object = Packet(7, 340282366920938463463374607431768211455, &Target, [30, 2]);
  var Copy = *Object;
  var Result = Object->update();
  if (Object->Bits != 0 || Object->Tag != 7 || Copy.Bits != 340282366920938463463374607431768211455 || *Copy.Address != 40)
  {
    free(Object);
    return 1;
  }
  free(Object);
  return Result;
}
