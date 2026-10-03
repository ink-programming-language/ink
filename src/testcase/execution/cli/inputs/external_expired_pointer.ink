// A pointer into a returned callee-local CString is rejected before native dereference.
import "C" func strchr(Text: *u8, Needle: i32): *u8;
import "C" func strcmp(Left: *u8, Right: *u8): i32;

func findTail(): *u8
{
  return strchr("prefix:tail", 58);
}

func main(): i32
{
  var Expired = findTail();
  return strcmp(Expired, ":tail");
}
