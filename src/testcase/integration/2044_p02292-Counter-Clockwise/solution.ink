// Translated from solution.cpp.

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func repl(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(int)(a);i<(int)(b);i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func each(itr: dynamic, v: dynamic)
{
  return cpp_expression("#include <bits/");
}

func pb(s: dynamic)
{
  return cpp_expression("#include <bi");
}

func mp(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func dbg(x: dynamic)
{
  return cpp_expression("#include <bits/stdc+");
}

func maxch(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <");
}

func minch(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <");
}

func uni(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func exist(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func bcnt(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func x(p: dynamic)
{
  return cpp_expression("#includ");
}

func y(p: dynamic)
{
  return cpp_expression("#includ");
}

var eps = 1e-10;

var inf = 1e12;

func operator_less(a: dynamic, b: dynamic)
{
  return if ((x(a) == x(b))) (y(a) < y(b)) else (x(a) < x(b));
}

func cross(a: dynamic, b: dynamic)
{
  return y((conj(a) * b));
}

func dot(a: dynamic, b: dynamic)
{
  return x((conj(a) * b));
}

class L
{
  func L(a: dynamic, b: dynamic)
  {
      push_back(a);
      push_back(b);
    }
}

func projection(l: dynamic, p: dynamic)
{
  var b = (l[1] - l[0]);
  var c = (p - l[0]);
  return (l[0] + (b * x((c / b))));
}

func reflection(l: dynamic, p: dynamic)
{
  return (p + (2.0 * ((projection(l, p) - p))));
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  b = (b - a);
  c = (c - a);
  if ((cross(b, c) > 0))
  {
    return +1;
  }
  if ((cross(b, c) < 0))
  {
    return -1;
  }
  if ((dot(b, c) < 0))
  {
    return +2;
  }
  if ((norm(b) < norm(c)))
  {
    return -2;
  }
  return 0;
}

var s = ["ONLINE_FRONT", "CLOCKWISE", "ON_SEGMENT", "COUNTER_CLOCKWISE", "ONLINE_BACK"];

func main()
{
  cin.sync_with_stdio(false);
  var q: dynamic;
  var x1: dynamic;
  var x2: dynamic;
  var x3: dynamic;
  var y1: dynamic;
  var y2: dynamic;
  var y3: dynamic;
  read(x1, y1, x2, y2);
  read(q);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(x3, y3);
    var res = ccw(P(x1, y1), P(x2, y2), P(x3, y3));
    write(s[(res + 2)], "\n");
  }
