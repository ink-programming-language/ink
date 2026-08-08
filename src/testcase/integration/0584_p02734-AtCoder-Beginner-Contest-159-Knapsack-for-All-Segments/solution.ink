// Translated from solution.cpp.

var mod = 998244353;

var n: dynamic;

var s: dynamic;

var a = cpp_array(3010);

var f = cpp_array(3010);

var ans: dynamic;

func main()
{
  scanf("%d%d", (&n), (&s));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = s;
        while ((j > a[i]))
        {
          ((j == s) && (cpp_assign((cpp_assign(ans, "+=", ((1 * f[(j - a[i])]) * (((n - i) + 1))))), "%=", mod)));
          (cpp_assign(f[j], "+=", f[(j - a[i])])) %= mod;
          j -= 1;
        }
      }
      (cpp_assign(f[a[i]], "+=", i)) %= mod;
      if ((a[i] == s))
      {
        (cpp_assign(ans, "+=", ((1 * i) * (((n - i) + 1))))) %= mod;
      }
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
