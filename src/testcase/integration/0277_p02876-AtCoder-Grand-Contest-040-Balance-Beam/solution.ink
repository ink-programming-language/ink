// Translated from solution.cpp.

func ALL(v: dynamic)
{
  return cpp_expression("#ifndef BZ #pragma GCC");
}

func rep(i: dynamic, l: dynamic, r: dynamic)
{
  cpp_macro("for (int i = (l); i < (r); ++i)");
}

class st
{
  var a: dynamic;
  var b: dynamic;
}

var N = 120000;

var a = cpp_array(N);

var n: dynamic;

var sm = cpp_array(N);

var ap = 0;

var aq = 1;

func gcd(a: dynamic, b: dynamic)
{
  while (b)
  {
    var q = (a % b);
    a = b;
    b = q;
  }
  return a;
}

func upd(p: dynamic, q: dynamic)
{
  var g = gcd(p, q);
  p /= g;
  q /= g;
  if (((lll(ap) * q) < (lll(p) * aq)))
  {
    ap = p;
    aq = q;
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(20);
  read(n);
  var sum = 0;
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      sm[(i + 1)] = (sm[i] + max(a[i].a, a[i].b));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var l = 0;
      var r = (n + 1);
      while (((r - l) > 1))
      {
        var m = (((l + r)) >> 1);
        var cur = ((sum - a[i].b) - sm[m]);
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
        var cnt = r;
        if ((i >= r))
        {
          cnt += 1;
        }
        cnt = (n - cnt);
        var cur = (sum - sm[r]);
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
          var p = (((cnt * a[i].b) + a[i].b) - cur);
          var q = (a[i].b * n);
          upd(p, q);
        }
      }
      i += 1;
    }
  }
  write(ap, " ", aq, "\n");
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (max(a.a, a.b) > max(b.a, b.b));
}
