// Translated from solution.cpp.

var ll = dynamic;

func gcd(x: dynamic, y: dynamic)
{
  return if ((y == 0)) x else gcd(y, (x % y));
}

func getSum(x: dynamic)
{
  var s = 0;
  while ((x > 0))
  {
    s += (x % 10);
    x /= 10;
  }
  return s;
}

func main()
{
  var t: dynamic;
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var x: dynamic;
    scanf("%lld", (&x));
    while ((gcd(x, getSum(x)) == 1))
    {
      x += 1;
    }
    printf("%lld\n", x);
  }
  return 0;
}
