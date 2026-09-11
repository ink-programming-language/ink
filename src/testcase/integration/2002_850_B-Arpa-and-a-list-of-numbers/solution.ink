// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(500010);

var b: dynamic = cpp_array(1000010);

var c: dynamic = cpp_array(1000010);

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

class IntervalSum
{
  var pre: dynamic = cpp_array(1000010);
  func init(a: dynamic, n: dynamic) -> dynamic
  {
      pre[1] = a[1];
      {
        var i: dynamic = 2;
        while ((i <= n))
        {
          pre[i] = (pre[(i - 1)] + a[i]);
          i += 1;
        }
      }
    }
  func sum(L: dynamic, R: dynamic) -> dynamic
  {
      if ((R < L))
      {
        return 0;
      }
      return (pre[R] - pre[(L - 1)]);
    }
}

var s1: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  n = read();
  y = read();
  x = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      b[a[i]] += 1;
      m = max(m, a[i]);
      i += 1;
    }
  }
  if ((m == 1))
  {
    m = 2;
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      c[i] = (b[i] * i);
      i += 1;
    }
  }
  s1.init(b, m);
  s2.init(c, m);
  {
    var d: dynamic = 2;
    var i: dynamic = 1;
    while ((d <= m))
    {
      while (((i < d) && ((1 * y) < ((1 * ((d - i))) * x))))
      {
        i += 1;
      }
      var tans: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((((j * d) + 1) <= m))
        {
          var t1: dynamic = s1.sum(((j * d) + 1), min(m, (((j * d) + i) - 1)));
          tans += (t1 * y);
          if (((i == d) || (((j * d) + i) > m)))
          {
            j += 1;
            continue;
          }
          t1 = s1.sum(((j * d) + i), min(m, (((j * d) + d) - 1)));
          var t2: dynamic = s2.sum(((j * d) + i), min(m, (((j * d) + d) - 1)));
          tans += (((((t1 * ((j + 1))) * d) - t2)) * x);
          j += 1;
        }
      }
      if ((d == 2))
      {
        ans = tans;
      } else
      {
        ans = min(ans, tans);
      }
      d += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
