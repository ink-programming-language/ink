// Translated from solution.cpp.

var maxn = (1e6 + 10);

var mod = 998244353;

var inf = 1e18;

var fac = cpp_array(maxn);

var ifac = cpp_array(maxn);

var sm1 = cpp_array(maxn);

var sm2 = cpp_array(maxn);

func Pow(a: dynamic, b: dynamic)
{
  var ans = 1;
  {
    while (b)
    {
      if ((b & 1))
      {
        ans = (((1 * ans) * a) % mod);
      }
      b >>= 1;
      a = (((1 * a) * a) % mod);
    }
  }
  return ans;
}

func C(n: dynamic, k: dynamic)
{
  if ((((n < 0) || (k < 0)) || (n < k)))
  {
    return 0;
  }
  return (((((1 * fac[n]) * ifac[k]) % mod) * ifac[(n - k)]) % mod);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie();
  fac[0] = 1;
  {
    var i = 1;
    while ((i < maxn))
    {
      fac[i] = (((1 * i) * fac[(i - 1)]) % mod);
      i += 1;
    }
  }
  ifac[(maxn - 1)] = Pow(fac[(maxn - 1)], (mod - 2));
  {
    var i = (maxn - 2);
    while ((i >= 0))
    {
      ifac[i] = (((1 * ifac[(i + 1)]) * ((i + 1))) % mod);
      i -= 1;
    }
  }
  var s: dynamic;
  read(s);
  var n = int_cpp((s).size());
  var cls = 0;
  var qus = 0;
  {
    var i = 0;
    while ((i < int_cpp((s).size())))
    {
      qus += (s[i] == cpp_char("?"));
      cls += (s[i] == cpp_char(")"));
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 0))
    {
      sm1[i] = (((sm1[(i + 1)] + C(qus, i))) % mod);
      sm2[i] = (((sm2[(i + 1)] + C((qus - 1), i))) % mod);
      i -= 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < int_cpp((s).size())))
    {
      if ((s[i] == cpp_char("?")))
      {
        ans = (((ans + sm2[max(0, ((i + 1) - cls))])) % mod);
      }
      if ((s[i] == cpp_char("(")))
      {
        ans = (((ans + sm1[max(0, ((i + 1) - cls))])) % mod);
      }
      i += 1;
    }
  }
  return cpp_comma(((cout << ans) << endl), 0);
}
