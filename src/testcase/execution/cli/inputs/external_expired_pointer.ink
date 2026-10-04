// A native-returned raw address may be discarded after its source storage ends.
import "C" func strchr(Text: *u8, Needle: i32): *u8;
import "C" func puts(Text: *u8): i32;

func findTail(): *u8
{
  return strchr("prefix:tail", 58);
}

func main(): i32
{
  var Expired = findTail();
  puts("PASS");
  return 0;
}
