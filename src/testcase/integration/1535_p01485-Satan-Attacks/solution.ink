// Translated from solution.cpp.

var inf: dynamic = 1e9;

var EPS: dynamic = 1e-6;

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

func intersectSS(a: dynamic, b: dynamic) -> dynamic
{
  return ((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) <= 0)) && (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) <= 0)));
}

func intersectSP(s: dynamic, p: dynamic) -> dynamic
{
  return ((abs(cross((s[0] - p), (s[1] - p))) < EPS) && (dot((s[0] - p), (s[1] - p)) < EPS));
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

func arrangementEX(l: dynamic, w: dynamic, source: dynamic) -> dynamic
{
  var cp: dynamic = cpp_construct(l.size());
  var plist: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(l.size())))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < cpp_cast(l.size())))
        {
          if (((!isParallel(l[i], l[j])) && intersectSS(l[i], l[j])))
          {
            var cpij: dynamic = crosspointLL(l[i], l[j]);
            cp[i].push_back(cpij);
            cp[j].push_back(cpij);
            plist.push_back(cpij);
          }
          {
            var d: dynamic = 0;
            while ((d < 2))
            {
              if (intersectSP(l[i], l[j][d]))
              {
                cp[i].push_back(l[j][d]);
              }
              if (intersectSP(l[j], l[i][d]))
              {
                cp[j].push_back(l[i][d]);
              }
              d += 1;
            }
          }
          j += 1;
        }
      }
      cp[i].push_back(l[i][0]);
      cp[i].push_back(l[i][1]);
      plist.push_back(l[i][0]);
      plist.push_back(l[i][1]);
      sort(cp[i].begin(), cp[i].end());
      cp[i].erase(unique(cp[i].begin(), cp[i].end()), cp[i].end());
      i += 1;
    }
  }
  plist.emplace_back((-INF), (-INF));
  sort(plist.begin(), plist.end());
  plist.erase(unique(plist.begin(), plist.end()), plist.end());
  var n: dynamic = plist.size();
  var conv: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      conv[plist[i]] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(cp.size())))
    {
      var len: dynamic = abs((l[i][1] - l[i][0]));
      {
        var j: dynamic = 0;
        while ((j < (cpp_cast(cp[i].size()) - 1)))
        {
          var jidx: dynamic = conv[cp[i][j]];
          var jp1idx: dynamic = conv[cp[i][(j + 1)]];
          var cap: dynamic = ((w[i] * abs((cp[i][j] - cp[i][(j + 1)]))) / len);
          adj[jidx][jp1idx] += cap;
          adj[jp1idx][jidx] += cap;
          j += 1;
        }
      }
      i += 1;
    }
  }
  for (var p: dynamic in source)
  {
    var idx: dynamic = conv[p];
    ret[0].emplace_back(idx, inf);
    ret[idx].emplace_back(0, inf);
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      for (var next: dynamic in adj[i])
      {
        ret[i].emplace_back(next.first, (next.second + EPS));
      }
      i += 1;
    }
  }
  return make_pair(ret, plist);
}

class edge
{
  var to: dynamic = cpp_uninitialized();
  var cap: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
  func edge(t: dynamic, c: dynamic, r: dynamic) -> dynamic
  {
      self->to = cpp_construct(t);
      self->cap = cpp_construct(c);
      self->rev = cpp_construct(r);
    }
  func edge() -> dynamic
  {
    }
}

func addedge(from_cpp: dynamic, to: dynamic, cap: dynamic, graph: dynamic) -> dynamic
{
  graph[from_cpp].emplace_back(to, cap, graph[to].size());
  graph[to].emplace_back(from_cpp, 0, (graph[from_cpp].size() - 1));
}

func makeflowgraph(adj: dynamic) -> dynamic
{
  var n: dynamic = adj.size();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      for (var next: dynamic in adj[i])
      {
        addedge(i, next.first, next.second, ret);
      }
      i += 1;
    }
  }
  return ret;
}

func dfs(v: dynamic, g: dynamic, flow: dynamic, adj: dynamic, used: dynamic) -> dynamic
{
  if (used[v])
  {
    return -1;
  }
  used[v] = true;
  if ((v == g))
  {
    return flow;
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(adj[v].size())))
    {
      var next: dynamic = adj[v][i];
      if ((next.cap > 0))
      {
        var ret: dynamic = dfs(next.to, g, min(flow, next.cap), adj, used);
        if ((ret > 0))
        {
          next.cap -= ret;
          adj[next.to][next.rev].cap += ret;
          return ret;
        }
      }
      i += 1;
    }
  }
  return -1;
}

func maxflow(s: dynamic, g: dynamic, graph: dynamic) -> dynamic
{
  var res: dynamic = 0;
  while (1)
  {
    var used: dynamic = cpp_construct((((graph.size() + 1)) * 2), false);
    var ret: dynamic = dfs(s, g, inf, graph, used);
    if ((ret == -1))
    {
      break;
    }
    res += ret;
  }
  return res;
}

func maxflow_adj(s: dynamic, g: dynamic, adj: dynamic) -> dynamic
{
  var graph: dynamic = makeflowgraph(adj);
  return maxflow(s, g, graph);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var xs: dynamic = cpp_uninitialized();
      var ys: dynamic = cpp_uninitialized();
      var xt: dynamic = cpp_uninitialized();
      var yt: dynamic = cpp_uninitialized();
      read(xs, ys, xt, yt, w[i]);
      l[i] = L(P(xs, ys), P(xt, yt));
      i += 1;
    }
  }
  var m: dynamic = cpp_uninitialized();
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      source[i] = P(x, y);
      i += 1;
    }
  }
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(x, y);
  var ret: dynamic = arrangementEX(l, w, source);
  var adj: dynamic = ret.first;
  var plist: dynamic = ret.second;
  var sidx: dynamic = 0;
  var gidx: dynamic = (lower_bound(plist.begin(), plist.end(), sink) - plist.begin());
  write(maxflow_adj(sidx, gidx, adj), "\n");
  return 0;
}
