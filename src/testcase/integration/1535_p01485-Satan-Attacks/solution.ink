// Translated from solution.cpp.

var inf = 1e9;

var EPS = 1e-6;

var INF = 1e12;

var PI = acos(-1);

func EQ(n: dynamic, m: dynamic)
{
  return cpp_expression("#include <iostream>");
}

var X = cpp_expression("#inclu");

var Y = cpp_expression("#inclu");

class L
{
  func L(a: dynamic, b: dynamic)
  {
      at(0) = a;
      at(1) = b;
    }
  func L()
  {
    }
}

func operator_less(a: dynamic, b: dynamic)
{
  return if ((!EQ(a.X, b.X))) (a.X < b.X) else ((a.Y + EPS) < b.Y);
}

func operator_equal(a: dynamic, b: dynamic)
{
  return (abs((a - b)) < EPS);
}

func dot(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).X;
}

func cross(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).Y;
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
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

func intersectSS(a: dynamic, b: dynamic)
{
  return ((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) <= 0)) && (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) <= 0)));
}

func intersectSP(s: dynamic, p: dynamic)
{
  return ((abs(cross((s[0] - p), (s[1] - p))) < EPS) && (dot((s[0] - p), (s[1] - p)) < EPS));
}

func isParallel(a: dynamic, b: dynamic)
{
  return (abs(cross(a, b)) < EPS);
}

func isParallel(a: dynamic, b: dynamic)
{
  return isParallel((a[1] - a[0]), (b[1] - b[0]));
}

func crosspointLL(l: dynamic, m: dynamic)
{
  var A = cross((l[1] - l[0]), (m[1] - m[0]));
  var B = cross((l[1] - l[0]), (l[1] - m[0]));
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

func arrangementEX(l: dynamic, w: dynamic, source: dynamic)
{
  var cp = cpp_construct(l.size());
  var plist: dynamic;
  {
    var i = 0;
    while ((i < cpp_cast(l.size())))
    {
      {
        var j = (i + 1);
        while ((j < cpp_cast(l.size())))
        {
          if (((!isParallel(l[i], l[j])) && intersectSS(l[i], l[j])))
          {
            var cpij = crosspointLL(l[i], l[j]);
            cp[i].push_back(cpij);
            cp[j].push_back(cpij);
            plist.push_back(cpij);
          }
          {
            var d = 0;
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
  var n = plist.size();
  var conv: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      conv[plist[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast(cp.size())))
    {
      var len = abs((l[i][1] - l[i][0]));
      {
        var j = 0;
        while ((j < (cpp_cast(cp[i].size()) - 1)))
        {
          var jidx = conv[cp[i][j]];
          var jp1idx = conv[cp[i][(j + 1)]];
          var cap = ((w[i] * abs((cp[i][j] - cp[i][(j + 1)]))) / len);
          adj[jidx][jp1idx] += cap;
          adj[jp1idx][jidx] += cap;
          j += 1;
        }
      }
      i += 1;
    }
  }
  for (var p in source)
  {
    var idx = conv[p];
    ret[0].emplace_back(idx, inf);
    ret[idx].emplace_back(0, inf);
  }
  {
    var i = 0;
    while ((i < n))
    {
      for (var next in adj[i])
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
  var to: dynamic;
  var cap: dynamic;
  var rev: dynamic;
  func edge(t: dynamic, c: dynamic, r: dynamic)
  {
      this->to = cpp_construct(t);
      this->cap = cpp_construct(c);
      this->rev = cpp_construct(r);
    }
  func edge()
  {
    }
}

func addedge(from_cpp: dynamic, to: dynamic, cap: dynamic, graph: dynamic)
{
  graph[from_cpp].emplace_back(to, cap, graph[to].size());
  graph[to].emplace_back(from_cpp, 0, (graph[from_cpp].size() - 1));
}

func makeflowgraph(adj: dynamic)
{
  var n = adj.size();
  {
    var i = 0;
    while ((i < n))
    {
      for (var next in adj[i])
      {
        addedge(i, next.first, next.second, ret);
      }
      i += 1;
    }
  }
  return ret;
}

func dfs(v: dynamic, g: dynamic, flow: dynamic, adj: dynamic, used: dynamic)
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
    var i = 0;
    while ((i < cpp_cast(adj[v].size())))
    {
      var next = adj[v][i];
      if ((next.cap > 0))
      {
        var ret = dfs(next.to, g, min(flow, next.cap), adj, used);
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

func maxflow(s: dynamic, g: dynamic, graph: dynamic)
{
  var res = 0;
  while (1)
  {
    var used = cpp_construct((((graph.size() + 1)) * 2), false);
    var ret = dfs(s, g, inf, graph, used);
    if ((ret == -1))
    {
      break;
    }
    res += ret;
  }
  return res;
}

func maxflow_adj(s: dynamic, g: dynamic, adj: dynamic)
{
  var graph = makeflowgraph(adj);
  return maxflow(s, g, graph);
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      var xs: dynamic;
      var ys: dynamic;
      var xt: dynamic;
      var yt: dynamic;
      read(xs, ys, xt, yt, w[i]);
      l[i] = L(P(xs, ys), P(xt, yt));
      i += 1;
    }
  }
  var m: dynamic;
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      source[i] = P(x, y);
      i += 1;
    }
  }
  var x: dynamic;
  var y: dynamic;
  read(x, y);
  var ret = arrangementEX(l, w, source);
  var adj = ret.first;
  var plist = ret.second;
  var sidx = 0;
  var gidx = (lower_bound(plist.begin(), plist.end(), sink) - plist.begin());
  write(maxflow_adj(sidx, gidx, adj), "\n");
  return 0;
}
