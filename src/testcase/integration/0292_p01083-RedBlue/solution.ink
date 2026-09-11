// Translated from solution.cpp.

var EPS: dynamic = 1e-5;

var INF: dynamic = 1e12;

var PI: dynamic = acos(-1);

func EQ(n: dynamic, m: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream>");
}

var X: dynamic = cpp_expression("#inclu");

var Y: dynamic = cpp_expression("#inclu");

class L
{
  func L(a: dynamic, b: dynamic) -> dynamic
  {
      at(0) = a;
      at(1) = b;
    }
  func L() -> dynamic
  {
    }
}

class C
{
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func C(p: dynamic, r: dynamic) -> dynamic
  {
      self->p = cpp_construct(p);
      self->r = cpp_construct(r);
    }
  func C() -> dynamic
  {
    }
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((!EQ(a.X, b.X))) ? (a.X < b.X) : ((a.Y + EPS) < b.Y);
}

func operator_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) < EPS);
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).X;
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).Y;
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  if ((cross(b, c) > EPS))
  {
    return +1;
  }
  if ((cross(b, c) < (-EPS)))
  {
    return -1;
  }
  if ((dot(b, c) < (-EPS)))
  {
    return +2;
  }
  if (((abs(c) - abs(b)) > EPS))
  {
    return -2;
  }
  return 0;
}

func unit(p: dynamic) -> dynamic
{
  return (p / abs(p));
}

func rotate(p: dynamic, rad: dynamic) -> dynamic
{
  return (p * P(cos(rad), sin(rad)));
}

func intersectSS(a: dynamic, b: dynamic) -> dynamic
{
  return ((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) <= 0)) && (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) <= 0)));
}

func projection(l: dynamic, p: dynamic) -> dynamic
{
  var t: dynamic = (dot((p - l[0]), (l[0] - l[1])) / norm((l[0] - l[1])));
  return (l[0] + (t * ((l[0] - l[1]))));
}

func distanceLP(l: dynamic, p: dynamic) -> dynamic
{
  return (abs(cross((l[1] - l[0]), (p - l[0]))) / abs((l[1] - l[0])));
}

func distanceSP(s: dynamic, p: dynamic) -> dynamic
{
  if ((dot((s[1] - s[0]), (p - s[0])) < EPS))
  {
    return abs((p - s[0]));
  }
  if ((dot((s[0] - s[1]), (p - s[1])) < EPS))
  {
    return abs((p - s[1]));
  }
  return distanceLP(s, p);
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return (abs(cross(a, b)) < EPS);
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return isParallel((a[1] - a[0]), (b[1] - b[0]));
}

