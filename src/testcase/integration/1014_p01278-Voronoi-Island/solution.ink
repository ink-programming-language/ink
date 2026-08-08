// Translated from solution.cpp.

var int_cpp = dynamic;

var FR = cpp_expression("#incl");

var SC = cpp_expression("#inclu");

func all(v: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (int)(n); i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (int)(f); i < (int)(n); i++)");
}

func each(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/s");
}

var inf = (1 << 55);

var EPS = cpp_expression("#includ");

func equals(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func lt(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/std");
}

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(x: dynamic = 0.0, y: dynamic = 0.0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_add(p: dynamic)
  {
      return Point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic)
  {
      return Point((x - p.x), (y - p.y));
    }
  func operator_multiply(a: dynamic)
  {
      return Point((x * a), (y * a));
    }
  func operator_divide(a: dynamic)
  {
      return Point((x / a), (y / a));
    }
  func abs()
  {
      return sqrt(norm());
    }
  func norm()
  {
      return ((x * x) + (y * y));
    }
  func operator_less(p: dynamic)
  {
      return if ((x != p.x)) (x < p.x) else (y < p.y);
    }
  func operator_equal(p: dynamic)
  {
      return ((fabs((x - p.x)) < EPS) && (fabs((y - p.y)) < EPS));
    }
}

class Circle
{
  var c: dynamic;
  var r: dynamic;
  func Circle(c: dynamic = Point(), r: dynamic = 0.0)
  {
      this->c = cpp_construct(c);
      this->r = cpp_construct(r);
    }
}

class Segment
{
  var p1: dynamic;
  var p2: dynamic;
  func Segment(p1: dynamic = Point(), p2: dynamic = Point())
  {
      this->p1 = cpp_construct(p1);
      this->p2 = cpp_construct(p2);
    }
}

func norm(v: dynamic)
{
  return ((v.x * v.x) + (v.y * v.y));
}

func abs(v: dynamic)
{
  return sqrt(norm(v));
}

func dot(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func isOrthogonal(a: dynamic, b: dynamic)
{
  return equals(dot(a, b), 0.0);
}

func isOrthogonal(a1: dynamic, a2: dynamic, b1: dynamic, b2: dynamic)
{
  return isOrthogonal((a1 - a2), (b1 - b2));
}

func isOrthogonal(s1: dynamic, s2: dynamic)
{
  return equals(dot((s1.p2 - s1.p1), (s2.p2 - s2.p1)), 0.0);
}

func isParallel(a: dynamic, b: dynamic)
{
  return equals(cross(a, b), 0.0);
}

func isParallel(a1: dynamic, a2: dynamic, b1: dynamic, b2: dynamic)
{
  return isParallel((a1 - a2), (b1 - b2));
}

func isParallel(s1: dynamic, s2: dynamic)
{
  return equals(cross((s1.p2 - s1.p1), (s2.p2 - s2.p1)), 0.0);
}

func project(s: dynamic, p: dynamic)
{
  var base = (s.p2 - s.p1);
  var r = (dot((p - s.p1), base) / norm(base));
  return (s.p1 + (base * r));
}

func reflect(s: dynamic, p: dynamic)
{
  return (p + (((project(s, p) - p)) * 2.0));
}

var COUNTER_CLOCKWISE = 1;

var CLOCKWISE = -1;

var ONLINE_BACK = 2;

var ONLINE_FRONT = -2;

var ON_SEGMENT = 0;

func ccw(p0: dynamic, p1: dynamic, p2: dynamic)
{
  var a = (p1 - p0);
  var b = (p2 - p0);
  if ((cross(a, b) > EPS))
  {
    return COUNTER_CLOCKWISE;
  }
  if ((cross(a, b) < (-EPS)))
  {
    return CLOCKWISE;
  }
  if ((dot(a, b) < (-EPS)))
  {
    return ONLINE_BACK;
  }
  if ((a.norm() < b.norm()))
  {
    return ONLINE_FRONT;
  }
  return ON_SEGMENT;
}

func intersect(p1: dynamic, p2: dynamic, p3: dynamic, p4: dynamic)
{
  return ((((ccw(p1, p2, p3) * ccw(p1, p2, p4)) <= 0) && ((ccw(p3, p4, p1) * ccw(p3, p4, p2)) <= 0)));
}

func intersect(s1: dynamic, s2: dynamic)
{
  return intersect(s1.p1, s1.p2, s2.p1, s2.p2);
}

func getDistance(a: dynamic, b: dynamic)
{
  return abs((a - b));
}

func getDistanceLP(l: dynamic, p: dynamic)
{
  return abs((cross((l.p2 - l.p1), (p - l.p1)) / abs((l.p2 - l.p1))));
}

func getDistanceSP(s: dynamic, p: dynamic)
{
  if ((dot((s.p2 - s.p1), (p - s.p1)) < 0.0))
  {
    return abs((p - s.p1));
  }
  if ((dot((s.p1 - s.p2), (p - s.p2)) < 0.0))
  {
    return abs((p - s.p2));
  }
  return getDistanceLP(s, p);
}

func getDistance(s1: dynamic, s2: dynamic)
{
  if (intersect(s1, s2))
  {
    return 0.0;
  }
  return min(min(getDistanceSP(s1, s2.p1), getDistanceSP(s1, s2.p2)), min(getDistanceSP(s2, s1.p1), getDistanceSP(s2, s1.p2)));
}

func getCrossPoint(s1: dynamic, s2: dynamic)
{
  var base = (s2.p2 - s2.p1);
  var d1 = abs(cross(base, (s1.p1 - s2.p1)));
  var d2 = abs(cross(base, (s1.p2 - s2.p1)));
  var t = (d1 / ((d1 + d2)));
  return (s1.p1 + (((s1.p2 - s1.p1)) * t));
}

func getCrossPointLL(l1: dynamic, l2: dynamic)
{
  var v1 = (l1.p2 - l1.p1);
  var v2 = (l2.p2 - l2.p1);
  var d = cross(v2, v1);
  if ((abs(d) < EPS))
  {
    return l2.p1;
  }
  return (l1.p1 + ((v1 * cross(v2, (l2.p2 - l1.p1))) * ((1.0 / d))));
}

func getPerpendicularBisector(p1: dynamic, p2: dynamic)
{
  var c = (((p1 + p2)) / 2.0);
  var q = Point((c.x + ((p1.y - p2.y))), (c.y + ((p2.x - p1.x))));
  return Line(c, q);
}

func getArea(p: dynamic)
{
  var ret = 0.0;
  {
    var i = 0;
    while ((i < cpp_cast(p.size())))
    {
      ret += cross(p[i], p[(((i + 1)) % p.size())]);
      i += 1;
    }
  }
  return (abs(ret) / 2.0);
}

var IN_POLYGON = 2;

var ON_POLYGON = 1;

var OUT_POLYGON = 0;

func contains(g: dynamic, p: dynamic)
{
  var n = g.size();
  var x = false;
  {
    var i = 0;
    while ((i < n))
    {
      var a = (g[i] - p);
      var b = (g[(((i + 1)) % n)] - p);
      if (((abs(cross(a, b)) < EPS) && (dot(a, b) < EPS)))
      {
        return ON_POLYGON;
      }
      if ((a.y > b.y))
      {
        swap(a, b);
      }
      if ((((a.y < EPS) && (EPS < b.y)) && (cross(a, b) > EPS)))
      {
        x = (!x);
      }
      i += 1;
    }
  }
  return (if (x) IN_POLYGON else OUT_POLYGON);
}

func convexCut(s: dynamic, l: dynamic)
{
  var t: dynamic;
  {
    var i = 0;
    while ((i < cpp_cast(s.size())))
    {
      var a = s[i];
      var b = s[(((i + 1)) % s.size())];
      if ((ccw(l.p1, l.p2, a) != -1))
      {
        t.push_back(a);
      }
      if (((ccw(l.p1, l.p2, a) * ccw(l.p1, l.p2, b)) < 0))
      {
        t.push_back(getCrossPointLL(Line(a, b), l));
      }
      i += 1;
    }
  }
  return t;
}

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  write(fixed, setprecision(12));
  var N: dynamic;
  var M: dynamic;
  while (cpp_comma(((cin >> N) >> M), N))
  {
    rep(i, N);
    read(l[i].x, l[i].y);
    rep(i, M);
    read(C[i].x, C[i].y);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var p = l;
      rep(j, M);
      if ((i != j))
      {
        p = convexCut(p, getPerpendicularBisector(C[i], C[j]));
      }
      write(getArea(p), "\n");
    }
