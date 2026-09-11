// Translated from solution.cpp.

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func repl(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)(a);i<(int)(b);i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

func each(itr: dynamic, v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func pb(s: dynamic) -> dynamic
{
  return cpp_expression("#include <bi");
}

func mp(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func dbg(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc+");
}

func maxch(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func minch(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func uni(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func exist(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func bcnt(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func x(p: dynamic) -> dynamic
{
  return cpp_expression("#includ");
}

func y(p: dynamic) -> dynamic
{
  return cpp_expression("#includ");
}

var eps: dynamic = 1e-10;

var inf: dynamic = 1e12;

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((x(a) == x(b))) ? (y(a) < y(b)) : (x(a) < x(b));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return y((conj(a) * b));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return x((conj(a) * b));
}

class L
{
  func L(a: dynamic, b: dynamic) -> dynamic
  {
      push_back(a);
      push_back(b);
    }
}

func projection(l: dynamic, p: dynamic) -> dynamic
{
  var b: dynamic = (l[1] - l[0]);
  var c: dynamic = (p - l[0]);
  return (l[0] + (b * x((c / b))));
}

func reflection(l: dynamic, p: dynamic) -> dynamic
{
  return (p + (2.0 * ((projection(l, p) - p))));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
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

var s: dynamic = ["ONLINE_FRONT", "CLOCKWISE", "ON_SEGMENT", "COUNTER_CLOCKWISE", "ONLINE_BACK"];

func main() -> dynamic
{
  cin.sync_with_stdio(false);
  var q: dynamic = cpp_uninitialized();
  var x1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var x3: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var y3: dynamic = cpp_uninitialized();
  read(x1, y1, x2, y2);
  read(q);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(x3, y3);
    var res: dynamic = ccw(P(x1, y1), P(x2, y2), P(x3, y3));
    write(s[(res + 2)], "\n");
  }
