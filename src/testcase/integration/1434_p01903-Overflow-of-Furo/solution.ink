// Translated from solution.cpp.

var INF = 1001001001001001;

var inf = 1000000007;

var MOD = 1000000007;

var PI = 3.1415926535897932;

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func ALL(a: dynamic)
{
  return cpp_expression("#include <iostrea");
}

func RALL(a: dynamic)
{
  return cpp_expression("#include <iostream>");
}

class Dinic
{
  var inf: dynamic;
  var g: dynamic;
  var min_cost: dynamic;
  var iter: dynamic;
  func Dinic(V: dynamic)
  {
      this->inf = cpp_construct(numeric_limits.max());
      this->g = cpp_construct(V);
    }
  func add_edge(from_cpp: dynamic, to: dynamic, cap: dynamic)
  {
      g[from_cpp].emplace_back([to, cap, cpp_cast(g[to].size()), false]);
      g[to].emplace_back([from_cpp, 0, (cpp_cast(g[from_cpp].size()) - 1), true]);
    }
  func bfs(s: dynamic, t: dynamic)
  {
      min_cost.assign(g.size(), -1);
      var que: dynamic;
      min_cost[s] = 0;
      que.push(s);
      while (((!que.empty()) && (min_cost[t] == -1)))
      {
        var p = que.front();
        que.pop();
        for (var e in g[p])
        {
          if (((e.cap > 0) && (min_cost[e.to] == -1)))
          {
            min_cost[e.to] = (min_cost[p] + 1);
            que.push(e.to);
          }
        }
      }
      return (min_cost[t] != -1);
    }
  func dfs(idx: dynamic, t: dynamic, flow: dynamic)
  {
      if ((idx == t))
      {
        return flow;
      }
      {
        var i = iter[idx];
        while ((i < g[idx].size()))
        {
          var e = g[idx][i];
          if (((e.cap > 0) && (min_cost[idx] < min_cost[e.to])))
          {
            var d = dfs(e.to, t, min(flow, e.cap));
            if ((d > 0))
            {
              e.cap -= d;
              g[e.to][e.rev].cap += d;
              return d;
            }
          }
          i += 1;
        }
      }
      return 0;
    }
  func max_flow(s: dynamic, t: dynamic)
  {
      var flow = 0;
      while (bfs(s, t))
      {
        iter.assign(g.size(), 0);
        var f = 0;
        while (((cpp_assign(f, "=", dfs(s, t, inf))) > 0))
        {
          flow += f;
        }
      }
      return flow;
    }
  func output()
  {
      {
        var i = 0;
        while ((i < g.size()))
        {
          for (var e in g[i])
          {
            if (e.isrev)
            {
              continue;
            }
            var rev_e = g[e.to][e.rev];
            write(i, "->", e.to, " (flow: ", rev_e.cap, "/", (e.cap + rev_e.cap), ")", "\n");
          }
          i += 1;
        }
      }
    }
}

func main()
{
  var k: dynamic;
  var n: dynamic;
  var m: dynamic;
  read(k, n, m);
  var source = ((n + k) + 1);
  var sink = 0;
  var dc = cpp_construct(((n + k) + 2));
  var overfuro = false;
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      read(a, b, c);
      if ((a > b))
      {
        swap(a, b);
      }
      if (cpp_binary(cpp_binary((a == sink), "and", (1 <= b)), "and", (b <= k)))
      {
        overfuro = true;
      }
      dc.add_edge(a, b, c);
      dc.add_edge(b, a, c);
      i += 1;
    }
  }
  if (overfuro)
  {
    write("overfuro", "\n");
    return 0;
  }
  {
    var i = 1;
    while ((i <= k))
    {
      dc.add_edge(source, i, INF);
      i += 1;
    }
  }
  var ans = dc.max_flow(source, sink);
  var add = 0;
  {
    var i = 0;
    while ((i < dc.g.size()))
    {
      for (var e in dc.g[i])
      {
        if (e.isrev)
        {
          continue;
        }
        var rev_e = dc.g[e.to][e.rev];
        if ((rev_e.cap == (e.cap + rev_e.cap)))
        {
          var extra = dc;
          extra.add_edge(i, e.to, INF);
          var tmp = extra.max_flow(source, sink);
          add = max(add, tmp);
        }
      }
      i += 1;
    }
  }
  write((ans + add), "\n");
  return 0;
}
