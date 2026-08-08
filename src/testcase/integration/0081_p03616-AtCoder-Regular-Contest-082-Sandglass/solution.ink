// Translated from solution.cpp.

var MAX = cpp_expression("#inclu");

var r = cpp_array(MAX);

var a = cpp_array(MAX);

var b = cpp_array(MAX);

var k1 = cpp_array(MAX);

func main()
{
  var x: dynamic;
  var k: dynamic;
  var q: dynamic;
  read(x, k);
  {
    var i = 1;
    while ((i <= k))
    {
      read(r[i]);
      i += 1;
    }
  }
  r[(k + 1)] = INT_MAX;
  a[0] = 0;
  b[0] = x;
  k1[0] = 1;
  {
    var i = 1;
    while ((i <= k))
    {
      var dt = (r[i] - r[(i - 1)]);
      if ((i & 1))
      {
        a[i] = max((a[(i - 1)] - dt), 0);
        b[i] = max((b[(i - 1)] - dt), 0);
        if ((a[i] == b[i]))
        {
          k1[i] = ((x + 1));
        } else
        {
          k1[i] = min((k1[(i - 1)] + max((dt - a[(i - 1)]), 0)), x);
        }
      } else
      {
        a[i] = min((a[(i - 1)] + dt), x);
        b[i] = min((b[(i - 1)] + dt), x);
        if ((a[i] == b[i]))
        {
          k1[i] = ((x + 1));
        } else
        {
          k1[i] = max((k1[(i - 1)] - max((((dt - x) + a[(i - 1)]) + 1), 0)), 0);
        }
      }
      i += 1;
    }
  }
  read(q);
  {
    var i = 1;
    while ((i <= q))
    {
      var tq: dynamic;
      var aq: dynamic;
      read(tq, aq);
      if ((tq == 0))
      {
        write(aq, "\n");
        i += 1;
        continue;
      }
      var p = ((lower_bound(r, ((r + k) + 2), tq) - r) - 1);
      var dt = (tq - r[p]);
      var res = min((a[p] + max(((aq - k1[p]) + 1), 0)), b[p]);
      res = if (((p & 1))) min((res + dt), x) else max((res - dt), 0);
      write(res, "\n");
      i += 1;
    }
  }
}
