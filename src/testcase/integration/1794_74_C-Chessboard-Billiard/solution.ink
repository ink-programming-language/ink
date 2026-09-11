// Translated from solution.cpp.

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  if ((y == 0))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(x, y);
  write((gcd((x - 1), (y - 1)) + 1), "\n");
}
