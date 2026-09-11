// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  } else
  {
    return gcd(b, (a % b));
  }
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a / gcd(a, b))) * b);
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  scanf("%lld%lld", (&N), (&M));
  {
    i = 0;
    while ((i < N))
    {
      scanf("%lld", (&a[i]));
      a[i] /= 2;
      i += 1;
    }
  }
  var L: dynamic = 1;
  {
    i = 0;
    while ((i < N))
    {
      L = lcm(L, a[i]);
      if ((L > M))
      {
        printf("0\n");
        return 0;
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < N))
    {
      if (((((L / a[i])) % 2) == 0))
      {
        printf("0\n");
        return 0;
      }
      i += 1;
    }
  }
  printf("%lld\n", (((M + L)) / ((2 * L))));
  return 0;
}
