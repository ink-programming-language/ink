// Translated from solution.cpp.

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

var inf = 1e9;

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

func mergeIfAble(s1: dynamic, s2: dynamic)
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

func mergeSegments(segs: dynamic)
{
  {
    var i = 0;
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
    var i = 0;
    while ((i < segs.size()))
    {
      {
        var j = (i + 1);
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
  var to: dynamic;
  var cost: dynamic;
  func edge()
  {
    }
  func edge(to: dynamic, cost: dynamic)
  {
      this->to = cpp_construct(to);
      this->cost = cpp_construct(cost);
    }
  func operator_less(e: dynamic)
  {
      return lt(cost, e.cost);
    }
}

func segmentArrangement(segs: dynamic, ps: dynamic)
{
  {
    var i = 0;
    while ((i < segs.size()))
    {
      ps.push_back(segs[i].p1);
      ps.push_back(segs[i].p2);
      {
        var j = (i + 1);
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
  var graph = cpp_construct(ps.size());
  {
    var i = 0;
    while ((i < segs.size()))
    {
      var ls: dynamic;
      {
        var j = 0;
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
        var j = 0;
        while (((j + 1) < ls.size()))
        {
          var u = ls[j].second;
          var v = ls[(j + 1)].second;
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

func dijkstra(p: dynamic, a: dynamic, b: dynamic)
{
  var que: dynamic;
  var mincost = cpp_construct(p.size(), inf);
  que.push(P(0.0, 0));
  mincost[0] = 0.0;
  while ((!que.empty()))
  {
    var c: dynamic;
    var now: dynamic;
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
      var i = 0;
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

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  write(fixed, setprecision(12));
  var Na: dynamic;
  var Nb: dynamic;
  read(Na, Nb);
  rep(i, Na);
  read(pa[i].x, pa[i].y);
  rep(i, Nb);
  read(pb[i].x, pb[i].y);
  var a_st = dijkstra(pb, pa[0], pa[1]);
  var b_st = dijkstra(pa, pb[0], pb[1]);
  if (((a_st != inf) || (b_st != inf)))
  {
    write(min(a_st, b_st), "\n");
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
