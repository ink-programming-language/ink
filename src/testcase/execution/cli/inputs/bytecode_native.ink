// A loaded executable resolves this native symbol in its own process.
import "C" func abs(Value: i32): i32;

public func main(): i32
{
  return abs(-17);
}
