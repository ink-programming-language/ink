// Translated from solution.cpp.

var MOD = (cpp_cast(1e9) + 7);

var MOD2 = 1007681537;

var INF = cpp_cast(1e9);

var LINF = cpp_cast(1e18);

var PI = acos(cpp_cast(-1));

var EPS = 1e-9;

func gcd(a: dynamic, b: dynamic)
{
  var r: dynamic;
  while (b)
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func fpow(n: dynamic, k: dynamic, p: dynamic = MOD)
{
  var r = 1;
  {
    while (k)
    {
      if ((k & 1))
      {
        r = ((r * n) % p);
      }
      n = ((n * n) % p);
      k >>= 1;
    }
  }
  return r;
}

func chkmin(a: dynamic, val: dynamic)
{
  return if ((val < a)) cpp_comma(cpp_assign(a, "=", val), 1) else 0;
}

func chkmax(a: dynamic, val: dynamic)
{
  return if ((a < val)) cpp_comma(cpp_assign(a, "=", val), 1) else 0;
}

func isqrt(k: dynamic)
{
  var r = (sqrt(k) + 1);
  while (((r * r) > k))
  {
    r -= 1;
  }
  return r;
}

func icbrt(k: dynamic)
{
  var r = (cbrt(k) + 1);
  while ((((r * r) * r) > k))
  {
    r -= 1;
  }
  return r;
}

func addmod(a: dynamic, val: dynamic, p: dynamic = MOD)
{
  if (((cpp_assign(a, "=", ((a + val)))) >= p))
  {
    a -= p;
  }
}

func submod(a: dynamic, val: dynamic, p: dynamic = MOD)
{
  if (((cpp_assign(a, "=", ((a - val)))) < 0))
  {
    a += p;
  }
}

func mult(a: dynamic, b: dynamic, p: dynamic = MOD)
{
  return ((cpp_cast(a) * b) % p);
}

func inv(a: dynamic, p: dynamic = MOD)
{
  return fpow(a, (p - 2), p);
}

func sign(x: dynamic)
{
  return (x + EPS);
}

func sign(x: dynamic, y: dynamic)
{
  return sign((x - y));
}

var maxn = (1e6 + 5);

var n: dynamic;

var a = cpp_array(maxn);

var pos1: dynamic;

var pos2: dynamic;

var mn = cpp_array((maxn << 1));

var sm = cpp_array((maxn << 1));

func upd(p: dynamic, val: dynamic)
{
  p += (1 << 19);
  mn[p] = cpp_assign(sm[p], "=", val);
  while ((p > 1))
  {
    p >>= 1;
    sm[p] = (sm[(p << 1)] + sm[((p << 1) | 1)]);
    mn[p] = min(mn[(p << 1)], (sm[(p << 1)] + mn[((p << 1) | 1)]));
  }
}

func check(mi: dynamic)
{
  {
    var i = (0);
    while ((i < (n)))
    {
      upd(i, (-a[i]));
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (int_cpp((pos2).size()))))
    {
      var ix = pos2[i];
      upd(ix, 200);
      i += 1;
    }
  }
  var ptr = (int_cpp((pos2).size()) - 1);
  {
    var x = (0);
    while ((x < ((int_cpp((pos1).size()) + 1))))
    {
      if ((mi >= (1000 * x)))
      {
        var y = min(int_cpp((pos2).size()), (((mi - (1000 * x))) / 2000));
        while ((ptr >= y))
        {
          var ix = pos2[ptr];
          upd(ix, -2000);
          ptr -= 1;
        }
        if (x)
        {
          var ix = pos1[(x - 1)];
          upd(ix, 100);
        }
        var rm = ((mi - (1000 * x)) - (2000 * y));
        if (((((!x) && (a[0] == 1000))) || (((!y) && (a[0] == 2000)))))
        {
          upd(0, (rm - a[0]));
        } else
        {
          upd(0, (rm + (a[0] / 10)));
        }
        if ((mn[1] >= 0))
        {
          return 1;
        }
      }
      x += 1;
    }
  }
  return 0;
}

func solve()
{
  read(n);
  {
    var i = (0);
    while ((i < (n)))
    {
      read(a[i]);
      if ((a[i] == 1000))
      {
        pos1.push_back(i);
      } else
      {
        pos2.push_back(i);
      }
      i += 1;
    }
  }
  var lo = 0;
  var hi = 600000000;
  while ((lo < hi))
  {
    var mi = ((lo + hi) >> 1);
    if ((!check(mi)))
    {
      lo = (mi + 1);
    } else
    {
      hi = mi;
    }
  }
  write((((lo + hi) >> 1)), "\n");
}

func main()
{
  var JUDGE_ONLINE = 1;
  if (fopen("in.txt", "r"))
  {
    JUDGE_ONLINE = 0;
    assert(freopen("in.txt", "r", stdin));
  } else
  {
    ios_base.sync_with_stdio(0);
    cin.tie(0);
  }
  solve();
  if ((!JUDGE_ONLINE))
  {
  }
  return 0;
}
