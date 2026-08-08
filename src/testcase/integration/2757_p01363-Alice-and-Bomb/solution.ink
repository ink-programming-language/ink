// Translated from solution.cpp.

var MAX = cpp_expression("#inc");

var inf = cpp_expression("#incl");

var linf = cpp_expression("#inc");

var eps = cpp_expression("#inclu");

var mod = cpp_expression("#include<b");

var pi = cpp_expression("#include");

var phi = cpp_expression("#include<bits/std");

var f = cpp_expression("#incl");

var s = cpp_expression("#inclu");

var mp = cpp_expression("#include<");

var pb = cpp_expression("#include<");

func all(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func pd(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> #defi");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);i++)");
}

func RFOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a)-1;(b)<=i;i--)");
}

func equals(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<bits/stdc+");
}

var dx = [1, 0, -1, 0, 1, 1, -1, -1];

var dy = [0, 1, 0, -1, 1, -1, 1, -1];

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(x: dynamic = 0, y: dynamic = 0)
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
  func operator_multiply(k: dynamic)
  {
      return Point((x * k), (y * k));
    }
  func operator_divide(k: dynamic)
  {
      return Point((x / k), (y / k));
    }
  func operator_less(p: dynamic)
  {
      return if (equals(x, p.x)) ((y - p.y) < (-eps)) else ((x - p.x) < (-eps));
    }
  func operator_equal(p: dynamic)
  {
      return ((fabs((x - p.x)) < eps) && (fabs((y - p.y)) < eps));
    }
  func abs()
  {
      return sqrt(norm());
    }
  func norm()
  {
      return (((x * x) + (y * y)));
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

func norm(a: dynamic)
{
  return (((a.x * a.x) + (a.y * a.y)));
}

func abs(a: dynamic)
{
  return sqrt(norm(a));
}

func dot(a: dynamic, b: dynamic)
{
  return (((a.x * b.x) + (a.y * b.y)));
}

func cross(a: dynamic, b: dynamic)
{
  return (((a.x * b.y) - (a.y * b.x)));
}

func project(s: dynamic, p: dynamic)
{
  var base = ((s.p2 - s.p1));
  var r = ((dot((p - s.p1), base) / base.norm()));
  return ((s.p1 + (base * r)));
}

func isParallel(a: dynamic, b: dynamic)
{
  return equals(cross(a, b), 0.0);
}

func isParallel(s: dynamic, t: dynamic)
{
  return equals(cross((s.p1 - s.p2), (t.p1 - t.p2)), 0.0);
}

func intersect(a: dynamic, b: dynamic)
{
  if ((((cross((a.p2 - a.p1), (b.p1 - a.p1)) * cross((a.p2 - a.p1), (b.p2 - a.p1))) < (-eps)) && ((cross((b.p2 - b.p1), (a.p1 - b.p1)) * cross((b.p2 - b.p1), (a.p2 - b.p1))) < (-eps))))
  {
    return true;
  }
  return false;
}

func intersectLS(L: dynamic, s: dynamic)
{
  return ((cross((L.p2 - L.p1), (s.p1 - L.p1)) * cross((L.p2 - L.p1), (s.p2 - L.p1))) < (-eps));
}

func ccw(p0: dynamic, p1: dynamic, p2: dynamic)
{
  var a = (p1 - p0);
  var b = (p2 - p0);
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

func getCrossPointLL(a: dynamic, b: dynamic)
{
  var A = cross((a.p2 - a.p1), (b.p2 - b.p1));
  var B = cross((a.p2 - a.p1), (a.p2 - b.p1));
  if (((abs(A) < eps) || (abs(B) < eps)))
  {
    return b.p1;
  }
  return (b.p1 + (((b.p2 - b.p1)) * ((B / A))));
}

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

var n: dynamic;

var m: dynamic;

var s: dynamic;

var ori = cpp_construct(0, 0);

var g: dynamic;

var buildings: dynamic;

var vp: dynamic;

var e = cpp_array(MAX);

func init()
{
  buildings.clear();
  vp.clear();
  FOR(i, 0, MAX)[i].clear();
  g.clear();
}

func add_edge(to: dynamic, from_cpp: dynamic, cost: dynamic)
{
  e[to].pb(mp(from_cpp, cost));
  e[from_cpp].pb(mp(to, cost));
}

func check(a: dynamic)
{
  cpp_statement("FOR(i,0,n)");
  if ((contains(buildings[i], a) == 2))
  {
    return false;
  }
  return true;
}

func check(s: dynamic)
{
  cpp_statement("FOR(i,0,n)");
  {
    var p = buildings[i];
    m = p.size();
    FOR(j, 0, m);
    {
      var a = p[j];
      var b = p[(((j + 1)) % m)];
      var c = p[((((j - 1) + m)) % m)];
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

func getdis(a: dynamic, b: dynamic)
{
  var c = project(L, a);
  if ((((ccw(s, b, c) == -2) && check((a + (((c - a)) / 2.0)))) && check(Segment(a, c))))
  {
    return abs((c - a));
  }
  c = Point(inf, inf);
  FOR(i, 0, n);
  {
    var p = buildings[i];
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
      var cp = getCrossPointLL(L, seg);
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

func dijkstra()
{
  var d = cpp_array(MAX);
  var pq: dynamic;
  fill(d, (d + MAX), inf);
  d[0] = 0;
  pq.push(mp(0, 0));
  while (pq.size())
  {
    var u = pq.top();
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
      var next = e[u.s][i].f;
      var cost = (e[u.s][i].s + d[u.s]);
      if ((cost < d[next]))
      {
        d[next] = cost;
        pq.push(mp(cost, next));
      }
    }
  }
  return inf;
}

func solve()
{
  if ((!check(Segment(ori, s))))
  {
    return 0;
  }
  vp.pb(ori);
  FOR(i, 0, n);
  {
    var p = buildings[i];
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
    var cost = inf;
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

func main()
{
  while (((cin >> n) && n))
  {
    init();
    read(s.x, s.y);
    FOR(i, 0, n);
    {
      read(m);
      var p: dynamic;
      FOR(j, 0, m);
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        p.pb(Point(x, y));
      }
      buildings.pb(p);
    }
    pd(solve());
  }
  return 0;
}
