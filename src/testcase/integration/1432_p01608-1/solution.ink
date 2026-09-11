// Translated from solution.cpp.

func REP(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(x);i++)");
}

func FOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();i++)");
}

func RREP(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=((int)(x)-1);i>=0;i--)");
}

func RFOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).rbegin())i=(c).rbegin();i!=(c).rend();i++)");
}

func ALL(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #include <cmath>");
}

func RALL(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #include <cmath> #");
}

func SZ(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #incl");
}

func mp(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdi");
}

func UNIQUE(v: dynamic) -> dynamic
{
  cpp_macro("v.erase( unique(v.begin(), v.end()), v.end() );");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  (os << "[");
  (os << "]");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  (os << "{");
  (os << "}");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  return (((((os << "(") << t.first) << ",") << t.second) << ")");
}

var EPS: dynamic = 1e-8;

var MOD: dynamic = 1000000007;

var INF: dynamic = 999999999;

class Edge
{
  var src: dynamic = cpp_uninitialized();
  var dst: dynamic = cpp_uninitialized();
  var weight: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  func Edge(f: dynamic, t: dynamic, c: dynamic, rev: dynamic = 0, ff: dynamic = 0) -> dynamic
  {
      self->src = cpp_construct(f);
      self->dst = cpp_construct(t);
      self->weight = cpp_construct(c);
      self->rev = cpp_construct(rev);
      self->f = cpp_construct(ff);
    }
}

func add_edge(G: dynamic, s: dynamic, t: dynamic, cap: dynamic, f: dynamic = 0) -> dynamic
{
  G[s].push_back(Edge(s, t, cap, G[t].size(), f));
  G[t].push_back(Edge(t, s, cap, (G[s].size() - 1), f));
}

func bfs(G: dynamic, level: dynamic, s: dynamic) -> dynamic
{
  level[s] = 0;
  var que: dynamic = cpp_uninitialized();
  que.push(s);
  while ((!que.empty()))
  {
    var v: dynamic = que.front();
    que.pop();
    REP(i, G[v].size());
    {
      var e: dynamic = G[v][i];
      if (((e.weight > 0) && (level[e.dst] < 0)))
      {
        level[e.dst] = (level[v] + 1);
        que.push(e.dst);
      }
    }
  }
}

func dfs(G: dynamic, level: dynamic, iter: dynamic, v: dynamic, t: dynamic, flow: dynamic) -> dynamic
{
  if ((v == t))
  {
    return flow;
  }
  {
    var i: dynamic = iter[v];
    while ((i < cpp_cast(G[v].size())))
    {
      var e: dynamic = G[v][i];
      if (((e.weight > 0) && (level[v] < level[e.dst])))
      {
        var d: dynamic = dfs(G, level, iter, e.dst, t, min(flow, e.weight));
        if ((d > 0))
        {
          e.weight -= d;
          G[e.dst][e.rev].weight += d;
          return d;
        }
      }
      i += 1;
    }
  }
  return 0;
}

func max_flow(G: dynamic, s: dynamic, t: dynamic) -> dynamic
{
  var flow: dynamic = 0;
  while (true)
  {
    var level: dynamic = cpp_construct(G.size(), -1);
    var iter: dynamic = cpp_construct(G.size(), 0);
    bfs(G, level, s);
    if ((level[t] < 0))
    {
      break;
    }
    var f: dynamic = 0;
    while (((cpp_assign(f, "=", dfs(G, level, iter, s, t, INF))) > 0))
    {
      flow += f;
    }
  }
  return flow;
}

var n: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n, w);
  REP(i, n);
  read(a[i]);
  REP(i, n);
  read(b[i]);
  var sum: dynamic = (accumulate(ALL(a), 0) + accumulate(ALL(b), 0));
  var res: dynamic = cpp_construct(-1, "");
  REP(k, 2);
  {
    var g: dynamic = cpp_construct(((n + (2 * w)) + 3));
    var s: dynamic = ((n + (2 * w)) + 1);
    var t: dynamic = ((n + (2 * w)) + 2);
    REP(i, (w + 1));
    {
      if ((i & 1))
      {
        add_edge(g, s, i, INF);
      } else
      {
        add_edge(g, i, t, INF);
      }
      if (((((((w + n) + i)) & 1)) ^ k))
      {
        add_edge(g, s, ((w + n) + i), INF);
      } else
      {
        add_edge(g, ((w + n) + i), t, INF);
      }
    }
    var score: dynamic = (sum - max_flow(g, s, t));
    var ans: dynamic = cpp_construct(n, cpp_char("1"));
    var q: dynamic = cpp_uninitialized();
    var att: dynamic = cpp_construct(g.size(), 0);
    att[s] = 1;
    q.push(s);
    while ((!q.empty()))
    {
      var u: dynamic = q.front();
      q.pop();
      FOR(e, g[u]);
      if ((e->weight != 0))
      {
        if ((!att[e->dst]))
        {
          att[e->dst] = 1;
          q.push(e->dst);
        }
      }
    }
    REP(i, (n + 1))[i] = (cpp_char("0") + ((att[(i + w)] == att[((i + w) + 1)])));
    res = max(res, make_pair(score, ans));
  }
  write(res.second, "\n");
  return 0;
}

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      add_edge(g, (w + i), ((w + i) + 1), a[i], 1);
      add_edge(g, i, (((2 * w) + i) + 1), b[i]);
    }
