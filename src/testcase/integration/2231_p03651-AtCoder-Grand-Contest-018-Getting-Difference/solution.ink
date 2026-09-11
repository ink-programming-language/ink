// Translated from solution.cpp.

var K: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

var Max: dynamic = cpp_uninitialized();

var GCD: dynamic = cpp_uninitialized();

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
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
