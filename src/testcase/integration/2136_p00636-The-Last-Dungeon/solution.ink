// Translated from solution.cpp.

func dump()
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #de");
}

func repi(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=int(a);i<int(b);i++)");
}

func peri(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=int(b);i-->int(a);)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func per(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func all(c: dynamic)
{
  return cpp_expression("#include <bits/");
}

var mp = cpp_expression("#include");

var mt = cpp_expression("#include <");

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << cpp_char("(")) << p.first) << cpp_char(",")) << p.second) << cpp_char(")"));
}

func print_tuple(argument_0: dynamic, argument_1: dynamic)
{
}

func print_tuple(os: dynamic, t: dynamic)
{
  print_tuple(os, t);
  ((os << (if (cpp_sizeof(Cdr)) "," else "")) << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  print_tuple((os << cpp_char("(")), t);
  return (os << cpp_char(")"));
}

func operator_shift_left(os: dynamic, c: dynamic)
{
  (os << cpp_char("["));
  {
    var i = begin(c);
    while ((i != end(c)))
    {
      ((os << (if ((i == begin(c))) "" else " ")) << (*i));
      i += 1;
    }
  }
  return (os << cpp_char("]"));
}

var INF = 1e9;

var MOD = (1e9 + 7);

var EPS = 1e-9;

var PI = 3.141592653589793;

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point()
  {
    }
  func Point(x: dynamic, y: dynamic)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_add_assign(p: dynamic)
  {
      x += p.x;
      y += p.y;
      return (*this);
    }
  func operator_subtract_assign(p: dynamic)
  {
      x -= p.x;
      y -= p.y;
      return (*this);
    }
  func operator(c: dynamic)
  {
      x *= c;
      y *= c;
      return (*this);
    }
  func operator(c: dynamic)
  {
      x /= c;
      y /= c;
      return (*this);
    }
}

func operator_add(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "+=", b);
}

func operator_subtract(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "-=", b);
}

func operator_multiply(a: dynamic, c: dynamic)
{
  return cpp_assign(a, "*=", c);
}

func operator_multiply(c: dynamic, a: dynamic)
{
  return cpp_assign(a, "*=", c);
}

func operator_divide(a: dynamic, c: dynamic)
{
  return cpp_assign(a, "/=", c);
}

func operator_equal(a: dynamic, b: dynamic)
{
  return ((abs((a.x - b.x)) < EPS) && (abs((a.y - b.y)) < EPS));
}

func operator_not_equal(a: dynamic, b: dynamic)
{
  return (!((a == b)));
}

func operator_less(a: dynamic, b: dynamic)
{
  return if ((abs((a.x - b.x)) > EPS)) (a.x < b.x) else if ((abs((a.y - b.y)) > EPS)) (a.y < b.y) else false;
}

class Line
{
  var pos: dynamic;
  var dir: dynamic;
  func Line()
  {
    }
  func Line(p: dynamic, d: dynamic)
  {
      this->pos = cpp_construct(p);
      this->dir = cpp_construct(d);
    }
  func Line(x: dynamic, y: dynamic, u: dynamic, v: dynamic)
  {
      this->pos = cpp_construct(x, y);
      this->dir = cpp_construct(u, v);
    }
}

class Segment
{
  var pos: dynamic;
  var dir: dynamic;
  func Segment()
  {
    }
  func Segment(p: dynamic, d: dynamic)
  {
      this->pos = cpp_construct(p);
      this->dir = cpp_construct(d);
    }
  func Segment(x: dynamic, y: dynamic, u: dynamic, v: dynamic)
  {
      this->pos = cpp_construct(x, y);
      this->dir = cpp_construct(u, v);
    }
  func Segment(l: dynamic)
  {
      this->pos = cpp_construct(l.pos);
      this->dir = cpp_construct(l.dir);
    }
  func cpp_function_1()
  {
      return Line(pos, dir);
    }
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << cpp_char("(")) << p.x) << cpp_char(",")) << p.y) << cpp_char(")"));
}

