// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

var pb = cpp_expression("#include");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func dbg(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func operator_shift_left(o: dynamic, p: dynamic)
{
  (((((o << "(") << p.fi) << ",") << p.se) << ")");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  (o << "[");
  for (var t in v)
  {
    ((o << t) << ",");
  }
  (o << "]");
  return o;
}

func F(L: dynamic, R: dynamic, r: dynamic)
{
  var f = __cpp_lambda_1;
  return (f(R) - f(L));
}

func main()
{
  var w: dynamic;
  var h: dynamic;
  var s: dynamic;
  while (cpp_comma((((cin >> w) >> h) >> s), s))
  {
    if ((h > w))
    {
      swap(h, w);
    }
    var check = __cpp_lambda_2;
    var ng = (sqrt(((w * w) + (h * h))) / 2);
    var ok = sqrt(((w * w) + (h * h)));
    rep(i, 40);
    {
      var mid = (((ng + ok)) / 2);
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

func __cpp_lambda_1(x: dynamic)
{
  return ((((x * sqrt(((r * r) - (x * x)))) + ((r * r) * asin((x / r))))) / 2.0);
}

func __cpp_lambda_2(r: dynamic)
{
  var lx = (w / 2);
  var rx = sqrt(((r * r) - ((h * h) / 4)));
  rx = min(rx, w);
  var S = F(lx, rx, r);
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
