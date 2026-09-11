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

var INF: dynamic = 19191919;

func dist(p: dynamic, q: dynamic) -> dynamic
{
  return (abs((p.fi - q.fi)) + abs((p.se - q.se)));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  var ct: dynamic = cpp_uninitialized();
  rep(i, (n - 1));
  {
    var a: dynamic = (s[i] - cpp_char("0"));
    var b: dynamic = (s[(i + 1)] - cpp_char("0"));
    ct[[a, b]] += 1;
  }
  var ans: dynamic = INF;
  var v: dynamic = cpp_construct(9);
  var p: dynamic = cpp_construct(9);
  rep(i, 9)[i] = (i + 1);
  while (true)
  {
    var pos: dynamic = cpp_construct(10);
    rep(i, 9)[p[i]] = [(i / 3), (i % 3)];
    var t: dynamic = 0;
    for (var pp: dynamic in ct)
    {
      var num: dynamic = pp.se;
      var a: dynamic = pp.fi.fi;
      var b: dynamic = pp.fi.se;
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
