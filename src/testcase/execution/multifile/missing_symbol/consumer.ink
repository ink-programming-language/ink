// A real module cannot supply a member that it does not define.
from provider import missingValue as localValue;

func main(): i32
{
  return localValue();
}
