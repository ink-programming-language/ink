// Translated from solution.cpp.

class Pt
{
  var x: dynamic;
  var y: dynamic;
  func Pt()
  {
    }
  func Pt(x: dynamic, y: dynamic)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func Pt(p: dynamic)
  {
      this->x = cpp_construct(p.x);
      this->y = cpp_construct(p.y);
    }
  func operator_add(p: dynamic)
  {
      return Pt((x + p.x), (y + p.y));
    }
  func operator_subtract()
  {
      return Pt((-x), (-y));
    }
  func operator_subtract(p: dynamic)
  {
      return Pt((x - p.x), (y - p.y));
    }
  func operator_multiply(t: dynamic)
  {
      return Pt((x * t), (y * t));
    }
  func operator_divide(t: dynamic)
  {
      return Pt((x / t), (y / t));
    }
  func dot(v: dynamic)
  {
      return ((x * v.x) + (y * v.y));
    }
  func cross(v: dynamic)
  {
      return ((x * v.y) - (y * v.x));
    }
  func mid(p: dynamic)
  {
      return Pt((((x + p.x)) / 2), (((y + p.y)) / 2));
    }
  func d2()
  {
      return ((x * x) + (y * y));
    }
  func d()
  {
      return sqrt(d2());
    }
  func rot(th: dynamic)
  {
      var c = cos(th);
      var s = sin(th);
      return Pt(((c * x) - (s * y)), ((s * x) + (c * y)));
    }
  func rot90()
  {
      return Pt((-y), x);
    }
  func operator_equal(pt: dynamic)
  {
      return ((x == pt.x) && (y == pt.y));
    }
  func operator_less(pt: dynamic)
  {
      return ((x < pt.x) || (((x == pt.x) && (y < pt.y))));
    }
  func print()
  {
      printf("(%d,%d)", x, y);
    }
}

var qs = cpp_array(7);

func main()
{
  var p: dynamic;
  var v: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  scanf("%lf%lf%lf%lf%d%d%d%d", (&p.x), (&p.y), (&v.x), (&v.y), (&a), (&b), (&c), (&d));
  qs[0] = Pt(b, 0);
  qs[1] = Pt(0, (0.5 * a));
  qs[2] = Pt(0, (0.5 * c));
  qs[3] = Pt((-d), (0.5 * c));
  qs[4] = Pt((-d), (-0.5 * c));
  qs[5] = Pt(0, (-0.5 * c));
  qs[6] = Pt(0, (-0.5 * a));
  var th = atan2(v.y, v.x);
  {
    var i = 0;
    while ((i < 7))
    {
      var r = (qs[i].rot(th) + p);
      printf("%.12lf %.12lf\n", r.x, r.y);
      i += 1;
    }
  }
  return 0;
}
