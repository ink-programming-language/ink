// Translated from solution.cpp.

func ISEQ(c: dynamic)
{
  return cpp_expression("#include <algorithm> #");
}

var EPS = 1e-8;

var INF = 1e12;

func sig(a: dynamic, b: dynamic = 0)
{
  return if ((a < (b - EPS))) -1 else if ((a > (b + EPS))) 1 else 0;
}

func eq(a: dynamic, b: dynamic)
{
  return (sig(abs((a - b))) == 0);
}

func norm(a: dynamic)
{
  return (a * a);
}

var X = cpp_expression("#inclu");

var Y = cpp_expression("#inclu");

var IINF = (1 << 28);

class C
{
  var o: dynamic;
  var r: dynamic;
  func C(o: dynamic, r: dynamic)
  {
      this->o = cpp_construct(o);
      this->r = cpp_construct(r);
    }
}

enum RELATION
{
  INCOMPARABLE = 0,
  SAME = 1,
  CONTAIN = 2,
  OVER = 4
}

func cRel(c1: dynamic, c2: dynamic)
{
  var d = abs((c1.o - c2.o));
  var rd = (c1.r - c2.r);
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

func cc_area(c1: dynamic, c2: dynamic)
{
  var rel = cRel(c1, c2);
  var d = abs((c1.o - c2.o));
  if ((rel.first != INCOMPARABLE))
  {
    var r = min(c1.r, c2.r);
    return ((r * r) * M_PI);
  }
  if ((rel.second <= 1))
  {
    return 0.0;
  }
  var rlcosA = (((((d * d) + (c1.r * c1.r)) - (c2.r * c2.r))) / ((2 * d)));
  var A = acos((rlcosA / c1.r));
  var B = acos((((d - rlcosA)) / c2.r));
  return ((((c1.r * c1.r) * A) + ((c2.r * c2.r) * B)) - ((d * c1.r) * sin(A)));
}

func main()
{
  var w: dynamic;
  var h: dynamic;
  var a: dynamic;
  var b: dynamic;
  var ab: dynamic;
  while (true)
  {
    read(w, h, a, b, ab);
    if ((w == 0))
    {
      break;
    }
    var changed = false;
    if (((a + EPS) < b))
    {
      swap(a, b);
      changed = true;
    }
    var ra = sqrt((a / M_PI));
    var rb = sqrt((b / M_PI));
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
    var lb = 0.0;
    var ub = ((ra + rb) + EPS);
    var ca = cpp_construct(P(0, 0), ra);
    while (((ub - lb) > EPS))
    {
      var mid = (((ub + lb)) / 2);
      var area = cc_area(ca, C(P(mid, 0), rb));
      if ((area < ab))
      {
        ub = mid;
      } else
      {
        lb = mid;
      }
    }
    var xb = (w - rb);
    var yb = (h - rb);
    var x = (ra - xb);
    var y = (ra - yb);
    var dis = sqrt(((x * x) + (y * y)));
    if (((dis + EPS) < lb))
    {
      write("impossible", "\n");
      continue;
    }
    w -= ((ra + rb));
    h -= ((ra + rb));
    var ss = sqrt(((w * w) + (h * h)));
    var sin = (h / ss);
    var cos = (w / ss);
    if (changed)
    {
      printf("%.10f %.10f %.10f %.10f %.10f %.10f\n", (ra + (cos * lb)), (ra + (sin * lb)), rb, ra, ra, ra);
    } else
    {
      printf("%.10f %.10f %.10f %.10f %.10f %.10f\n", ra, ra, ra, (ra + (cos * lb)), (ra + (sin * lb)), rb);
    }
  }
}
