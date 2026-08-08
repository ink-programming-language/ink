// Translated from solution.cpp.

var MOD = (1e9 + 7);

var inf = 2e9;

var INF = 8e18;

var fre = cpp_array(5001, 26);

var ts = cpp_array(5001);

var ff = cpp_array(5001);

var fac = cpp_array(5001);

var nf = cpp_array(5001);

var s: dynamic;

func fe(x: dynamic, e: dynamic)
{
  var r = 1;
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

func ncr(n: dynamic, r: dynamic)
{
  var re = fac[n];
  re = (((re * fe(fac[r], (MOD - 2)))) % MOD);
  re = (((re * fe(fac[(n - r)], (MOD - 2)))) % MOD);
  return re;
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  fac[0] = 1;
  {
    var i = 1;
    while ((i <= 5000))
    {
      fac[i] = (((cpp_cast(fac[(i - 1)]) * cpp_cast(i))) % MOD);
      i += 1;
    }
  }
  memset(ff, 0, cpp_sizeof(ff));
  memset(fre, 0, cpp_sizeof(fre));
  var n: dynamic;
  var ans = 0;
  read(n);
  read(s);
  {
    var i = 0;
    while ((i < n))
    {
      ts[i] = cpp_cast(((s[i] - cpp_char("a"))));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      nf[1] = 1;
      {
        var j = 2;
        while ((j <= n))
        {
          nf[j] = ((((ff[(j - 1)] - fre[ts[i]][(j - 1)]) + MOD)) % MOD);
          j += 1;
        }
      }
      {
        var j = 1;
        while ((j <= n))
        {
          ff[j] = (((((ff[j] - fre[ts[i]][j]) + nf[j]) + MOD)) % MOD);
          j += 1;
        }
      }
      {
        var j = 1;
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
    var i = 1;
    while ((i <= n))
    {
      ans = (((ans + (cpp_cast(ff[i]) * ncr((n - 1), (i - 1))))) % MOD);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
