// Translated from solution.cpp.

var ll: dynamic = dynamic;

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  return  ((y == 0)) ? x : gcd(y, (x % y));
}

func getSum(x: dynamic) -> dynamic
{
  var s: dynamic = 0;
  while ((x > 0))
  {
    s += (x % 10);
    x /= 10;
  }
  return s;
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    scanf("%lld", (&x));
    while ((gcd(x, getSum(x)) == 1))
    {
      x += 1;
    }
    printf("%lld\n", x);
  }
  return 0;
}
