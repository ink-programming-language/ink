// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var N: dynamic = (3e5 + 5);

var mo: dynamic = 998244353;

var a: dynamic = cpp_array(N);

var f: dynamic = cpp_array(N);

var inv: dynamic = cpp_array(N);

func work() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      m += a[i];
      i += 1;
    }
  }
  inv[1] = 1;
  {
    var i: dynamic = 2;
    while ((i <= m))
    {
      inv[i] = (((1 * inv[(mo % i)]) * ((mo - (mo / i)))) % mo);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var y: dynamic = cpp_uninitialized();
    while ((i < m))
    {
      y = ((((i * ((n - 1))) % mo) * inv[(m - i)]) % mo);
      f[(i + 1)] = (((((((((((y + 1)) % mo) * f[i]) % mo) - ((y * ((f[(i - 1)] + 1))) % mo))) % mo) + mo)) % mo);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans = (((ans + f[a[i]])) % mo);
      i += 1;
    }
  }
  printf("%d\n", ((((ans - f[m]) + mo)) % mo));
}

func main() -> dynamic
{
  FGF.work();
  return 0;
}
