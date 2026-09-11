// Translated from solution.cpp.

var maxv: dynamic = 2045;

var mod: dynamic = (1e9 + 7);

var maxn: dynamic = (1e6 + 40);

var fac: dynamic = cpp_array(maxn);

var inv: dynamic = cpp_array(maxn);

func qpow(a: dynamic, p: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  var xx: dynamic = a;
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

func init() -> dynamic
{
  fac[0] = 1;
  inv[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      fac[i] = (((fac[(i - 1)] * i)) % mod);
      inv[i] = ((inv[(i - 1)] * qpow(i, (mod - 2))) % mod);
      i += 1;
    }
  }
}

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxv);

var dp: dynamic = cpp_array(maxv);

func culC(a: dynamic, b: dynamic) -> dynamic
{
  return ((((fac[a] * inv[(a - b)]) % mod) * inv[b]) % mod);
}

func path(sx: dynamic, sy: dynamic, tx: dynamic, ty: dynamic) -> dynamic
{
  return culC((((ty - sy) + tx) - sx), (tx - sx));
}

func solve() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      var ans: dynamic = 0;
      {
        var j: dynamic = 0;
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

func main() -> dynamic
{
  init();
  read(h, w, n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var c: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
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
