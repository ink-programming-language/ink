// Translated from solution.cpp.

func gcd(x: dynamic, y: dynamic)
{
  if ((y == 0))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func main()
{
  var x: dynamic;
  var y: dynamic;
  read(x, y);
  write((gcd((x - 1), (y - 1)) + 1), "\n");
}
