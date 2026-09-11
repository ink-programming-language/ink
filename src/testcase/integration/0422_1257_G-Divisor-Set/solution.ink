// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

var EPS: dynamic = 1e-9;

func binpow(b: dynamic, p: dynamic, mod: dynamic) -> dynamic
{
  var ans: dynamic = 1;
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

func pre() -> dynamic
{
}

var Divisors: dynamic = cpp_uninitialized();

func Divisor(x: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 2;
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

func check(prim: dynamic, p: dynamic, divs: dynamic) -> dynamic
{
  for (var v: dynamic in divs)
  {
    if ((binpow(prim, (((p - 1)) / v), p) == 1))
    {
      return 0;
    }
  }
  return 1;
}

func getRoot(p: dynamic) -> dynamic
{
  var ans: dynamic = 2;
  var divs: dynamic = Divisor((p - 1));
  while ((!check(ans, p, divs)))
  {
    ans += 1;
  }
  return ans;
}

func __cpp_top_level_1() -> dynamic
{
}

var NTTMOD: dynamic = 998244353;

var PRIMITIVE_ROOT: dynamic = 3;

var MAXB: dynamic = (1 << 21);

func modInv(a: dynamic) -> dynamic
{
  return  ((a <= 1)) ? a : ((cpp_cast(((NTTMOD - (NTTMOD / a)))) * modInv((NTTMOD % a))) % NTTMOD);
}

func NTT(P: dynamic, n: dynamic, oper: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    var j: dynamic = 0;
    while ((i < (n - 1)))
    {
      {
        var s: dynamic = n;
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
    var d: dynamic = 0;
    while ((((1 << d)) < n))
    {
      var m: dynamic = (1 << d);
      var m2: dynamic = (m * 2);
      var unit_p0: dynamic = binpow(PRIMITIVE_ROOT, (((NTTMOD - 1)) / m2), NTTMOD);
      if ((oper < 0))
      {
        unit_p0 = modInv(unit_p0);
      }
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var unit: dynamic = 1;
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              var P1: dynamic = P[((i + j) + m)];
              var P2: dynamic = P[(i + j)];
              var t: dynamic = ((unit * P1) % NTTMOD);
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

func mul(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = cpp_construct(max(0, ((cpp_cast(a.size()) + cpp_cast(b.size())) - 1)), 0);
  var A: dynamic = cpp_array(MAXB);
  var B: dynamic = cpp_array(MAXB);
  var C: dynamic = cpp_array(MAXB);
  var len: dynamic = 1;
  while ((len < cpp_cast(ret.size())))
  {
    len <<= 1;
  }
  {
    var i: dynamic = 0;
    while ((i < len))
    {
      A[i] =  ((i < cpp_cast(a.size()))) ? a[i] : 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < len))
    {
      B[i] =  ((i < cpp_cast(b.size()))) ? b[i] : 0;
      i += 1;
    }
  }
  NTT(A, len, 1);
  NTT(B, len, 1);
  {
    var i: dynamic = 0;
    while ((i < len))
    {
      C[i] = ((cpp_cast(A[i]) * B[i]) % NTTMOD);
      i += 1;
    }
  }
  NTT(C, len, -1);
  {
    var i: dynamic = 0;
    var inv: dynamic = modInv(len);
    while ((i < cpp_cast(ret.size())))
    {
      ret[i] = ((cpp_cast(C[i]) * inv) % NTTMOD);
      i += 1;
    }
  }
  return ret;
}

func binpow(b: dynamic, p: dynamic) -> dynamic
{
  var ans: dynamic = vector(1, 1);
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

func calc(arr: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    return vector((arr[l] + 1), 1);
  }
  var mid: dynamic = (((l + r)) >> 1);
  var x: dynamic = calc(arr, l, mid);
  var y: dynamic = calc(arr, (mid + 1), r);
  return mul(x, y);
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var freq: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      freq[x] += 1;
      i += 1;
    }
  }
  var vals: dynamic = cpp_uninitialized();
  for (var v: dynamic in freq)
  {
    vals.emplace_back(v.second);
  }
  sort((vals).begin(), (vals).end());
  var pp: dynamic = calc(vals, 0, (vals.size() - 1));
  write(pp[(n / 2)], cpp_char("\n"));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  pre();
  var t: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      solve();
      i += 1;
    }
  }
}