func operator_shift_left(os: dynamic, l: dynamic)
{
  return (((((os << cpp_char("(")) << l.pos) << cpp_char(",")) << l.dir) << cpp_char(")"));
}

func operator_shift_left(os: dynamic, s: dynamic)
{
  return (((((os << cpp_char("(")) << s.pos) << cpp_char(",")) << s.dir) << cpp_char(")"));
}

func Signum(x: dynamic)
{
  return if ((x < (-EPS))) -1 else if ((x > EPS)) 1 else 0;
}

func Abs(p: dynamic)
{
  return sqrt(((p.x * p.x) + (p.y * p.y)));
}

func Abs2(p: dynamic)
{
  return ((p.x * p.x) + (p.y * p.y));
}

func Dot(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func Cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func Rot(p: dynamic, t: dynamic)
{
  return Point(((cos(t) * p.x) - (sin(t) * p.y)), ((sin(t) * p.x) + (cos(t) * p.y)));
}

func CCW(a: dynamic, b: dynamic, c: dynamic)
{
  b -= a;
  c -= a;
  if (true)
  {
    return sign;
  }
  if ((Dot(b, c) < (-EPS)))
  {
    return -2;
  }
  if ((Abs2(b) < (Abs2(c) - EPS)))
  {
    return 2;
  }
  return 0;
}

func IntersectSP(s: dynamic, p: dynamic)
{
  return (CCW(s.pos, (s.pos + s.dir), p) == 0);
}

func IntersectSS(a: dynamic, b: dynamic)
{
  var c1 = CCW(a.pos, (a.pos + a.dir), b.pos);
  var c2 = CCW(a.pos, (a.pos + a.dir), (b.pos + b.dir));
  var c3 = CCW(b.pos, (b.pos + b.dir), a.pos);
  var c4 = CCW(b.pos, (b.pos + b.dir), (a.pos + a.dir));
  return (((c1 * c2) <= 0) && ((c3 * c4) <= 0));
}

func InterPointLL(a: dynamic, b: dynamic)
{
  if ((abs(Cross(a.dir, b.dir)) < EPS))
  {
    return a.pos;
  }
  return (a.pos + ((Cross((b.pos - a.pos), b.dir) / Cross(a.dir, b.dir)) * a.dir));
}

func InterPointLS(l: dynamic, s: dynamic)
{
  return InterPointLL(Line(s), l);
}

func InterPointSS(a: dynamic, b: dynamic)
{
  if ((abs(Cross(a.dir, b.dir)) < EPS))
  {
    if (IntersectSP(b, a.pos))
    {
      return a.pos;
    }
    if (IntersectSP(b, (a.pos + a.dir)))
    {
      return (a.pos + a.dir);
    }
    if (IntersectSP(a, b.pos))
    {
      return b.pos;
    }
    if (IntersectSP(a, (b.pos + b.dir)))
    {
      return (b.pos + b.dir);
    }
  }
  return InterPointLL(Line(a), Line(b));
}

func ConvexCut(ps: dynamic, l: dynamic)
{
  var n = ps.size();
  var res: dynamic;
  return res;
}

class Edge
{
  var src: dynamic;
  var dst: dynamic;
  var weight: dynamic;
  func Edge()
  {
    }
  func Edge(s: dynamic, d: dynamic, w: dynamic)
  {
      this->src = cpp_construct(s);
      this->dst = cpp_construct(d);
      this->weight = cpp_construct(w);
    }
  func operator_less(e: dynamic)
  {
      return (Signum((weight - e.weight)) < 0);
    }
  func operator_greater(e: dynamic)
  {
      return (Signum((weight - e.weight)) > 0);
    }
}

func SegmentArrangement(ss: dynamic, g: dynamic, ps: dynamic)
{
  rep(i, ss.size());
  {
    ps.push_back(ss[i].pos);
    ps.push_back((ss[i].pos + ss[i].dir));
    repi(j, (i + 1), ss.size());
    if (IntersectSS(ss[i], ss[j]))
    {
      ps.push_back(InterPointSS(ss[i], ss[j]));
    }
  }
  sort(all(ps));
  ps.erase(unique(all(ps)), ps.end());
  g.resize(ps.size());
  rep(i, ss.size());
  {
    var ds: dynamic;
    rep(j, ps.size());
    if (IntersectSP(ss[i], ps[j]))
    {
      ds.push_back(mp(Abs((ps[j] - ss[i].pos)), j));
    }
    sort(all(ds));
    rep(j, (ds.size() - 1));
    {
      var u = ds[j].second;
      var v = ds[(j + 1)].second;
      var w = (ds[(j + 1)].first - ds[j].first);
      g[u].push_back(Edge(u, v, w));
      g[v].push_back(Edge(v, u, w));
    }
  }
}

func Dijkstra(g: dynamic, v: dynamic, dist: dynamic)
{
  var pq: dynamic;
  pq.emplace(-1, v, 0);
  while (pq.size())
  {
    var cur = pq.top();
    pq.pop();
    if ((dist[cur.dst] != INF))
    {
      continue;
    }
    dist[cur.dst] = cur.weight;
    for (var e in g[cur.dst])
    {
      pq.emplace(e.src, e.dst, (cur.weight + e.weight));
    }
  }
}

func main()
{
  {
    var n: dynamic;
    while (((cin >> n) && n))
    {
      for (var p in ps)
      {
        read(p.x, p.y);
      }
      var ss: dynamic;
      {
        var tmp: dynamic;
        for (var p in tmp)
        {
          ss.emplace_back(p.first, (p.second - p.first));
        }
      }
      var g: dynamic;
      var qs: dynamic;
      SegmentArrangement(ss, g, qs);
      var dist = cpp_construct(qs.size(), INF);
      var pq: dynamic;
      rep(i, qs.size());
      if ((abs(qs[i].x) < EPS))
      {
        pq.emplace(-1, i, 0);
      }
      while (pq.size())
      {
        var cur = pq.top();
        pq.pop();
        if ((dist[cur.dst] != INF))
        {
          continue;
        }
        dist[cur.dst] = cur.weight;
        for (var e in g[cur.dst])
        {
          pq.emplace(e.src, e.dst, (cur.weight + e.weight));
        }
      }
      var res = INF;
      rep(i, qs.size());
      if ((abs((qs[i].x - 4)) < EPS))
      {
        res = min(res, dist[i]);
      }
      if ((res == INF))
      {
        puts("impossible");
      } else
      {
        printf("%.12f\n", res);
      }
    }
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var c1 = CCW(l.pos, (l.pos + l.dir), ps[i]);
    var c2 = CCW(l.pos, (l.pos + l.dir), ps[(((i + 1)) % n)]);
    if ((c1 != -1))
    {
      res.push_back(ps[i]);
    }
    if (((c1 * c2) == -1))
    {
      res.push_back(InterPointLS(l, Segment(ps[i], (ps[(((i + 1)) % n)] - ps[i]))));
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
          var cs = [Point(0, 0), Point(4, 0), Point(4, 4), Point(0, 4)];
          rep(j, n);
          if ((j != i))
          {
            var l = cpp_construct((((ps[j] + ps[i])) / 2), Rot((ps[j] - ps[i]), (PI / 2)));
            cs = ConvexCut(cs, l);
          }
          rep(i, cs.size());
          {
            var p1 = cs[i];
            var p2 = cs[(((i + 1)) % cs.size())];
            if (((((abs(p1.y) < EPS) || (abs((p1.y - 4)) < EPS))) && (abs((p2.y - p1.y)) < EPS)))
            {
              continue;
            }
            tmp.insert(mp(min(p1, p2), max(p1, p2)));
          }
        }
