// Translated from solution.cpp.

var MOD = 1000000007;

var EPS = 1e-9;

func binpow(b: dynamic, p: dynamic, mod: dynamic)
{
  var ans = 1;
  b %= mod;
  {
    while (p)
    {
      if ((p & 1))
      {
        ans = ((ans * b) % mod);
      }
      b = ((b * b) % mod);
      p >>= 1;
    }
  }
  return ans;
}

func pre()
{
}

var Divisors: dynamic;

func Divisor(x: dynamic)
{
  var ans: dynamic;
  {
    var i = 2;
    while (((i * i) <= x))
    {
      if (((x % i) == 0))
      {
        ans.emplace_back(i);
        while (((x % i) == 0))
        {
          x /= i;
        }
      }
      i += 1;
    }
  }
  if ((x > 1))
  {
    ans.emplace_back(x);
  }
  return ans;
}

func check(prim: dynamic, p: dynamic, divs: dynamic)
{
  for (var v in divs)
  {
    if ((binpow(prim, (((p - 1)) / v), p) == 1))
    {
      return 0;
    }
  }
  return 1;
}

func getRoot(p: dynamic)
{
  var ans = 2;
  var divs = Divisor((p - 1));
  while ((!check(ans, p, divs)))
  {
    ans += 1;
  }
  return ans;
}

func __cpp_top_level_1()
{
}

var NTTMOD = 998244353;

var PRIMITIVE_ROOT = 3;

var MAXB = (1 << 21);

func modInv(a: dynamic)
{
  return if ((a <= 1)) a else ((cpp_cast(((NTTMOD - (NTTMOD / a)))) * modInv((NTTMOD % a))) % NTTMOD);
}

func NTT(P: dynamic, n: dynamic, oper: dynamic)
{
  {
    var i = 1;
    var j = 0;
    while ((i < (n - 1)))
    {
      {
        var s = n;
        while (cpp_comma(cpp_assign(j, "^=", cpp_assign(s, ">>=", 1)), ((~j) & s)))
        {
        }
      }
      if ((i < j))
      {
        swap(P[i], P[j]);
      }
      i += 1;
    }
  }
  {
    var d = 0;
    while ((((1 << d)) < n))
    {
      var m = (1 << d);
      var m2 = (m * 2);
      var unit_p0 = binpow(PRIMITIVE_ROOT, (((NTTMOD - 1)) / m2), NTTMOD);
      if ((oper < 0))
      {
        unit_p0 = modInv(unit_p0);
      }
      {
        var i = 0;
        while ((i < n))
        {
          var unit = 1;
          {
            var j = 0;
            while ((j < m))
            {
              var P1 = P[((i + j) + m)];
              var P2 = P[(i + j)];
              var t = ((unit * P1) % NTTMOD);
              P1 = ((((P2 - t) + NTTMOD)) % NTTMOD);
              P2 = (((P2 + t)) % NTTMOD);
              unit = ((unit * unit_p0) % NTTMOD);
              j += 1;
            }
          }
          i += m2;
        }
      }
      d += 1;
    }
  }
}

func mul(a: dynamic, b: dynamic)
{
  var ret = cpp_construct(max(0, ((cpp_cast(a.size()) + cpp_cast(b.size())) - 1)), 0);
  var A = cpp_array(MAXB);
  var B = cpp_array(MAXB);
  var C = cpp_array(MAXB);
  var len = 1;
  while ((len < cpp_cast(ret.size())))
  {
    len <<= 1;
  }
  {
    var i = 0;
    while ((i < len))
    {
      A[i] = if ((i < cpp_cast(a.size()))) a[i] else 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < len))
    {
      B[i] = if ((i < cpp_cast(b.size()))) b[i] else 0;
      i += 1;
    }
  }
  NTT(A, len, 1);
  NTT(B, len, 1);
  {
    var i = 0;
    while ((i < len))
    {
      C[i] = ((cpp_cast(A[i]) * B[i]) % NTTMOD);
      i += 1;
    }
  }
  NTT(C, len, -1);
  {
    var i = 0;
    var inv = modInv(len);
    while ((i < cpp_cast(ret.size())))
    {
      ret[i] = ((cpp_cast(C[i]) * inv) % NTTMOD);
      i += 1;
    }
  }
  return ret;
}

func binpow(b: dynamic, p: dynamic)
{
  var ans = vector(1, 1);
  {
    while (p)
    {
      if ((p & 1))
      {
        ans = mul(ans, b);
      }
      b = mul(b, b);
      p >>= 1;
    }
  }
  return ans;
}

func calc(arr: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    return vector((arr[l] + 1), 1);
  }
  var mid = (((l + r)) >> 1);
  var x = calc(arr, l, mid);
  var y = calc(arr, (mid + 1), r);
  return mul(x, y);
}

func solve()
{
  var n: dynamic;
  read(n);
  var freq: dynamic;
  {
    var i = 0;
    while ((i < (n)))
    {
      var x: dynamic;
      read(x);
      freq[x] += 1;
      i += 1;
    }
  }
  var vals: dynamic;
  for (var v in freq)
  {
    vals.emplace_back(v.second);
  }
  sort((vals).begin(), (vals).end());
  var pp = calc(vals, 0, (vals.size() - 1));
  write(pp[(n / 2)], cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  pre();
  var t = 1;
  {
    var i = 1;
    while ((i <= t))
    {
      solve();
      i += 1;
    }
  }
}
