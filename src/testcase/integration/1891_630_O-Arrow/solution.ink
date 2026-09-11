// Translated from solution.cpp.

class Pt
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Pt() -> dynamic
  {
    }
  func Pt(x: dynamic, y: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func Pt(p: dynamic) -> dynamic
  {
      self->x = cpp_construct(p.x);
      self->y = cpp_construct(p.y);
    }
  func operator_add(p: dynamic) -> dynamic
  {
      return Pt((x + p.x), (y + p.y));
    }
  func operator_subtract() -> dynamic
  {
      return Pt((-x), (-y));
    }
  func operator_subtract(p: dynamic) -> dynamic
  {
      return Pt((x - p.x), (y - p.y));
    }
  func operator_multiply(t: dynamic) -> dynamic
  {
      return Pt((x * t), (y * t));
    }
  func operator_divide(t: dynamic) -> dynamic
  {
      return Pt((x / t), (y / t));
    }
  func dot(v: dynamic) -> dynamic
  {
      return ((x * v.x) + (y * v.y));
    }
  func cross(v: dynamic) -> dynamic
  {
      return ((x * v.y) - (y * v.x));
    }
  func mid(p: dynamic) -> dynamic
  {
      return Pt((((x + p.x)) / 2), (((y + p.y)) / 2));
    }
  func d2() -> dynamic
  {
      return ((x * x) + (y * y));
    }
  func d() -> dynamic
  {
      return sqrt(d2());
    }
  func rot(th: dynamic) -> dynamic
  {
      var c: dynamic = cos(th);
      var s: dynamic = sin(th);
      return Pt(((c * x) - (s * y)), ((s * x) + (c * y)));
    }
  func rot90() -> dynamic
  {
      return Pt((-y), x);
    }
  func operator_equal(pt: dynamic) -> dynamic
  {
      return ((x == pt.x) && (y == pt.y));
    }
  func operator_less(pt: dynamic) -> dynamic
  {
      return ((x < pt.x) || (((x == pt.x) && (y < pt.y))));
    }
  func print() -> dynamic
  {
      printf("(%d,%d)", x, y);
    }
}

var qs: dynamic = cpp_array(7);

func main() -> dynamic
{
  var p: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  scanf("%lf%lf%lf%lf%d%d%d%d", (&p.x), (&p.y), (&v.x), (&v.y), (&a), (&b), (&c), (&d));
  qs[0] = Pt(b, 0);
  qs[1] = Pt(0, (0.5 * a));
  qs[2] = Pt(0, (0.5 * c));
  qs[3] = Pt((-d), (0.5 * c));
  qs[4] = Pt((-d), (-0.5 * c));
  qs[5] = Pt(0, (-0.5 * c));
  qs[6] = Pt(0, (-0.5 * a));
  var th: dynamic = atan2(v.y, v.x);
  {
    var i: dynamic = 0;
    while ((i < 7))
    {
      var r: dynamic = (qs[i].rot(th) + p);
      printf("%.12lf %.12lf\n", r.x, r.y);
      i += 1;
    }
  }
  return 0;
}
