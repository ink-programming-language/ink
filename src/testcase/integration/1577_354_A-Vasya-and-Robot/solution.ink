// Translated from solution.cpp.

var eps = 1e-9;

var pi = acos(-1.0);

var maxn = (cpp_cast(1e5) + 10);

var n: dynamic;

var l: dynamic;

var r: dynamic;

var ql: dynamic;

var qr: dynamic;

var w = cpp_array(maxn);

var sum = cpp_array(maxn);

func solve()
{
  sum[0] = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      sum[i] = (sum[(i - 1)] + w[i]);
      i += 1;
    }
  }
  var res = cpp_cast(1e18);
  var ll = 1;
  var rr = n;
  var cur = 0;
  {
    var i = 0;
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

func main()
{
  read(n, l, r, ql, qr);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&w[i]));
      i += 1;
    }
  }
  var res = cpp_cast(1e18);
  res = min(res, solve());
  reverse((w + 1), ((w + n) + 1));
  swap(l, r);
  swap(ql, qr);
  res = min(res, solve());
  write(res, "\n");
  return 0;
}
