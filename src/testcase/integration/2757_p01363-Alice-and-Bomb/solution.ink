// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#inc");

var inf: dynamic = cpp_expression("#incl");

var linf: dynamic = cpp_expression("#inc");

var eps: dynamic = cpp_expression("#inclu");

var mod: dynamic = cpp_expression("#include<b");

var pi: dynamic = cpp_expression("#include");

var phi: dynamic = cpp_expression("#include<bits/std");

var f: dynamic = cpp_expression("#incl");

var s: dynamic = cpp_expression("#inclu");

var mp: dynamic = cpp_expression("#include<");

var pb: dynamic = cpp_expression("#include<");

func all(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func pd(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h> #defi");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);i++)");
}

func RFOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a)-1;(b)<=i;i--)");
}

func equals(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc+");
}

var dx: dynamic = [1, 0, -1, 0, 1, 1, -1, -1];

var dy: dynamic = [0, 1, 0, -1, 1, -1, 1, -1];

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0, y: dynamic = 0) -> dynamic
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
  func operator_multiply(k: dynamic) -> dynamic
  {
      return Point((x * k), (y * k));
    }
  func operator_divide(k: dynamic) -> dynamic
  {
      return Point((x / k), (y / k));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      return  (equals(x, p.x)) ? ((y - p.y) < (-eps)) : ((x - p.x) < (-eps));
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((fabs((x - p.x)) < eps) && (fabs((y - p.y)) < eps));
    }
  func abs() -> dynamic
  {
      return sqrt(norm());
    }
  func norm() -> dynamic
  {
      return (((x * x) + (y * y)));
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

func norm(a: dynamic) -> dynamic
{
  return (((a.x * a.x) + (a.y * a.y)));
}

func abs(a: dynamic) -> dynamic
{
  return sqrt(norm(a));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return (((a.x * b.x) + (a.y * b.y)));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return (((a.x * b.y) - (a.y * b.x)));
}

func project(s: dynamic, p: dynamic) -> dynamic
{
  var base: dynamic = ((s.p2 - s.p1));
  var r: dynamic = ((dot((p - s.p1), base) / base.norm()));
  return ((s.p1 + (base * r)));
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return equals(cross(a, b), 0.0);
}

func isParallel(s: dynamic, t: dynamic) -> dynamic
{
  return equals(cross((s.p1 - s.p2), (t.p1 - t.p2)), 0.0);
}

func intersect(a: dynamic, b: dynamic) -> dynamic
{
  if ((((cross((a.p2 - a.p1), (b.p1 - a.p1)) * cross((a.p2 - a.p1), (b.p2 - a.p1))) < (-eps)) && ((cross((b.p2 - b.p1), (a.p1 - b.p1)) * cross((b.p2 - b.p1), (a.p2 - b.p1))) < (-eps))))
  {
    return true;
  }
  return false;
}

func intersectLS(L: dynamic, s: dynamic) -> dynamic
{
  return ((cross((L.p2 - L.p1), (s.p1 - L.p1)) * cross((L.p2 - L.p1), (s.p2 - L.p1))) < (-eps));
}

func ccw(p0: dynamic, p1: dynamic, p2: dynamic) -> dynamic
{
  var a: dynamic = (p1 - p0);
  var b: dynamic = (p2 - p0);
  if ((cross(a, b) > eps))
  {
    return 1;
  }
  if ((cross(a, b) < (-eps)))
  {
    return -1;
  }
  if ((dot(a, b) < (-eps)))
  {
    return 2;
  }
  if ((a.norm() < b.norm()))
  {
    return -2;
  }
  return 0;
}

func getCrossPointLL(a: dynamic, b: dynamic) -> dynamic
{
  var A: dynamic = cross((a.p2 - a.p1), (b.p2 - b.p1));
  var B: dynamic = cross((a.p2 - a.p1), (a.p2 - b.p1));
  if (((abs(A) < eps) || (abs(B) < eps)))
  {
    return b.p1;
  }
  return (b.p1 + (((b.p2 - b.p1)) * ((B / A))));
}

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
      if (((abs(cross(a, b)) < eps) && (dot(a, b) < eps)))
      {
        return 1;
      }
      if ((a.y > b.y))
      {
        swap(a, b);
      }
      if ((((a.y < eps) && (eps < b.y)) && (cross(a, b) > eps)))
      {
        x = (!x);
      }
      i += 1;
    }
  }
  if (x)
  {
    return 2;
  }
  return 0;
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var ori: dynamic = cpp_construct(0, 0);

var g: dynamic = cpp_uninitialized();

var buildings: dynamic = cpp_uninitialized();

var vp: dynamic = cpp_uninitialized();

