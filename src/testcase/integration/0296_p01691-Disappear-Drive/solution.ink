// Translated from solution.cpp.

func dump()
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #define dump(.");
}

class DUMP
{
  func operator(t: dynamic)
  {
      if (this->tellp())
      {
        ((*this) << ", ");
      }
      ((*this) << t);
      return (*this);
    }
}

var EPS = 1e-8;

class point
{
  var x: dynamic;
  var y: dynamic;
  func point(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func point(p: dynamic)
  {
      this->x = cpp_construct(p.x);
      this->y = cpp_construct(p.y);
    }
  func operator_add(p: dynamic)
  {
      return point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic)
  {
      return point((x - p.x), (y - p.y));
    }
  func operator_multiply(s: dynamic)
  {
      return point((x * s), (y * s));
    }
  func operator_multiply(p: dynamic)
  {
      return point(((x * p.x) - (y * p.y)), ((x * p.y) + (y * p.x)));
    }
  func operator_divide(s: dynamic)
  {
      return point((x / s), (y / s));
    }
  func operator_less(p: dynamic)
  {
      return (((x + EPS) < p.x) || (((abs((x - p.x)) < EPS) && ((y + EPS) < p.y))));
    }
  func operator_equal(p: dynamic)
  {
      return ((abs((x - p.x)) < EPS) && (abs((y - p.y)) < EPS));
    }
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << cpp_char("(")) << p.x) << ", ") << p.y) << cpp_char(")"));
}

func rotate90(p: dynamic)
{
  return point((-p.y), p.x);
}

func norm(p: dynamic)
{
  return ((p.x * p.x) + (p.y * p.y));
}

func abs(p: dynamic)
{
  return sqrt(norm(p));
}

func dot(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

class line
{
  var a: dynamic;
  var b: dynamic;
  func line(a: dynamic, b: dynamic)
  {
      this->a = cpp_construct(a);
      this->b = cpp_construct(b);
    }
}

class segment
{
  var a: dynamic;
  var b: dynamic;
  func segment(a: dynamic, b: dynamic)
  {
      this->a = cpp_construct(a);
      this->b = cpp_construct(b);
    }
}

class circle
{
  var c: dynamic;
  var r: dynamic;
  func circle(c: dynamic, r: dynamic)
  {
      this->c = cpp_construct(c);
      this->r = cpp_construct(r);
    }
}

func dist(a: dynamic, b: dynamic)
{
  return abs((a - b));
}

func dist(l: dynamic, p: dynamic)
{
  return (abs(cross((l.b - l.a), (p - l.a))) / abs((l.b - l.a)));
}

func dist(s: dynamic, p: dynamic)
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

func intersect(c: dynamic, p: dynamic)
{
  return ((dist(p, c.c) + EPS) < c.r);
}

func intersect(c: dynamic, s: dynamic)
{
  return ((dist(s, c.c) + EPS) < c.r);
}

func crosspoint(a: dynamic, b: dynamic)
{
  var tmp = cross((a.b - a.a), (b.b - b.a));
  if ((abs(tmp) < EPS))
  {
    return a.a;
  }
  return (b.a + ((((b.b - b.a)) * cross((a.b - a.a), (a.a - b.a))) / tmp));
}

func tangent(c: dynamic, p: dynamic)
{
  var x = norm((p - c.c));
  var d = (x - (c.r * c.r));
  if ((d < (-EPS)))
  {
    return vector();
  }
  d = max(d, 0.0);
  var p1 = (((p - c.c)) * (((c.r * c.r) / x)));
  var p2 = rotate90((((p - c.c)) * ((((-c.r) * sqrt(d)) / x))));
  var res: dynamic;
  res.push_back(((c.c + p1) - p2));
  res.push_back(((c.c + p1) + p2));
  return res;
}

var W = 50.0;

var H = 94.0;

var INF = INT_MAX;

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func out(p: dynamic)
{
  return ((((p.x < EPS) || (p.y < EPS)) || ((p.x + EPS) > W)) || ((p.y + EPS) > H));
}

func get_line(c: dynamic, p: dynamic)
{
  var ts = tangent(c, p);
  var res: dynamic;
  for (var t in ts)
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

func main()
{
  var start = cpp_construct((W / 2.0), 0);
  var goal = cpp_construct((W / 2.0), H);
  cout.flags(ios.fixed);
  cout.precision(10);
  var N: dynamic;
  var D: dynamic;
  read(N, D);
  var circles: dynamic;
  circles.reserve(N);
  {
    var i = 0;
    while ((i < N))
    {
      var x: dynamic;
      var y: dynamic;
      var r: dynamic;
      read(x, y, r);
      circles.emplace_back(point(x, y), r);
      i += 1;
    }
  }
  var cnt = 0;
  for (var c in circles)
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
  var s_lines: dynamic;
  var g_lines: dynamic;
  s_lines.reserve((2 * N));
  g_lines.reserve((2 * N));
  for (var c in circles)
  {
    var ls = get_line(c, start);
    s_lines.insert(s_lines.end(), ls.begin(), ls.end());
    ls = get_line(c, goal);
    g_lines.insert(g_lines.end(), ls.begin(), ls.end());
  }
  var ans = INF;
  for (var s_l in s_lines)
  {
    for (var g_l in g_lines)
    {
      var cp = crosspoint(s_l, g_l);
      if (out(cp))
      {
        continue;
      }
      cnt = 0;
      for (var c in circles)
      {
        var i1 = intersect(c, s1);
        var i2 = intersect(c, s2);
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
