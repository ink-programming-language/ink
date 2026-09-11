// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

var pb: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func dbg(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func operator_shift_left(o: dynamic, p: dynamic) -> dynamic
{
  (((((o << "(") << p.fi) << ",") << p.se) << ")");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  (o << "[");
  for (var t: dynamic in v)
  {
    ((o << t) << ",");
  }
  (o << "]");
  return o;
}

func f(x: dynamic) -> dynamic
{
  return ((x * ((x + 1))) / 2);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf(" %d %d", (&n), (&m));
  var add: dynamic = cpp_construct((n + 1), vi(3));
  var ans: dynamic = 0;
  var a: dynamic = [];
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[0] = add[i][0];
      {
        var j: dynamic = 1;
        while ((j < 3))
        {
          a[j] += add[i][j];
          j += 1;
        }
      }
      ans += ((i * i) * i);
      ans -= ((i * i) * (((a[0] + a[1]) + a[2])));
      ans += (i * ((((a[0] * a[1]) + (a[1] * a[2])) + (a[2] * a[0]))));
      ans -= ((a[0] * a[1]) * a[2]);
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var q: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    scanf(" %d %lld", (&q), (&k));
    if ((q == 0))
    {
      var l: dynamic = 0;
      var r: dynamic = 100000;
      while (((r - l) > 1))
      {
        var mid: dynamic = (((l + r)) / 2);
        if ((f(mid) < k))
        {
          l = mid;
        } else
        {
          r = mid;
        }
      }
      if ((r <= n))
      {
        add[r][q] += 1;
      }
    } else
    {
      if ((k <= n))
      {
        add[k][q] += 1;
      }
    }
  }
