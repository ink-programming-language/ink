import "C" func puts(Text: *u8): i32;
import "C" func strcpy(Destination: *u8, Source: *u8): *u8;

func replace(Number: *i32): void
{
  *Number = 42;
}

func main(): i32
{
  var Number: i32 = 7;
  var Address = &Number;
  var Alias = &*Address;
  replace(Alias);
  if (Number != 42 || *Address != 42)
  {
    return 1;
  }
  Number = 19;
  if (*Alias != 19)
  {
    return 2;
  }
  *(&Number) = 23;
  if (Number != 23)
  {
    return 3;
  }

  // C writes into the same byte that subsequent Ink loads observe.
  var Byte: u8 = 65;
  var Returned = strcpy(&Byte, "");
  if (Byte != 0 || *Returned != 0)
  {
    return 4;
  }
  *Returned = 66;
  if (Byte != 66)
  {
    return 5;
  }
  puts("AddressOf.PASS");
  return 0;
}
