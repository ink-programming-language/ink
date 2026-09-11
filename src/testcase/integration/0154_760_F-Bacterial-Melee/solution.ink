// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var inf: dynamic = 2e9;

var INF: dynamic = 8e18;

var fre: dynamic = cpp_array(5001, 26);

var ts: dynamic = cpp_array(5001);

var ff: dynamic = cpp_array(5001);

var fac: dynamic = cpp_array(5001);

var nf: dynamic = cpp_array(5001);

var s: dynamic = cpp_uninitialized();

func fe(x: dynamic, e: dynamic) -> dynamic
{
  var r: dynamic = 1;
  while (e)
  {
    if ((e & 1))
    {
      r = (((r * x)) % MOD);
    }
    x = (((x * x)) % MOD);
    e >>= 1;
  }
  return r;
}

func ncr(n: dynamic, r: dynamic) -> dynamic
{
  var re: dynamic = fac[n];
  re = (((re * fe(fac[r], (MOD - 2)))) % MOD);
  re = (((re * fe(fac[(n - r)], (MOD - 2)))) % MOD);
  return re;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  fac[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= 5000))
    {
      fac[i] = (((cpp_cast(fac[(i - 1)]) * cpp_cast(i))) % MOD);
      i += 1;
    }
  }
  memset(ff, 0, cpp_sizeof(ff));
  memset(fre, 0, cpp_sizeof(fre));
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n);
  read(s);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      ts[i] = cpp_cast(((s[i] - cpp_char("a"))));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      nf[1] = 1;
      {
        var j: dynamic = 2;
        while ((j <= n))
        {
          nf[j] = ((((ff[(j - 1)] - fre[ts[i]][(j - 1)]) + MOD)) % MOD);
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          ff[j] = (((((ff[j] - fre[ts[i]][j]) + nf[j]) + MOD)) % MOD);
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          fre[ts[i]][j] = nf[j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans = (((ans + (cpp_cast(ff[i]) * ncr((n - 1), (i - 1))))) % MOD);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
