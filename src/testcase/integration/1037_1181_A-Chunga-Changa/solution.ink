// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  while ((~scanf("%lld%lld%lld", (&x), (&y), (&z))))
  {
    var n1: dynamic = (x / z);
    var m1: dynamic = (x % z);
    var n2: dynamic = (y / z);
    var m2: dynamic = (y % z);
    var sum: dynamic = (n1 + n2);
    var k: dynamic = 0;
    if (((m1 + m2) >= z))
    {
      var k1: dynamic = (z - m1);
      var k2: dynamic = (z - m2);
      k = min(k1, k2);
      sum = (sum + 1);
    }
    printf("%lld %lld\n", sum, k);
  }
  return 0;
}
