// Translated from solution.cpp.

var MOD: dynamic = (cpp_cast(1e9) + 7);

var MOD2: dynamic = 1007681537;

var INF: dynamic = cpp_cast(1e9);

var LINF: dynamic = cpp_cast(1e18);

var PI: dynamic = acos(cpp_cast(-1));

var EPS: dynamic = 1e-9;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  while (b)
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcd(a, b)) * b);
}

func fpow(n: dynamic, k: dynamic, p: dynamic = MOD) -> dynamic
{
  var r: dynamic = 1;
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

func chkmin(a: dynamic, val: dynamic) -> dynamic
{
  return  ((val < a)) ? cpp_comma(cpp_assign(a, "=", val), 1) : 0;
}

func chkmax(a: dynamic, val: dynamic) -> dynamic
{
  return  ((a < val)) ? cpp_comma(cpp_assign(a, "=", val), 1) : 0;
}

func isqrt(k: dynamic) -> dynamic
{
  var r: dynamic = (sqrt(k) + 1);
  while (((r * r) > k))
  {
    r -= 1;
  }
  return r;
}

func icbrt(k: dynamic) -> dynamic
{
  var r: dynamic = (cbrt(k) + 1);
  while ((((r * r) * r) > k))
  {
    r -= 1;
  }
  return r;
}

func addmod(a: dynamic, val: dynamic, p: dynamic = MOD) -> dynamic
{
  if (((cpp_assign(a, "=", ((a + val)))) >= p))
  {
    a -= p;
  }
}

func submod(a: dynamic, val: dynamic, p: dynamic = MOD) -> dynamic
{
  if (((cpp_assign(a, "=", ((a - val)))) < 0))
  {
    a += p;
  }
}

func mult(a: dynamic, b: dynamic, p: dynamic = MOD) -> dynamic
{
  return ((cpp_cast(a) * b) % p);
}

func inv(a: dynamic, p: dynamic = MOD) -> dynamic
{
  return fpow(a, (p - 2), p);
}

func sign(x: dynamic) -> dynamic
{
  return (x + EPS);
}

func sign(x: dynamic, y: dynamic) -> dynamic
{
  return sign((x - y));
}

var maxn: dynamic = (1e6 + 5);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var pos1: dynamic = cpp_uninitialized();

var pos2: dynamic = cpp_uninitialized();

var mn: dynamic = cpp_array((maxn << 1));

var sm: dynamic = cpp_array((maxn << 1));

func upd(p: dynamic, val: dynamic) -> dynamic
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

func check(mi: dynamic) -> dynamic
{
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      upd(i, (-a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (int_cpp((pos2).size()))))
    {
      var ix: dynamic = pos2[i];
      upd(ix, 200);
      i += 1;
    }
  }
  var cpp_ptr: dynamic = (int_cpp((pos2).size()) - 1);
  {
    var x: dynamic = (0);
    while ((x < ((int_cpp((pos1).size()) + 1))))
    {
      if ((mi >= (1000 * x)))
      {
        var y: dynamic = min(int_cpp((pos2).size()), (((mi - (1000 * x))) / 2000));
        while ((cpp_ptr >= y))
        {
          var ix: dynamic = pos2[cpp_ptr];
          upd(ix, -2000);
          cpp_ptr -= 1;
        }
        if (x)
        {
          var ix: dynamic = pos1[(x - 1)];
          upd(ix, 100);
        }
        var rm: dynamic = ((mi - (1000 * x)) - (2000 * y));
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

func solve() -> dynamic
{
  read(n);
  {
    var i: dynamic = (0);
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
  var lo: dynamic = 0;
  var hi: dynamic = 600000000;
  while ((lo < hi))
  {
    var mi: dynamic = ((lo + hi) >> 1);
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

func main() -> dynamic
{
  var JUDGE_ONLINE: dynamic = 1;
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
