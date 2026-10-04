// Class signatures and values retain the provider's nominal identity in a third independently built object.
from provider import Point;

func advance(Value: Point): Point
{
  var Result = Value;
  Result.add(2);
  return Result;
}
