// Import the Ink wrapper's real signature while leaving the C symbol inside its provider.
from provider import absolute;

// The loaded image performs its native call after crossing the source module boundary.
func main(): i32
{
  return absolute(-42);
}