var e: dynamic = cpp_array(MAX);

func init() -> dynamic
{
  buildings.clear();
  vp.clear();
  FOR(i, 0, MAX)[i].clear();
  g.clear();
}

func add_edge(to: dynamic, from_cpp: dynamic, cost: dynamic) -> dynamic
{
  e[to].pb(mp(from_cpp, cost));
  e[from_cpp].pb(mp(to, cost));
}

func check(a: dynamic) -> dynamic
{
  cpp_statement("FOR(i,0,n)");
  if ((contains(buildings[i], a) == 2))
  {
    return false;
  }
  return true;
}

func check(s: dynamic) -> dynamic
{
  cpp_statement("FOR(i,0,n)");
  {
    var p: dynamic = buildings[i];
    m = p.size();
    FOR(j, 0, m);
    {
      var a: dynamic = p[j];
      var b: dynamic = p[(((j + 1)) % m)];
      var c: dynamic = p[((((j - 1) + m)) % m)];
      if (isParallel(Segment(a, b), s))
      {
        continue;
      }
      if (intersect(Segment(a, b), s))
      {
        return false;
      }
    }
  }
  return true;
}

func getdis(a: dynamic, b: dynamic) -> dynamic
{
  var c: dynamic = project(L, a);
  if ((((ccw(s, b, c) == -2) && check((a + (((c - a)) / 2.0)))) && check(Segment(a, c))))
  {
    return abs((c - a));
  }
  c = Point(inf, inf);
  FOR(i, 0, n);
  {
    var p: dynamic = buildings[i];
    m = p.size();
    FOR(j, 0, m);
    {
      if (isParallel(L, seg))
      {
        continue;
      }
      if ((!intersectLS(L, seg)))
      {
        continue;
      }
      var cp: dynamic = getCrossPointLL(L, seg);
      if (((ccw(s, b, cp) == -2) && (abs((s - cp)) < abs((s - c)))))
      {
        c = cp;
      }
    }
  }
  if ((((ccw(s, b, c) == -2) && check((a + (((c - a)) / 2.0)))) && check(Segment(a, c))))
  {
    return abs((c - a));
  }
  return inf;
}

func dijkstra() -> dynamic
{
  var d: dynamic = cpp_array(MAX);
  var pq: dynamic = cpp_uninitialized();
  fill(d, (d + MAX), inf);
  d[0] = 0;
  pq.push(mp(0, 0));
  while (pq.size())
  {
    var u: dynamic = pq.top();
    pq.pop();
    if ((d[u.s] < u.f))
    {
      continue;
    }
    if ((u.s == vp.size()))
    {
      return u.f;
    }
    FOR(i, 0, e[u.s].size());
    {
      var next: dynamic = e[u.s][i].f;
      var cost: dynamic = (e[u.s][i].s + d[u.s]);
      if ((cost < d[next]))
      {
        d[next] = cost;
        pq.push(mp(cost, next));
      }
    }
  }
  return inf;
}

func solve() -> dynamic
{
  if ((!check(Segment(ori, s))))
  {
    return 0;
  }
  vp.pb(ori);
  FOR(i, 0, n);
  {
    var p: dynamic = buildings[i];
    m = p.size();
    FOR(j, 0, m);
    {
      if ((!check(Segment(s, p[j]))))
      {
        continue;
      }
      vp.pb(p[j]);
      if (((ccw(s, p[j], p[((((j - 1) + m)) % m)]) * ccw(s, p[j], p[(((j + 1)) % m)])) == 1))
      {
        g.pb(p[j]);
      }
    }
  }
  FOR(i, 0, vp.size());
  {
    FOR(j, (i + 1), vp.size());
    {
      if ((check((vp[i] + (((vp[j] - vp[i])) / 2.0))) && check(Segment(vp[i], vp[j]))))
      {
        add_edge(i, j, abs((vp[i] - vp[j])));
      }
    }
  }
  FOR(i, 0, vp.size());
  {
    var cost: dynamic = inf;
    FOR(j, 0, g.size());
    {
      if ((vp[i] == g[j]))
      {
        cost = 0;
        break;
      } else
      {
        cost = min(cost, getdis(vp[i], g[j]));
      }
    }
    add_edge(vp.size(), i, cost);
  }
  return dijkstra();
}

func main() -> dynamic
{
  while (((cin >> n) && n))
  {
    init();
    read(s.x, s.y);
    FOR(i, 0, n);
    {
      read(m);
      var p: dynamic = cpp_uninitialized();
      FOR(j, 0, m);
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        read(x, y);
        p.pb(Point(x, y));
      }
      buildings.pb(p);
    }
    pd(solve());
  }
  return 0;
}
