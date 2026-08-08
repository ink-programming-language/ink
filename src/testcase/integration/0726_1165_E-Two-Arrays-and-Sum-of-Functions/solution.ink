// Translated from solution.cpp.

var maxn = (1e6 + 7);

var mod = 998244353;

var a = cpp_array(maxn);

var b = cpp_array(maxn);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      a[i] = ((a[i] * (((n - i) + 1))) * i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n));
  sort((b + 1), ((b + 1) + n));
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      ans = (((ans + (((a[i] % mod) * b[((n - i) + 1)]) % mod))) % mod);
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
