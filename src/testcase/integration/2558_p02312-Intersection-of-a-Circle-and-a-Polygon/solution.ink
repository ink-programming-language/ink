// Translated from solution.cpp.

var x = cpp_expression("#inclu");

var y = cpp_expression("#inclu");

var X = cpp_expression("#incl");

var Y = cpp_expression("#inclu");

var cpt = cpp_expression("#include<");

var EPS = 1e-9;

var pi = acos(-1);

func deq(a: dynamic, b: dynamic)
{
  return (abs((a - b)) < EPS);
}

enum Orientation
{
  CCW,
  CW,
  CNEITHER
}

class Pt
{
  func Pt()
  {
    }
  func Pt(a: dynamic)
  {
      this->cpx = cpp_construct(a);
    }
  func x()
  {
      return cpp_cast((*this));
    }
  func y()
  {
      return (cpp_cast(this))[1];
    }
  func operator_equal(b: dynamic)
  {
      return (abs(((*this) - b)) < EPS);
    }
  func operator_less(b: dynamic)
  {
      return ((x < b.x) || (((x == b.x) && (y < b.y))));
    }
}

func operator_shift_right(is: dynamic, p: dynamic)
{
  return ((is >> p.x) >> p.y);
}

func dot(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).x;
}

func det(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).y;
}

func angle(a: dynamic, b: dynamic)
{
  return arg((b - a));
}

func angle(a: dynamic, b: dynamic, c: dynamic)
{
  return arg((((a - b)) / ((c - b))));
}

func slope(a: dynamic, b: dynamic)
{
  return (((b.y - a.y)) / ((b.x - a.x)));
}

func rotate(a: dynamic, theta: dynamic)
{
  return (a * polar(cpp_cast(1.0), theta));
}

func rotate(a: dynamic, p: dynamic, theta: dynamic)
{
  return (rotate((a - p), theta) + p);
}

func project(p: dynamic, v: dynamic)
{
  return ((v * dot(p, v)) / norm(v));
}

func project(p: dynamic, a: dynamic, b: dynamic)
{
  return (a + project((p - a), (b - a)));
}

func reflect(p: dynamic, a: dynamic, b: dynamic)
{
  return (a + (conj((((p - a)) / ((b - a)))) * ((b - a))));
}

class Circle
{
  var c: dynamic;
  var r: dynamic;
  func operator_equal(b: dynamic)
  {
      return ((c == b.c) && deq(r, b.r));
    }
}

func areapolygoncircle(p: dynamic, c: dynamic)
{
  var n = p.size();
  var r = 0;
  {
    var i = (n - 1);
    var j = 0;
    while ((j < n))
    {
      var v = (abs((p[j] - p[i])) / ((p[j] - p[i])));
      if ((p[j] == p[i]))
      {
        i = cpp_update(j, "++");
        continue;
      }
      assert((p[j] != p[i]));
      var a = (((p[i] - c.c)) * v);
      var b = (((p[j] - c.c)) * v);
      var d = (norm(c.r) - norm(a.y));
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
      var l: dynamic;
      var m: dynamic;
      r += ((norm(c.r) * (((cpp_assign(l, "=", (atan2(b.y, min(b.x, (-d))) - atan2(a.y, min(a.x, (-d)))))) + (cpp_assign(m, "=", (atan2(b.y, max(b.x, d)) - atan2(a.y, max(a.x, d)))))))) + (a.y * ((min(d, max(a.x, (-d))) - max((-d), min(b.x, d))))));
      assert((((((-pi) < l) && ((-pi) < m)) && (l < pi)) && (m < pi)));
      i = cpp_update(j, "++");
    }
  }
  return (r / 2);
}

func solve()
{
  var n: dynamic;
  var r: dynamic;
  read(n, r);
  for (var a in p)
  {
    read(a);
  }
  write(fixed, setprecision(10), areapolygoncircle(p, [0, cpp_cast(r)]), "\n");
}

func main()
{
  AIZU_CGL_7_H.solve();
}
