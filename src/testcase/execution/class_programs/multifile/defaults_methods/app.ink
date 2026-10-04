// Both object orders must resolve constructor defaults, hidden methods, class arguments and frozen constants.
from provider import Point;
from provider import frozen;
from adapter import advance;

func main(): i32
{
  var Original = Point();
  const Changed = advance(Original);
  const Frozen = comptime frozen();
  if (Original.X != 38 || Changed.X != 40 || Frozen.X != 40)
  {
    return 1;
  }
  return Original.add(2);
}
