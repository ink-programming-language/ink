import "C" func memset(Destination: *bool, Value: i32, Count: u64): *bool;

func main(): i32
{
  var Flag = false;
  memset(&Flag, 1, 1);
  if (Flag)
  {
    return 42;
  }
  return 0;
}
