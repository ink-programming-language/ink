// Returning an element pointer does not extend the callee-local array storage lifetime.
import "C" func puts(Text: *u8): i32;
func escapedElement(): *i32
{
  var Values = [10, 20];
  var Index: i32 = 1;
  return &Values[Index];
}

func main(): i32
{
  var Address = escapedElement();
  // Copying/discarding an escaped address does not access the expired storage.
  puts("PASS");
  return 0;
}
