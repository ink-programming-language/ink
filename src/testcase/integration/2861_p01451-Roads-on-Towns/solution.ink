// Translated from solution.cpp.

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

var inf: dynamic = 1e9;

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

func mergeIfAble(s1: dynamic, s2: dynamic) -> dynamic
{
  if ((abs(cross((s1.p2 - s1.p1), (s2.p2 - s2.p1))) > EPS))
  {
    return false;
  }
  if (((ccw(s1.p1, s2.p1, s1.p2) == COUNTER_CLOCKWISE) || (ccw(s1.p1, s2.p1, s1.p2) == CLOCKWISE)))
  {
    return false;
  }
  if (((ccw(s1.p1, s1.p2, s2.p1) == ONLINE_FRONT) || (ccw(s2.p1, s2.p2, s1.p1) == ONLINE_FRONT)))
  {
    return false;
  }
  s1 = Segment(min(s1.p1, s2.p1), max(s1.p2, s2.p2));
  return true;
}

func mergeSegments(segs: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < segs.size()))
    {
      if ((segs[i].p2 < segs[i].p1))
      {
        swap(segs[i].p1, segs[i].p2);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < segs.size()))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < segs.size()))
        {
          if (mergeIfAble(segs[i], segs[j]))
          {
            segs[cpp_update(j, "--")] = segs.back();
            segs.pop_back();
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

class edge
{
  var to: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  func edge() -> dynamic
  {
    }
  func edge(to: dynamic, cost: dynamic) -> dynamic
  {
      self->to = cpp_construct(to);
      self->cost = cpp_construct(cost);
    }
  func operator_less(e: dynamic) -> dynamic
  {
      return lt(cost, e.cost);
    }
}

func segmentArrangement(segs: dynamic, ps: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < segs.size()))
    {
      ps.push_back(segs[i].p1);
      ps.push_back(segs[i].p2);
      {
        var j: dynamic = (i + 1);
        while ((j < segs.size()))
        {
          if (intersect(segs[i], segs[j]))
          {
            ps.push_back(getCrossPoint(segs[i], segs[j]));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(ps.begin(), ps.end());
  ps.erase(unique(ps.begin(), ps.end()), ps.end());
  var graph: dynamic = cpp_construct(ps.size());
  {
    var i: dynamic = 0;
    while ((i < segs.size()))
    {
      var ls: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < ps.size()))
        {
          if (intersect(segs[i], ps[j]))
          {
            ls.emplace_back(getDistanceSP(segs[i], ps[j]), j);
          }
          j += 1;
        }
      }
      sort(ls.begin(), ls.end());
      {
        var j: dynamic = 0;
        while (((j + 1) < ls.size()))
        {
          var u: dynamic = ls[j].second;
          var v: dynamic = ls[(j + 1)].second;
          graph[u].emplace_back(v, getDistance(ps[u], ps[v]));
          graph[v].emplace_back(u, getDistance(ps[u], ps[v]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return graph;
}

func dijkstra(p: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  var que: dynamic = cpp_uninitialized();
  var mincost: dynamic = cpp_construct(p.size(), inf);
  que.push(P(0.0, 0));
  mincost[0] = 0.0;
  while ((!que.empty()))
  {
    var c: dynamic = cpp_uninitialized();
    var now: dynamic = cpp_uninitialized();
    tie(c, now) = que.top();
    que.pop();
    if ((now == 1))
    {
      return (c + getDistance(a, b));
    }
    if ((mincost[now] < c))
    {
      continue;
    }
    {
      var i: dynamic = 0;
      while ((i < p.size()))
      {
        if (intersect(p[now], p[i], a, b))
        {
          i += 1;
          continue;
        }
        if (((getDistance(p[now], p[i]) + c) < mincost[i]))
        {
          mincost[i] = (getDistance(p[now], p[i]) + c);
          que.push(P(mincost[i], i));
        }
        i += 1;
      }
    }
  }
  return inf;
}

func main() -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  write(fixed, setprecision(12));
  var Na: dynamic = cpp_uninitialized();
  var Nb: dynamic = cpp_uninitialized();
  read(Na, Nb);
  rep(i, Na);
  read(pa[i].x, pa[i].y);
  rep(i, Nb);
  read(pb[i].x, pb[i].y);
  var a_st: dynamic = dijkstra(pb, pa[0], pa[1]);
  var b_st: dynamic = dijkstra(pa, pb[0], pb[1]);
  if (((a_st != inf) || (b_st != inf)))
  {
    write(min(a_st, b_st), "\n");
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
