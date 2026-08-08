// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
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

var n: dynamic;

var x: dynamic;

var y: dynamic;

var a = cpp_array(500010);

var b = cpp_array(1000010);

var c = cpp_array(1000010);

var m: dynamic;

var ans: dynamic;

class IntervalSum
{
  var pre: dynamic = cpp_array(1000010);
  func init(a: dynamic, n: dynamic)
  {
      pre[1] = a[1];
      {
        var i = 2;
        while ((i <= n))
        {
          pre[i] = (pre[(i - 1)] + a[i]);
          i += 1;
        }
      }
    }
  func sum(L: dynamic, R: dynamic)
  {
      if ((R < L))
      {
        return 0;
      }
      return (pre[R] - pre[(L - 1)]);
    }
}

var s1: dynamic;

var s2: dynamic;

func main()
{
  n = read();
  y = read();
  x = read();
  {
    var i = 1;
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
    var i = 1;
    while ((i <= m))
    {
      c[i] = (b[i] * i);
      i += 1;
    }
  }
  s1.init(b, m);
  s2.init(c, m);
  {
    var d = 2;
    var i = 1;
    while ((d <= m))
    {
      while (((i < d) && ((1 * y) < ((1 * ((d - i))) * x))))
      {
        i += 1;
      }
      var tans = 0;
      {
        var j = 0;
        while ((((j * d) + 1) <= m))
        {
          var t1 = s1.sum(((j * d) + 1), min(m, (((j * d) + i) - 1)));
          tans += (t1 * y);
          if (((i == d) || (((j * d) + i) > m)))
          {
            j += 1;
            continue;
          }
          t1 = s1.sum(((j * d) + i), min(m, (((j * d) + d) - 1)));
          var t2 = s2.sum(((j * d) + i), min(m, (((j * d) + d) - 1)));
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
