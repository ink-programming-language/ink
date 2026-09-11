// Translated from solution.cpp.

var mod: dynamic = 998244353;

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(3010);

var f: dynamic = cpp_array(3010);

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d", (&n), (&s));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = s;
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
