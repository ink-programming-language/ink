// Translated from solution.cpp.

var K: dynamic;

var N: dynamic;

var Max: dynamic;

var GCD: dynamic;

func gcd(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main()
{
  var i: dynamic;
  var x: dynamic;
  scanf("%d%d%d", (&N), (&K), (&x));
  Max = cpp_assign(GCD, "=", x);
  {
    i = 2;
    while ((i <= N))
    {
      scanf("%d", (&x));
      Max = max(Max, x);
      GCD = gcd(GCD, x);
      i += 1;
    }
  }
  if ((((K % GCD) == 0) && (K <= Max)))
  {
    puts("POSSIBLE");
  } else
  {
    puts("IMPOSSIBLE");
  }
}