func crosspointLL(l: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = cross((l[1] - l[0]), (m[1] - m[0]));
  var B: dynamic = cross((l[1] - l[0]), (l[1] - m[0]));
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

func crosspointCL(c: dynamic, l: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var mid: dynamic = projection(l, c.p);
  var d: dynamic = distanceLP(l, c.p);
  if (EQ(d, c.r))
  {
    ret.push_back(mid);
  } else if ((d < c.r))
  {
    var len: dynamic = sqrt(((c.r * c.r) - (d * d)));
    ret.push_back((mid + (len * unit((l[1] - l[0])))));
    ret.push_back((mid - (len * unit((l[1] - l[0])))));
  }
  return ret;
}

func getTangentLine(c: dynamic, p: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var dir: dynamic = (p - c.p);
  if (((c.r + EPS) < abs(dir)))
  {
    var a: dynamic = abs(dir);
    var b: dynamic = sqrt(((a * a) - (c.r * c.r)));
    var psi: dynamic = arg((p - c.p));
    var phi: dynamic = (PI - acos((b / a)));
    ret.emplace_back(p, (p + (b * P(cos((psi + phi)), sin((psi + phi))))));
    ret.emplace_back(p, (p + (b * P(cos((psi - phi)), sin((psi - phi))))));
  } else if ((abs((c.r - abs(dir))) < EPS))
  {
    ret.push_back(L(p, (p + (dir * P(0, 1)))));
  }
  return ret;
}

class edge
{
  var to: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
  var cap: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  func edge(to: dynamic, rev: dynamic, cap: dynamic, cost: dynamic) -> dynamic
  {
      self->to = cpp_construct(to);
      self->rev = cpp_construct(rev);
      self->cap = cpp_construct(cap);
      self->cost = cpp_construct(cost);
    }
  func edge() -> dynamic
  {
    }
}

func min_cost_flow(s: dynamic, g: dynamic, f: dynamic, adj: dynamic) -> dynamic
{
  var n: dynamic = adj.size();
  var res: dynamic = 0;
  while ((f > 0))
  {
    mincost[s] = 0;
    while (1)
    {
      var update: dynamic = false;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((mincost[i] == INF))
          {
            i += 1;
            continue;
          }
          {
            var j: dynamic = 0;
            while ((j < cpp_cast(adj[i].size())))
            {
              var e: dynamic = adj[i][j];
              if (((e.cap > 0) && (((mincost[i] + e.cost) + EPS) < mincost[e.to])))
              {
                mincost[e.to] = (mincost[i] + e.cost);
                prevv[e.to] = i;
                preve[e.to] = j;
                update = true;
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      if ((!update))
      {
        break;
      }
    }
    if ((mincost[g] == INF))
    {
      return -1;
    }
    var d: dynamic = f;
    {
      var v: dynamic = g;
      while ((v != s))
      {
        d = min(d, adj[prevv[v]][preve[v]].cap);
        v = prevv[v];
      }
    }
    f -= d;
    res += (d * mincost[g]);
    {
      var v: dynamic = g;
      while ((v != s))
      {
        var e: dynamic = adj[prevv[v]][preve[v]];
        e.cap -= d;
        adj[v][e.rev].cap += d;
        v = prevv[v];
      }
    }
  }
  return res;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var c: dynamic = cpp_construct(2);
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      read(x, y, r);
      c[i] = C(P(x, y), r);
      i += 1;
    }
  }
  var pos: dynamic = cpp_construct(2, VP(n));
  var tangent: dynamic = cpp_construct(2, vector(n));
  {
    var d: dynamic = 0;
    while ((d < 2))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var x: dynamic = cpp_uninitialized();
          var y: dynamic = cpp_uninitialized();
          read(x, y);
          pos[d][i] = P(x, y);
          var ret: dynamic = cpp_uninitialized();
          {
            var dd: dynamic = 0;
            while ((dd < 2))
            {
              ret = getTangentLine(c[dd], pos[d][i]);
              tangent[d][i].insert(tangent[d][i].end(), ret.begin(), ret.end());
              dd += 1;
            }
          }
          i += 1;
        }
      }
      d += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          var cand: dynamic = cpp_uninitialized();
          cand.push_back((((pos[0][i] + pos[1][j])) / 2.0));
          for (var t1: dynamic in tangent[0][i])
          {
            for (var t2: dynamic in tangent[1][j])
            {
              if ((!isParallel(t1, t2)))
              {
                cand.push_back(crosspointLL(t1, t2));
              }
            }
          }
          for (var cp: dynamic in cand)
          {
            if ((((((distanceSP(L(pos[0][i], cp), c[0].p) + EPS) > c[0].r) && ((distanceSP(L(pos[0][i], cp), c[1].p) + EPS) > c[1].r)) && ((distanceSP(L(pos[1][j], cp), c[0].p) + EPS) > c[0].r)) && ((distanceSP(L(pos[1][j], cp), c[1].p) + EPS) > c[1].r)))
            {
              dist[i][j] = min(dist[i][j], (abs((pos[0][i] - cp)) + abs((pos[1][j] - cp))));
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var adj: dynamic = cpp_construct(((2 * n) + 2));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((dist[i][j] == INF))
          {
            j += 1;
            continue;
          }
          adj[(i + 2)].emplace_back(((j + 2) + n), adj[((j + 2) + n)].size(), 1, dist[i][j]);
          adj[((j + 2) + n)].emplace_back((i + 2), (adj[(i + 2)].size() - 1), 0, (-dist[i][j]));
          j += 1;
        }
      }
      adj[0].emplace_back((i + 2), adj[(i + 2)].size(), 1, 0);
      adj[(i + 2)].emplace_back(0, (adj[0].size() - 1), 0, 0);
      adj[((i + 2) + n)].emplace_back(1, adj[1].size(), 1, 0);
      adj[1].emplace_back(((i + 2) + n), (adj[((i + 2) + n)].size() - 1), 0, 0);
      i += 1;
    }
  }
  var ans: dynamic = min_cost_flow(0, 1, n, adj);
  if ((ans == -1))
  {
    write("Impossible", "\n");
  } else
  {
    write(fixed, setprecision(10));
    write(ans, "\n");
  }
  return 0;
}
