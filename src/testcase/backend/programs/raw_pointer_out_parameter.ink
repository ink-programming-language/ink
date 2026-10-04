import "C" func strtoll(Text: *u8, End: **u8, Base: i32): i64;

func main(): i32
{
  var Dummy: u8 = 0;
  var End: *u8 = &Dummy;
  var Value = strtoll("41x", &End, 10);
  if (Value != 41 || *End != 120 || Dummy != 0)
  {
    return 1;
  }
  return 42;
}
