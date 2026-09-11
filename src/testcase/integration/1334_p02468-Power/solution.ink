// Translated from solution.cpp.

func mod_pow(n: dynamic, m: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while ((m > 0))
  {
    if ((m & 1))
    {
      res = ((res * n) % mod);
    }
    n = ((n * n) % mod);
    m >>= 1;
  }
  return res;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var mod: dynamic = 1000000007;
  scanf("%lld %lld", (&n), (&m));
  printf("%d\n", mod_pow(n, m, mod));
  return 0;
}
