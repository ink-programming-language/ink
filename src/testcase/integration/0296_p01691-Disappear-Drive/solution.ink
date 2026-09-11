// Translated from solution.cpp.

func dump() -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #define dump(.");
}

class DUMP
{
  func operator(t: dynamic) -> dynamic
  {
      if (self->tellp())
      {
        ((*self) << ", ");
      }
      ((*self) << t);
      return (*self);
    }
}

var EPS: dynamic = 1e-8;

class point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func point(x: dynamic = 0, y: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func point(p: dynamic) -> dynamic
  {
      self->x = cpp_construct(p.x);
      self->y = cpp_construct(p.y);
    }
  func operator_add(p: dynamic) -> dynamic
  {
      return point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic) -> dynamic
  {
      return point((x - p.x), (y - p.y));
    }
  func operator_multiply(s: dynamic) -> dynamic
  {
      return point((x * s), (y * s));
    }
  func operator_multiply(p: dynamic) -> dynamic
  {
      return point(((x * p.x) - (y * p.y)), ((x * p.y) + (y * p.x)));
    }
  func operator_divide(s: dynamic) -> dynamic
  {
      return point((x / s), (y / s));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      return (((x + EPS) < p.x) || (((abs((x - p.x)) < EPS) && ((y + EPS) < p.y))));
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((abs((x - p.x)) < EPS) && (abs((y - p.y)) < EPS));
    }
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << p.x) << ", ") << p.y) << cpp_char(")"));
}

func rotate90(p: dynamic) -> dynamic
{
  return point((-p.y), p.x);
}

func norm(p: dynamic) -> dynamic
{
  return ((p.x * p.x) + (p.y * p.y));
}

func abs(p: dynamic) -> dynamic
{
  return sqrt(norm(p));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

class line
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func line(a: dynamic, b: dynamic) -> dynamic
  {
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
    }
}

class segment
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func segment(a: dynamic, b: dynamic) -> dynamic
  {
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
    }
}

class circle
{
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func circle(c: dynamic, r: dynamic) -> dynamic
  {
      self->c = cpp_construct(c);
      self->r = cpp_construct(r);
    }
}

func dist(a: dynamic, b: dynamic) -> dynamic
{
  return abs((a - b));
}

func dist(l: dynamic, p: dynamic) -> dynamic
{
  return (abs(cross((l.b - l.a), (p - l.a))) / abs((l.b - l.a)));
}

func dist(s: dynamic, p: dynamic) -> dynamic
{
  if ((dot((s.b - s.a), (p - s.a)) < 0))
  {
    return dist(p, s.a);
  }
  if ((dot((s.a - s.b), (p - s.b)) < 0))
  {
    return dist(p, s.b);
  }
  return dist(line(s.a, s.b), p);
}

func intersect(c: dynamic, p: dynamic) -> dynamic
{
  return ((dist(p, c.c) + EPS) < c.r);
}

func intersect(c: dynamic, s: dynamic) -> dynamic
{
  return ((dist(s, c.c) + EPS) < c.r);
}

func crosspoint(a: dynamic, b: dynamic) -> dynamic
{
  var tmp: dynamic = cross((a.b - a.a), (b.b - b.a));
  if ((abs(tmp) < EPS))
  {
    return a.a;
  }
  return (b.a + ((((b.b - b.a)) * cross((a.b - a.a), (a.a - b.a))) / tmp));
}

func tangent(c: dynamic, p: dynamic) -> dynamic
{
  var x: dynamic = norm((p - c.c));
  var d: dynamic = (x - (c.r * c.r));
  if ((d < (-EPS)))
  {
    return vector();
  }
  d = max(d, 0.0);
  var p1: dynamic = (((p - c.c)) * (((c.r * c.r) / x)));
  var p2: dynamic = rotate90((((p - c.c)) * ((((-c.r) * sqrt(d)) / x))));
  var res: dynamic = cpp_uninitialized();
  res.push_back(((c.c + p1) - p2));
  res.push_back(((c.c + p1) + p2));
  return res;
}

var W: dynamic = 50.0;

var H: dynamic = 94.0;

var INF: dynamic = INT_MAX;

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func out(p: dynamic) -> dynamic
{
  return ((((p.x < EPS) || (p.y < EPS)) || ((p.x + EPS) > W)) || ((p.y + EPS) > H));
}

func get_line(c: dynamic, p: dynamic) -> dynamic
{
  var ts: dynamic = tangent(c, p);
  var res: dynamic = cpp_uninitialized();
  for (var t: dynamic in ts)
  {
    if ((t == p))
    {
      res.emplace_back(p, (p + rotate90((c.c - p))));
    } else
    {
      res.emplace_back(p, t);
    }
  }
  return res;
}

func main() -> dynamic
{
  var start: dynamic = cpp_construct((W / 2.0), 0);
  var goal: dynamic = cpp_construct((W / 2.0), H);
  cout.flags(ios.fixed);
  cout.precision(10);
  var N: dynamic = cpp_uninitialized();
  var D: dynamic = cpp_uninitialized();
  read(N, D);
  var circles: dynamic = cpp_uninitialized();
  circles.reserve(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      read(x, y, r);
      circles.emplace_back(point(x, y), r);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  for (var c: dynamic in circles)
  {
    if (intersect(c, segment(start, goal)))
    {
      cnt += 1;
    }
  }
  if ((cnt <= D))
  {
    write(94.0, "\n");
    return 0;
  }
  var s_lines: dynamic = cpp_uninitialized();
  var g_lines: dynamic = cpp_uninitialized();
  s_lines.reserve((2 * N));
  g_lines.reserve((2 * N));
  for (var c: dynamic in circles)
  {
    var ls: dynamic = get_line(c, start);
    s_lines.insert(s_lines.end(), ls.begin(), ls.end());
    ls = get_line(c, goal);
    g_lines.insert(g_lines.end(), ls.begin(), ls.end());
  }
  var ans: dynamic = INF;
  for (var s_l: dynamic in s_lines)
  {
    for (var g_l: dynamic in g_lines)
    {
      var cp: dynamic = crosspoint(s_l, g_l);
      if (out(cp))
      {
        continue;
      }
      cnt = 0;
      for (var c: dynamic in circles)
      {
        var i1: dynamic = intersect(c, s1);
        var i2: dynamic = intersect(c, s2);
        if (((i1 && i2) && (!intersect(c, cp))))
        {
          cnt += 2;
        } else if ((i1 || i2))
        {
          cnt += 1;
        }
      }
      if ((cnt <= D))
      {
        chmin(ans, (dist(start, cp) + dist(cp, goal)));
      }
    }
  }
  if ((ans == INF))
  {
    puts("-1");
  } else
  {
    write(ans, "\n");
  }
  return 0;
}
