// Translated from solution.cpp.

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i = a; i < (b); ++i)");
}

func trav(a: dynamic, x: dynamic)
{
  return cpp_expression("#include <bits/s");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/s");
}

func sz(x: dynamic)
{
  return cpp_expression("#include <bits/");
}

var N = 2050;

var INF = cpp_cast(1e18);

class edge
{
  var to: dynamic;
  var cap: dynamic;
  var rev: dynamic;
  func edge(to: dynamic, cap: dynamic, rev: dynamic)
  {
      to = to;
      cap = cap;
      rev = rev;
    }
}

class Dinic
{
  var G: dynamic = cpp_array(N);
  var level: dynamic = cpp_array(N);
  var iter: dynamic = cpp_array(N);
  func add_edge(from_cpp: dynamic, to: dynamic, cap: dynamic)
  {
      G[from_cpp].push_back(edge(to, cap, G[to].size()));
      G[to].push_back(edge(from_cpp, 0, (G[from_cpp].size() - 1)));
    }
  func bfs(s: dynamic)
  {
      memset(level, -1, cpp_sizeof((level)));
      var que: dynamic;
      level[s] = 0;
      que.push(s);
      while ((!que.empty()))
      {
        var v = que.front();
        que.pop();
        {
          var i = 0;
          while ((i < G[v].size()))
          {
            var e = G[v][i];
            if (((e.cap > 0) && (level[e.to] < 0)))
            {
              level[e.to] = (level[v] + 1);
              que.push(e.to);
            }
            i += 1;
          }
        }
      }
    }
  func dfs(v: dynamic, t: dynamic, f: dynamic)
  {
      if ((v == t))
      {
        return f;
      }
      {
        var i = iter[v];
        while ((i < G[v].size()))
        {
          var e = G[v][i];
          if (((e.cap > 0) && (level[v] < level[e.to])))
          {
            var d = dfs(e.to, t, min(e.cap, f));
            if ((d > 0))
            {
              e.cap -= d;
              G[e.to][e.rev].cap += d;
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
      {
        while (true)
        {
          bfs(s);
          if ((level[t] < 0))
          {
            return flow;
          }
          memset(iter, 0, cpp_sizeof((iter)));
          var f: dynamic;
          while (((cpp_assign(f, "=", dfs(s, t, INF))) > 0))
          {
            flow += f;
          }
        }
      }
    }
}

var dinic: dynamic;

var n: dynamic;

var p = cpp_array(N);

var mp: dynamic;

var tp = [[1, 0], [2, 3]];

func gettype(i: dynamic)
{
  return tp[((((p[i].first % 2) + 2)) % 2)][((((p[i].second % 2) + 2)) % 2)];
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  read(n);
  var sum = 0;
  var ds = ((2 * n) + 1);
  var dt = ((2 * n) + 2);
  rep(i, 1, (n + 1));
  {
    var w: dynamic;
    read(p[i].first, p[i].second, w);
    swap(p[i].first, p[i].second);
    sum += w;
    dinic.add_edge(i, (i + n), w);
    mp[p[i]] = i;
  }
  rep(i, 1, (n + 1));
  {
    if ((((p[i].first % 2) == 0) && ((p[i].second % 2) == 0)))
    {
      {
        var x = 0;
        while ((x <= 1))
        {
          {
            var y0 = 0;
            while ((y0 <= 1))
            {
              rep(y1, 0, 2);
              {
                var idx1 = mp[[(p[i].first + ((x - 1))), (p[i].second + ((y0 - 1)))]];
                var idx2 = mp[[(p[i].first + ((x - 1))), (p[i].second + y0)]];
                var idx3 = mp[[(p[i].first + x), (p[i].second + ((y1 - 1)))]];
                var idx4 = mp[[(p[i].first + x), (p[i].second + y1)]];
                if ((((idx1 && idx2) && idx3) && idx4))
                {
                  var vs = [idx1, idx2, idx3, idx4];
                  sort(all(vs), __cpp_lambda_1);
                  var la = ds;
                  for (var v in vs)
                  {
                    dinic.add_edge(la, v, INF);
                    la = (v + n);
                  }
                  dinic.add_edge(la, dt, INF);
                }
              }
              y0 += 1;
            }
          }
          x += 1;
        }
      }
    }
  }
  write((sum - dinic.max_flow(ds, dt)), cpp_char("\n"));
}

func __cpp_lambda_1(i: dynamic, j: dynamic)
{
  return (gettype(i) < gettype(j));
}
