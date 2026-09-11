// Translated from solution.cpp.

func dump() -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #de");
}

func repi(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=int(a);i<int(b);i++)");
}

func peri(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=int(b);i-->int(a);)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

func per(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

var mp: dynamic = cpp_expression("#include");

var mt: dynamic = cpp_expression("#include <");

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << p.first) << cpp_char(",")) << p.second) << cpp_char(")"));
}

func print_tuple(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
}

func print_tuple(os: dynamic, t: dynamic) -> dynamic
{
  print_tuple(os, t);
  ((os << ( (cpp_sizeof(Cdr)) ? "," : "")) << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  print_tuple((os << cpp_char("(")), t);
  return (os << cpp_char(")"));
}

func operator_shift_left(os: dynamic, c: dynamic) -> dynamic
{
  (os << cpp_char("["));
  {
    var i: dynamic = begin(c);
    while ((i != end(c)))
    {
      ((os << ( ((i == begin(c))) ? "" : " ")) << (*i));
      i += 1;
    }
  }
  return (os << cpp_char("]"));
}

var INF: dynamic = 1e9;

var MOD: dynamic = (1e9 + 7);

var EPS: dynamic = 1e-9;

var PI: dynamic = 3.141592653589793;

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point() -> dynamic
  {
    }
  func Point(x: dynamic, y: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func operator_add_assign(p: dynamic) -> dynamic
  {
      x += p.x;
      y += p.y;
      return (*self);
    }
  func operator_subtract_assign(p: dynamic) -> dynamic
  {
      x -= p.x;
      y -= p.y;
      return (*self);
    }
  func operator(c: dynamic) -> dynamic
  {
      x *= c;
      y *= c;
      return (*self);
    }
  func operator(c: dynamic) -> dynamic
  {
      x /= c;
      y /= c;
      return (*self);
    }
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "+=", b);
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "-=", b);
}

func operator_multiply(a: dynamic, c: dynamic) -> dynamic
{
  return cpp_assign(a, "*=", c);
}

func operator_multiply(c: dynamic, a: dynamic) -> dynamic
{
  return cpp_assign(a, "*=", c);
}

func operator_divide(a: dynamic, c: dynamic) -> dynamic
{
  return cpp_assign(a, "/=", c);
}

func operator_equal(a: dynamic, b: dynamic) -> dynamic
{
  return ((abs((a.x - b.x)) < EPS) && (abs((a.y - b.y)) < EPS));
}

func operator_not_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (!((a == b)));
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((abs((a.x - b.x)) > EPS)) ? (a.x < b.x) :  ((abs((a.y - b.y)) > EPS)) ? (a.y < b.y) : false;
}

class Line
{
  var pos: dynamic = cpp_uninitialized();
  var dir: dynamic = cpp_uninitialized();
  func Line() -> dynamic
  {
    }
  func Line(p: dynamic, d: dynamic) -> dynamic
  {
      self->pos = cpp_construct(p);
      self->dir = cpp_construct(d);
    }
  func Line(x: dynamic, y: dynamic, u: dynamic, v: dynamic) -> dynamic
  {
      self->pos = cpp_construct(x, y);
      self->dir = cpp_construct(u, v);
    }
}

class Segment
{
  var pos: dynamic = cpp_uninitialized();
  var dir: dynamic = cpp_uninitialized();
  func Segment() -> dynamic
  {
    }
  func Segment(p: dynamic, d: dynamic) -> dynamic
  {
      self->pos = cpp_construct(p);
      self->dir = cpp_construct(d);
    }
  func Segment(x: dynamic, y: dynamic, u: dynamic, v: dynamic) -> dynamic
  {
      self->pos = cpp_construct(x, y);
      self->dir = cpp_construct(u, v);
    }
  func Segment(l: dynamic) -> dynamic
  {
      self->pos = cpp_construct(l.pos);
      self->dir = cpp_construct(l.dir);
    }
  func cpp_function_1() -> dynamic
  {
      return Line(pos, dir);
    }
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << p.x) << cpp_char(",")) << p.y) << cpp_char(")"));
}

func operator_shift_left(os: dynamic, l: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << l.pos) << cpp_char(",")) << l.dir) << cpp_char(")"));
}

func operator_shift_left(os: dynamic, s: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << s.pos) << cpp_char(",")) << s.dir) << cpp_char(")"));
}

func Signum(x: dynamic) -> dynamic
{
  return  ((x < (-EPS))) ? -1 :  ((x > EPS)) ? 1 : 0;
}

func Abs(p: dynamic) -> dynamic
{
  return sqrt(((p.x * p.x) + (p.y * p.y)));
}

func Abs2(p: dynamic) -> dynamic
{
  return ((p.x * p.x) + (p.y * p.y));
}

func Dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func Cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func Rot(p: dynamic, t: dynamic) -> dynamic
{
  return Point(((cos(t) * p.x) - (sin(t) * p.y)), ((sin(t) * p.x) + (cos(t) * p.y)));
}

func CCW(a: dynamic, b: dynamic, c: dynamic) -> dynamic
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

func IntersectSP(s: dynamic, p: dynamic) -> dynamic
{
  return (CCW(s.pos, (s.pos + s.dir), p) == 0);
}

