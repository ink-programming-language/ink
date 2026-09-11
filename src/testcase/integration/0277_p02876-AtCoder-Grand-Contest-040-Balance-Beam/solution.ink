// Translated from solution.cpp.

func ALL(v: dynamic) -> dynamic
{
  return cpp_expression("#ifndef BZ #pragma GCC");
}

func rep(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  cpp_macro("for (int i = (l); i < (r); ++i)");
}

class st
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

var N: dynamic = 120000;

var a: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var sm: dynamic = cpp_array(N);

var ap: dynamic = 0;

var aq: dynamic = 1;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  while (b)
  {
    var q: dynamic = (a % b);
    a = b;
    b = q;
  }
  return a;
}

func upd(p: dynamic, q: dynamic) -> dynamic
{
  var g: dynamic = gcd(p, q);
  p /= g;
  q /= g;
  if (((lll(ap) * q) < (lll(p) * aq)))
  {
    ap = p;
    aq = q;
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(20);
  read(n);
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i].a, a[i].b);
      sum += a[i].a;
      i += 1;
    }
  }
  sort(a, (a + n), __cpp_lambda_1);
  sm[0] = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      sm[(i + 1)] = (sm[i] + max(a[i].a, a[i].b));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var l: dynamic = 0;
      var r: dynamic = (n + 1);
      while (((r - l) > 1))
      {
        var m: dynamic = (((l + r)) >> 1);
        var cur: dynamic = ((sum - a[i].b) - sm[m]);
        if ((i < m))
        {
          cur += max(a[i].a, a[i].b);
        }
        if ((cur > 0))
        {
          l = m;
        } else
        {
          r = m;
        }
      }
      if ((r <= n))
      {
        var cnt: dynamic = r;
        if ((i >= r))
        {
          cnt += 1;
        }
        cnt = (n - cnt);
        var cur: dynamic = (sum - sm[r]);
        if ((i < r))
        {
          cur += max(a[i].a, a[i].b);
        }
        if ((cur <= 0))
        {
          upd((cnt + 1), n);
        } else
        {
          assert((cur <= a[i].b));
          var p: dynamic = (((cnt * a[i].b) + a[i].b) - cur);
          var q: dynamic = (a[i].b * n);
          upd(p, q);
        }
      }
      i += 1;
    }
  }
  write(ap, " ", aq, "\n");
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (max(a.a, a.b) > max(b.a, b.b));
}
