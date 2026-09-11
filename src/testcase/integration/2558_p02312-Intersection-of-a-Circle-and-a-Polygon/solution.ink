// Translated from solution.cpp.

var x: dynamic = cpp_expression("#inclu");

var y: dynamic = cpp_expression("#inclu");

var X: dynamic = cpp_expression("#incl");

var Y: dynamic = cpp_expression("#inclu");

var cpt: dynamic = cpp_expression("#include<");

var EPS: dynamic = 1e-9;

var pi: dynamic = acos(-1);

func deq(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) < EPS);
}

enum Orientation
{
  enum_field CCW;
  enum_field CW;
  enum_field CNEITHER;
}

class Pt
{
  func Pt() -> dynamic
  {
    }
  func Pt(a: dynamic) -> dynamic
  {
      self->cpx = cpp_construct(a);
    }
  func x() -> dynamic
  {
      return cpp_cast((*self));
    }
  func y() -> dynamic
  {
      return (cpp_cast(self))[1];
    }
  func operator_equal(b: dynamic) -> dynamic
  {
      return (abs(((*self) - b)) < EPS);
    }
  func operator_less(b: dynamic) -> dynamic
  {
      return ((x < b.x) || (((x == b.x) && (y < b.y))));
    }
}

func operator_shift_right(is: dynamic, p: dynamic) -> dynamic
{
  return ((is >> p.x) >> p.y);
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).x;
}

func det(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).y;
}

func angle(a: dynamic, b: dynamic) -> dynamic
{
  return arg((b - a));
}

func angle(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  return arg((((a - b)) / ((c - b))));
}

func slope(a: dynamic, b: dynamic) -> dynamic
{
  return (((b.y - a.y)) / ((b.x - a.x)));
}

func rotate(a: dynamic, theta: dynamic) -> dynamic
{
  return (a * polar(cpp_cast(1.0), theta));
}

func rotate(a: dynamic, p: dynamic, theta: dynamic) -> dynamic
{
  return (rotate((a - p), theta) + p);
}

func project(p: dynamic, v: dynamic) -> dynamic
{
  return ((v * dot(p, v)) / norm(v));
}

func project(p: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  return (a + project((p - a), (b - a)));
}

func reflect(p: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  return (a + (conj((((p - a)) / ((b - a)))) * ((b - a))));
}

class Circle
{
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func operator_equal(b: dynamic) -> dynamic
  {
      return ((c == b.c) && deq(r, b.r));
    }
}

func areapolygoncircle(p: dynamic, c: dynamic) -> dynamic
{
  var n: dynamic = p.size();
  var r: dynamic = 0;
  {
    var i: dynamic = (n - 1);
    var j: dynamic = 0;
    while ((j < n))
    {
      var v: dynamic = (abs((p[j] - p[i])) / ((p[j] - p[i])));
      if ((p[j] == p[i]))
      {
        i = cpp_update(j, "++");
        continue;
      }
      assert((p[j] != p[i]));
      var a: dynamic = (((p[i] - c.c)) * v);
      var b: dynamic = (((p[j] - c.c)) * v);
      var d: dynamic = (norm(c.r) - norm(a.y));
      if (cpp_expression("/*deq(a.y,0)*/"))
      {
        i = cpp_update(j, "++");
        continue;
      }
      if ((d < 0))
      {
        d = 0;
      }
      d = sqrt(d);
      var l: dynamic = cpp_uninitialized();
      var m: dynamic = cpp_uninitialized();
      r += ((norm(c.r) * (((cpp_assign(l, "=", (atan2(b.y, min(b.x, (-d))) - atan2(a.y, min(a.x, (-d)))))) + (cpp_assign(m, "=", (atan2(b.y, max(b.x, d)) - atan2(a.y, max(a.x, d)))))))) + (a.y * ((min(d, max(a.x, (-d))) - max((-d), min(b.x, d))))));
      assert((((((-pi) < l) && ((-pi) < m)) && (l < pi)) && (m < pi)));
      i = cpp_update(j, "++");
    }
  }
  return (r / 2);
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(n, r);
  for (var a: dynamic in p)
  {
    read(a);
  }
  write(fixed, setprecision(10), areapolygoncircle(p, [0, cpp_cast(r)]), "\n");
}

func main() -> dynamic
{
  AIZU_CGL_7_H.solve();
}
