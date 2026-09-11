// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var FR: dynamic = cpp_expression("#incl");

var SC: dynamic = cpp_expression("#inclu");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (int)(n); i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = (int)(f); i < (int)(n); i++)");
}

func each(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/s");
}

var inf: dynamic = (1 << 55);

var EPS: dynamic = cpp_expression("#includ");

func equals(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func lt(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0.0, y: dynamic = 0.0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func operator_add(p: dynamic) -> dynamic
  {
      return Point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic) -> dynamic
  {
      return Point((x - p.x), (y - p.y));
    }
  func operator_multiply(a: dynamic) -> dynamic
  {
      return Point((x * a), (y * a));
    }
  func operator_divide(a: dynamic) -> dynamic
  {
      return Point((x / a), (y / a));
    }
  func abs() -> dynamic
  {
      return sqrt(norm());
    }
  func norm() -> dynamic
  {
      return ((x * x) + (y * y));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      return  ((x != p.x)) ? (x < p.x) : (y < p.y);
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((fabs((x - p.x)) < EPS) && (fabs((y - p.y)) < EPS));
    }
}

class Circle
{
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func Circle(c: dynamic = Point(), r: dynamic = 0.0) -> dynamic
  {
      self->c = cpp_construct(c);
      self->r = cpp_construct(r);
    }
}

class Segment
{
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  func Segment(p1: dynamic = Point(), p2: dynamic = Point()) -> dynamic
  {
      self->p1 = cpp_construct(p1);
      self->p2 = cpp_construct(p2);
    }
}

func norm(v: dynamic) -> dynamic
{
  return ((v.x * v.x) + (v.y * v.y));
}

func abs(v: dynamic) -> dynamic
{
  return sqrt(norm(v));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func isOrthogonal(a: dynamic, b: dynamic) -> dynamic
{
  return equals(dot(a, b), 0.0);
}

func isOrthogonal(a1: dynamic, a2: dynamic, b1: dynamic, b2: dynamic) -> dynamic
{
  return isOrthogonal((a1 - a2), (b1 - b2));
}

func isOrthogonal(s1: dynamic, s2: dynamic) -> dynamic
{
  return equals(dot((s1.p2 - s1.p1), (s2.p2 - s2.p1)), 0.0);
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return equals(cross(a, b), 0.0);
}

func isParallel(a1: dynamic, a2: dynamic, b1: dynamic, b2: dynamic) -> dynamic
{
  return isParallel((a1 - a2), (b1 - b2));
}

func isParallel(s1: dynamic, s2: dynamic) -> dynamic
{
  return equals(cross((s1.p2 - s1.p1), (s2.p2 - s2.p1)), 0.0);
}

func project(s: dynamic, p: dynamic) -> dynamic
{
  var base: dynamic = (s.p2 - s.p1);
  var r: dynamic = (dot((p - s.p1), base) / norm(base));
  return (s.p1 + (base * r));
}

func reflect(s: dynamic, p: dynamic) -> dynamic
{
  return (p + (((project(s, p) - p)) * 2.0));
}

var COUNTER_CLOCKWISE: dynamic = 1;

var CLOCKWISE: dynamic = -1;

var ONLINE_BACK: dynamic = 2;

var ONLINE_FRONT: dynamic = -2;

var ON_SEGMENT: dynamic = 0;

func ccw(p0: dynamic, p1: dynamic, p2: dynamic) -> dynamic
{
  var a: dynamic = (p1 - p0);
  var b: dynamic = (p2 - p0);
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

func intersect(p1: dynamic, p2: dynamic, p3: dynamic, p4: dynamic) -> dynamic
{
  return ((((ccw(p1, p2, p3) * ccw(p1, p2, p4)) <= 0) && ((ccw(p3, p4, p1) * ccw(p3, p4, p2)) <= 0)));
}

func intersect(s1: dynamic, s2: dynamic) -> dynamic
{
  return intersect(s1.p1, s1.p2, s2.p1, s2.p2);
}

func getDistance(a: dynamic, b: dynamic) -> dynamic
{
  return abs((a - b));
}

func getDistanceLP(l: dynamic, p: dynamic) -> dynamic
{
  return abs((cross((l.p2 - l.p1), (p - l.p1)) / abs((l.p2 - l.p1))));
}

func getDistanceSP(s: dynamic, p: dynamic) -> dynamic
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

func getDistance(s1: dynamic, s2: dynamic) -> dynamic
{
  if (intersect(s1, s2))
  {
    return 0.0;
  }
  return min(min(getDistanceSP(s1, s2.p1), getDistanceSP(s1, s2.p2)), min(getDistanceSP(s2, s1.p1), getDistanceSP(s2, s1.p2)));
}

func getCrossPoint(s1: dynamic, s2: dynamic) -> dynamic
{
  var base: dynamic = (s2.p2 - s2.p1);
  var d1: dynamic = abs(cross(base, (s1.p1 - s2.p1)));
  var d2: dynamic = abs(cross(base, (s1.p2 - s2.p1)));
  var t: dynamic = (d1 / ((d1 + d2)));
  return (s1.p1 + (((s1.p2 - s1.p1)) * t));
}

func getCrossPointLL(l1: dynamic, l2: dynamic) -> dynamic
{
  var v1: dynamic = (l1.p2 - l1.p1);
  var v2: dynamic = (l2.p2 - l2.p1);
  var d: dynamic = cross(v2, v1);
  if ((abs(d) < EPS))
  {
    return l2.p1;
  }
  return (l1.p1 + ((v1 * cross(v2, (l2.p2 - l1.p1))) * ((1.0 / d))));
}

func getPerpendicularBisector(p1: dynamic, p2: dynamic) -> dynamic
{
  var c: dynamic = (((p1 + p2)) / 2.0);
  var q: dynamic = Point((c.x + ((p1.y - p2.y))), (c.y + ((p2.x - p1.x))));
  return Line(c, q);
}

func getArea(p: dynamic) -> dynamic
{
  var ret: dynamic = 0.0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(p.size())))
    {
      ret += cross(p[i], p[(((i + 1)) % p.size())]);
      i += 1;
    }
  }
  return (abs(ret) / 2.0);
}

var IN_POLYGON: dynamic = 2;

var ON_POLYGON: dynamic = 1;

var OUT_POLYGON: dynamic = 0;

func contains(g: dynamic, p: dynamic) -> dynamic
{
  var n: dynamic = g.size();
  var x: dynamic = false;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = (g[i] - p);
      var b: dynamic = (g[(((i + 1)) % n)] - p);
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
  return ( (x) ? IN_POLYGON : OUT_POLYGON);
}

func convexCut(s: dynamic, l: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(s.size())))
    {
      var a: dynamic = s[i];
      var b: dynamic = s[(((i + 1)) % s.size())];
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

func main() -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  write(fixed, setprecision(12));
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> N) >> M), N))
  {
    rep(i, N);
    read(l[i].x, l[i].y);
    rep(i, M);
    read(C[i].x, C[i].y);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var p: dynamic = l;
      rep(j, M);
      if ((i != j))
      {
        p = convexCut(p, getPerpendicularBisector(C[i], C[j]));
      }
      write(getArea(p), "\n");
    }
