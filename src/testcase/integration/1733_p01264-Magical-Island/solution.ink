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

func F(L: dynamic, R: dynamic, r: dynamic) -> dynamic
{
  var f: dynamic = __cpp_lambda_1;
  return (f(R) - f(L));
}

func main() -> dynamic
{
  var w: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  while (cpp_comma((((cin >> w) >> h) >> s), s))
  {
    if ((h > w))
    {
      swap(h, w);
    }
    var check: dynamic = __cpp_lambda_2;
    var ng: dynamic = (sqrt(((w * w) + (h * h))) / 2);
    var ok: dynamic = sqrt(((w * w) + (h * h)));
    rep(i, 40);
    {
      var mid: dynamic = (((ng + ok)) / 2);
      if (check(mid))
      {
        ok = mid;
      } else
      {
        ng = mid;
      }
    }
    printf("%.10f\n", ((ok * ok) * 4));
  }
  return 0;
}

func __cpp_lambda_1(x: dynamic) -> dynamic
{
  return ((((x * sqrt(((r * r) - (x * x)))) + ((r * r) * asin((x / r))))) / 2.0);
}

func __cpp_lambda_2(r: dynamic) -> dynamic
{
  var lx: dynamic = (w / 2);
  var rx: dynamic = sqrt(((r * r) - ((h * h) / 4)));
  rx = min(rx, w);
  var S: dynamic = F(lx, rx, r);
  S -= ((((rx - lx)) * h) / 2);
  if ((r > h))
  {
    rx = sqrt(((r * r) - (h * h)));
    rx = min(rx, w);
    if ((lx < rx))
    {
      S -= F(lx, rx, r);
      S += (((rx - lx)) * h);
    }
  }
  return ((S * 4) >= s);
}
