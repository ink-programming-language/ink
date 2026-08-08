// Translated from solution.cpp.

var maxv = 2045;

var mod = (1e9 + 7);

var maxn = (1e6 + 40);

var fac = cpp_array(maxn);

var inv = cpp_array(maxn);

func qpow(a: dynamic, p: dynamic)
{
  var ans = 1;
  var xx = a;
  while ((p > 0))
  {
    if ((p & 1))
    {
      ans = (((xx * ans)) % mod);
    }
    xx = (((xx * xx)) % mod);
    p >>= 1;
  }
  return ans;
}

func init()
{
  fac[0] = 1;
  inv[0] = 1;
  {
    var i = 1;
    while ((i < maxn))
    {
      fac[i] = (((fac[(i - 1)] * i)) % mod);
      inv[i] = ((inv[(i - 1)] * qpow(i, (mod - 2))) % mod);
      i += 1;
    }
  }
}

var h: dynamic;

var w: dynamic;

var n: dynamic;

var a = cpp_array(maxv);

var dp = cpp_array(maxv);

func culC(a: dynamic, b: dynamic)
{
  return ((((fac[a] * inv[(a - b)]) % mod) * inv[b]) % mod);
}

func path(sx: dynamic, sy: dynamic, tx: dynamic, ty: dynamic)
{
  return culC((((ty - sy) + tx) - sx), (tx - sx));
}

func solve()
{
  {
    var i = 0;
    while ((i <= n))
    {
      var ans = 0;
      {
        var j = 0;
        while ((j < i))
        {
          if ((a[j].second <= a[i].second))
          {
            ans += ((path(a[j].first, a[j].second, a[i].first, a[i].second) * dp[j]) % mod);
            ans %= mod;
          }
          j += 1;
        }
      }
      dp[i] = ((((path(1, 1, a[i].first, a[i].second) - ans)) % mod) + mod);
      dp[i] %= mod;
      i += 1;
    }
  }
}

func main()
{
  init();
  read(h, w, n);
  {
    var i = 0;
    while ((i < n))
    {
      var c: dynamic;
      var r: dynamic;
      scanf("%d%d", (&r), (&c));
      a[i].first = r;
      a[i].second = c;
      i += 1;
    }
  }
  sort(a, (a + n));
  a[n] = pair(h, w);
  solve();
  write(dp[n], "\n");
  return 0;
}
