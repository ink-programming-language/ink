// Translated from solution.cpp.

func main()
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  while ((~scanf("%lld%lld%lld", (&x), (&y), (&z))))
  {
    var n1 = (x / z);
    var m1 = (x % z);
    var n2 = (y / z);
    var m2 = (y % z);
    var sum = (n1 + n2);
    var k = 0;
    if (((m1 + m2) >= z))
    {
      var k1 = (z - m1);
      var k2 = (z - m2);
      k = min(k1, k2);
      sum = (sum + 1);
    }
    printf("%lld %lld\n", sum, k);
  }
  return 0;
}
