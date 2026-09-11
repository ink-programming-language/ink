// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var minimum: dynamic = 1;
  var maximum: dynamic = cpp_uninitialized();
  read(n, k);
  if ((k == n))
  {
    minimum = 0;
  }
  if ((k == 0))
  {
    maximum = 0;
    minimum = 0;
  } else if (((n / k) >= 3))
  {
    maximum = (2 * k);
  } else
  {
    var p: dynamic = (n / 3);
    maximum = (3 * p);
    k = (k - (n % 3));
    maximum = (maximum - k);
  }
  write(minimum, " ", maximum);
  return 0;
}
