// An import supplies the real signature; an i64 local cannot masquerade as an i32 argument.
from provider import value as localValue;

func main(): i32
{
  var Argument: i64 = 42;
  return localValue(Argument);
}
