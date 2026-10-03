// Import the real overload set under one alias without redeclaring any signatures.
from library import pick as remote;

// Check every overload after linking before returning the scenario's success value.
func main(): i32
{
  var Small: u8 = 7;
  if (remote(7) != 17)
  {
    return 1;
  }
  if (remote(Small) != 20)
  {
    return 2;
  }
  if (remote(true) != 30 || remote(false) != 31)
  {
    return 3;
  }
  if (remote() != 4)
  {
    return 4;
  }
  return 42;
}
