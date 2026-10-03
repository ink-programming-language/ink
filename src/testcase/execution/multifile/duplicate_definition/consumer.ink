// An unrelated valid import must still compile when another source module has an error.
from second_provider import value as localValue;

func main(): i32
{
  return localValue();
}
