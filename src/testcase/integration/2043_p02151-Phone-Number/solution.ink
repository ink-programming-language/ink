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

var INF = 19191919;

func dist(p: dynamic, q: dynamic)
{
  return (abs((p.fi - q.fi)) + abs((p.se - q.se)));
}

func main()
{
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  var ct: dynamic;
  rep(i, (n - 1));
  {
    var a = (s[i] - cpp_char("0"));
    var b = (s[(i + 1)] - cpp_char("0"));
    ct[[a, b]] += 1;
  }
  var ans = INF;
  var v = cpp_construct(9);
  var p = cpp_construct(9);
  rep(i, 9)[i] = (i + 1);
  while (true)
  {
    var pos = cpp_construct(10);
    rep(i, 9)[p[i]] = [(i / 3), (i % 3)];
    var t = 0;
    for (var pp in ct)
    {
      var num = pp.se;
      var a = pp.fi.fi;
      var b = pp.fi.se;
      t += (num * dist(pos[a], pos[b]));
    }
    if ((t < ans))
    {
      ans = t;
      v = p;
    }
    if (!((next_permutation(all(p)))))
    {
      break;
    }
  }
  rep(i, 3);
  {
    (rep(j, 3) << v[((3 * i) + j)]);
    write("\n");
  }
  return 0;
}