func IntersectSS(a: dynamic, b: dynamic) -> dynamic
{
  var c1: dynamic = CCW(a.pos, (a.pos + a.dir), b.pos);
  var c2: dynamic = CCW(a.pos, (a.pos + a.dir), (b.pos + b.dir));
  var c3: dynamic = CCW(b.pos, (b.pos + b.dir), a.pos);
  var c4: dynamic = CCW(b.pos, (b.pos + b.dir), (a.pos + a.dir));
  return (((c1 * c2) <= 0) && ((c3 * c4) <= 0));
}

func InterPointLL(a: dynamic, b: dynamic) -> dynamic
{
  if ((abs(Cross(a.dir, b.dir)) < EPS))
  {
    return a.pos;
  }
  return (a.pos + ((Cross((b.pos - a.pos), b.dir) / Cross(a.dir, b.dir)) * a.dir));
}

func InterPointLS(l: dynamic, s: dynamic) -> dynamic
{
  return InterPointLL(Line(s), l);
}

func InterPointSS(a: dynamic, b: dynamic) -> dynamic
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

func ConvexCut(ps: dynamic, l: dynamic) -> dynamic
{
  var n: dynamic = ps.size();
  var res: dynamic = cpp_uninitialized();
  return res;
}

class Edge
{
  var src: dynamic = cpp_uninitialized();
  var dst: dynamic = cpp_uninitialized();
  var weight: dynamic = cpp_uninitialized();
  func Edge() -> dynamic
  {
    }
  func Edge(s: dynamic, d: dynamic, w: dynamic) -> dynamic
  {
      self->src = cpp_construct(s);
      self->dst = cpp_construct(d);
      self->weight = cpp_construct(w);
    }
  func operator_less(e: dynamic) -> dynamic
  {
      return (Signum((weight - e.weight)) < 0);
    }
  func operator_greater(e: dynamic) -> dynamic
  {
      return (Signum((weight - e.weight)) > 0);
    }
}

func SegmentArrangement(ss: dynamic, g: dynamic, ps: dynamic) -> dynamic
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
    var ds: dynamic = cpp_uninitialized();
    rep(j, ps.size());
    if (IntersectSP(ss[i], ps[j]))
    {
      ds.push_back(mp(Abs((ps[j] - ss[i].pos)), j));
    }
    sort(all(ds));
    rep(j, (ds.size() - 1));
    {
      var u: dynamic = ds[j].second;
      var v: dynamic = ds[(j + 1)].second;
      var w: dynamic = (ds[(j + 1)].first - ds[j].first);
      g[u].push_back(Edge(u, v, w));
      g[v].push_back(Edge(v, u, w));
    }
  }
}

func Dijkstra(g: dynamic, v: dynamic, dist: dynamic) -> dynamic
{
  var pq: dynamic = cpp_uninitialized();
  pq.emplace(-1, v, 0);
  while (pq.size())
  {
    var cur: dynamic = pq.top();
    pq.pop();
    if ((dist[cur.dst] != INF))
    {
      continue;
    }
    dist[cur.dst] = cur.weight;
    for (var e: dynamic in g[cur.dst])
    {
      pq.emplace(e.src, e.dst, (cur.weight + e.weight));
    }
  }
}

func main() -> dynamic
{
  {
    var n: dynamic = cpp_uninitialized();
    while (((cin >> n) && n))
    {
      for (var p: dynamic in ps)
      {
        read(p.x, p.y);
      }
      var ss: dynamic = cpp_uninitialized();
      {
        var tmp: dynamic = cpp_uninitialized();
        for (var p: dynamic in tmp)
        {
          ss.emplace_back(p.first, (p.second - p.first));
        }
      }
      var g: dynamic = cpp_uninitialized();
      var qs: dynamic = cpp_uninitialized();
      SegmentArrangement(ss, g, qs);
      var dist: dynamic = cpp_construct(qs.size(), INF);
      var pq: dynamic = cpp_uninitialized();
      rep(i, qs.size());
      if ((abs(qs[i].x) < EPS))
      {
        pq.emplace(-1, i, 0);
      }
      while (pq.size())
      {
        var cur: dynamic = pq.top();
        pq.pop();
        if ((dist[cur.dst] != INF))
        {
          continue;
        }
        dist[cur.dst] = cur.weight;
        for (var e: dynamic in g[cur.dst])
        {
          pq.emplace(e.src, e.dst, (cur.weight + e.weight));
        }
      }
      var res: dynamic = INF;
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var c1: dynamic = CCW(l.pos, (l.pos + l.dir), ps[i]);
    var c2: dynamic = CCW(l.pos, (l.pos + l.dir), ps[(((i + 1)) % n)]);
    if ((c1 != -1))
    {
      res.push_back(ps[i]);
    }
    if (((c1 * c2) == -1))
    {
      res.push_back(InterPointLS(l, Segment(ps[i], (ps[(((i + 1)) % n)] - ps[i]))));
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          var cs: dynamic = [Point(0, 0), Point(4, 0), Point(4, 4), Point(0, 4)];
          rep(j, n);
          if ((j != i))
          {
            var l: dynamic = cpp_construct((((ps[j] + ps[i])) / 2), Rot((ps[j] - ps[i]), (PI / 2)));
            cs = ConvexCut(cs, l);
          }
          rep(i, cs.size());
          {
            var p1: dynamic = cs[i];
            var p2: dynamic = cs[(((i + 1)) % cs.size())];
            if (((((abs(p1.y) < EPS) || (abs((p1.y - 4)) < EPS))) && (abs((p2.y - p1.y)) < EPS)))
            {
              continue;
            }
            tmp.insert(mp(min(p1, p2), max(p1, p2)));
          }
        }
