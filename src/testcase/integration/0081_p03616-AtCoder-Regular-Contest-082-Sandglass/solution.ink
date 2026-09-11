// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#inclu");

var r: dynamic = cpp_array(MAX);

var a: dynamic = cpp_array(MAX);

var b: dynamic = cpp_array(MAX);

var k1: dynamic = cpp_array(MAX);

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(x, k);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= k))
    {
      var dt: dynamic = (r[i] - r[(i - 1)]);
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
    var i: dynamic = 1;
    while ((i <= q))
    {
      var tq: dynamic = cpp_uninitialized();
      var aq: dynamic = cpp_uninitialized();
      read(tq, aq);
      if ((tq == 0))
      {
        write(aq, "\n");
        i += 1;
        continue;
      }
      var p: dynamic = ((lower_bound(r, ((r + k) + 2), tq) - r) - 1);
      var dt: dynamic = (tq - r[p]);
      var res: dynamic = min((a[p] + max(((aq - k1[p]) + 1), 0)), b[p]);
      res =  (((p & 1))) ? min((res + dt), x) : max((res - dt), 0);
      write(res, "\n");
      i += 1;
    }
  }
}
