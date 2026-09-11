// Translated from solution.cpp.

var eps: dynamic = 1e-9;

var pi: dynamic = acos(-1.0);

var maxn: dynamic = (cpp_cast(1e5) + 10);

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var ql: dynamic = cpp_uninitialized();

var qr: dynamic = cpp_uninitialized();

var w: dynamic = cpp_array(maxn);

var sum: dynamic = cpp_array(maxn);

func solve() -> dynamic
{
  sum[0] = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sum[i] = (sum[(i - 1)] + w[i]);
      i += 1;
    }
  }
  var res: dynamic = cpp_cast(1e18);
  var ll: dynamic = 1;
  var rr: dynamic = n;
  var cur: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i & 1))
      {
        res = min(res, ((cur + (((sum[rr] - sum[(ll - 1)])) * l)) + (((((rr - ll) + 1)) * 1) * ql)));
        cur += ((w[rr] * 1) * r);
        rr -= 1;
      } else
      {
        res = min(res, ((cur + (((sum[rr] - sum[(ll - 1)])) * r)) + (((((rr - ll) + 1)) * 1) * qr)));
        cur += ((w[ll] * 1) * l);
        ll += 1;
      }
      i += 1;
    }
  }
  res = min(res, cur);
  return res;
}

func main() -> dynamic
{
  read(n, l, r, ql, qr);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&w[i]));
      i += 1;
    }
  }
  var res: dynamic = cpp_cast(1e18);
  res = min(res, solve());
  reverse((w + 1), ((w + n) + 1));
  swap(l, r);
  swap(ql, qr);
  res = min(res, solve());
  write(res, "\n");
  return 0;
}
