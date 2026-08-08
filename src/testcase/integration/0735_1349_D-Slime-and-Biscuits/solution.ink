// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var ans: dynamic;

var N = (3e5 + 5);

var mo = 998244353;

var a = cpp_array(N);

var f = cpp_array(N);

var inv = cpp_array(N);

func work()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      m += a[i];
      i += 1;
    }
  }
  inv[1] = 1;
  {
    var i = 2;
    while ((i <= m))
    {
      inv[i] = (((1 * inv[(mo % i)]) * ((mo - (mo / i)))) % mo);
      i += 1;
    }
  }
  {
    var i = 1;
    var y: dynamic;
    while ((i < m))
    {
      y = ((((i * ((n - 1))) % mo) * inv[(m - i)]) % mo);
      f[(i + 1)] = (((((((((((y + 1)) % mo) * f[i]) % mo) - ((y * ((f[(i - 1)] + 1))) % mo))) % mo) + mo)) % mo);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      ans = (((ans + f[a[i]])) % mo);
      i += 1;
    }
  }
  printf("%d\n", ((((ans - f[m]) + mo)) % mo));
}

func main()
{
  FGF.work();
  return 0;
}
