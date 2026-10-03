// Importing the full overload set must not silently choose the first provider definition.
from provider import choose;

// Both imported signatures can represent 7, so semantic analysis must report an ambiguous call.
func main(): i32
{
  return choose(7);
}
