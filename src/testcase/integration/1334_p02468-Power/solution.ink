// Translated from solution.cpp.

func mod_pow(n: dynamic, m: dynamic, mod: dynamic)
{
  var res = 1;
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  var mod = 1000000007;
  scanf("%lld %lld", (&n), (&m));
  printf("%d\n", mod_pow(n, m, mod));
  return 0;
}
