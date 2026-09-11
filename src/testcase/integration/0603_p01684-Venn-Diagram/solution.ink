// Translated from solution.cpp.

func ISEQ(c: dynamic) -> dynamic
{
  return cpp_expression("#include <algorithm> #");
}

var EPS: dynamic = 1e-8;

var INF: dynamic = 1e12;

func sig(a: dynamic, b: dynamic = 0) -> dynamic
{
  return  ((a < (b - EPS))) ? -1 :  ((a > (b + EPS))) ? 1 : 0;
}

func eq(a: dynamic, b: dynamic) -> dynamic
{
  return (sig(abs((a - b))) == 0);
}

func norm(a: dynamic) -> dynamic
{
  return (a * a);
}

var X: dynamic = cpp_expression("#inclu");

var Y: dynamic = cpp_expression("#inclu");

var IINF: dynamic = (1 << 28);

class C
{
  var o: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func C(o: dynamic, r: dynamic) -> dynamic
  {
      self->o = cpp_construct(o);
      self->r = cpp_construct(r);
    }
}

enum RELATION
{
  enum_field INCOMPARABLE = 0;
  enum_field SAME = 1;
  enum_field CONTAIN = 2;
  enum_field OVER = 4;
}

func cRel(c1: dynamic, c2: dynamic) -> dynamic
{
  var d: dynamic = abs((c1.o - c2.o));
  var rd: dynamic = (c1.r - c2.r);
  if (cpp_binary(eq(c1.o, c2.o), "and", eq(c1.r, c2.r)))
  {
    return make_pair(SAME, IINF);
  }
  if ((sig(d, rd) < 0))
  {
    return make_pair(OVER, 0);
  }
  if ((sig(d, rd) == 0))
  {
    return make_pair(OVER, 1);
  }
  if ((sig(d, (-rd)) < 0))
  {
    return make_pair(CONTAIN, 0);
  }
  if ((sig(d, (-rd)) == 0))
  {
    return make_pair(CONTAIN, 1);
  }
  if ((sig(d, (c1.r + c2.r)) < 0))
  {
    return make_pair(INCOMPARABLE, 2);
  }
  if ((sig(d, (c1.r + c2.r)) == 0))
  {
    return make_pair(INCOMPARABLE, 1);
  }
  return make_pair(INCOMPARABLE, 0);
}

func cc_area(c1: dynamic, c2: dynamic) -> dynamic
{
  var rel: dynamic = cRel(c1, c2);
  var d: dynamic = abs((c1.o - c2.o));
  if ((rel.first != INCOMPARABLE))
  {
    var r: dynamic = min(c1.r, c2.r);
    return ((r * r) * M_PI);
  }
  if ((rel.second <= 1))
  {
    return 0.0;
  }
  var rlcosA: dynamic = (((((d * d) + (c1.r * c1.r)) - (c2.r * c2.r))) / ((2 * d)));
  var A: dynamic = acos((rlcosA / c1.r));
  var B: dynamic = acos((((d - rlcosA)) / c2.r));
  return ((((c1.r * c1.r) * A) + ((c2.r * c2.r) * B)) - ((d * c1.r) * sin(A)));
}

func main() -> dynamic
{
  var w: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var ab: dynamic = cpp_uninitialized();
  while (true)
  {
    read(w, h, a, b, ab);
    if ((w == 0))
    {
      break;
    }
    var changed: dynamic = false;
    if (((a + EPS) < b))
    {
      swap(a, b);
      changed = true;
    }
    var ra: dynamic = sqrt((a / M_PI));
    var rb: dynamic = sqrt((b / M_PI));
    if (cpp_binary(((w + EPS) < (2 * ra)), "or", ((h + EPS) < (2 * ra))))
    {
      write("impossible", "\n");
      continue;
    }
    if ((abs((b - ab)) < EPS))
    {
      if (changed)
      {
        printf("%.10f %.10f %.10f %.10f %.10f %.10f\n", ra, ra, rb, ra, ra, ra);
      } else
      {
        printf("%.10f %.10f %.10f %.10f %.10f %.10f\n", ra, ra, ra, ra, ra, rb);
      }
      continue;
    }
    var lb: dynamic = 0.0;
    var ub: dynamic = ((ra + rb) + EPS);
    var ca: dynamic = cpp_construct(P(0, 0), ra);
    while (((ub - lb) > EPS))
    {
      var mid: dynamic = (((ub + lb)) / 2);
      var area: dynamic = cc_area(ca, C(P(mid, 0), rb));
      if ((area < ab))
      {
        ub = mid;
      } else
      {
        lb = mid;
      }
    }
    var xb: dynamic = (w - rb);
    var yb: dynamic = (h - rb);
    var x: dynamic = (ra - xb);
    var y: dynamic = (ra - yb);
    var dis: dynamic = sqrt(((x * x) + (y * y)));
    if (((dis + EPS) < lb))
    {
      write("impossible", "\n");
      continue;
    }
    w -= ((ra + rb));
    h -= ((ra + rb));
    var ss: dynamic = sqrt(((w * w) + (h * h)));
    var sin: dynamic = (h / ss);
    var cos: dynamic = (w / ss);
    if (changed)
    {
      printf("%.10f %.10f %.10f %.10f %.10f %.10f\n", (ra + (cos * lb)), (ra + (sin * lb)), rb, ra, ra, ra);
    } else
    {
      printf("%.10f %.10f %.10f %.10f %.10f %.10f\n", ra, ra, ra, (ra + (cos * lb)), (ra + (sin * lb)), rb);
    }
  }
}
