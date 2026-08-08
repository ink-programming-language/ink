// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(n);(i)++)");
}

class FordFulkerson
{
  var graph: dynamic;
  var used: dynamic;
  var INF: dynamic;
  func FordFulkerson(V: dynamic)
  {
      this->graph = cpp_construct(V);
      this->used = cpp_construct(V);
      this->INF = cpp_construct(numeric_limits.max());
    }
  func add_edge(from_cpp: dynamic, to: dynamic, cap: dynamic)
  {
      graph[from_cpp].push_back([to, cap, cpp_cast(graph[to].size()), false]);
      graph[to].push_back([from_cpp, 0, (cpp_cast(graph[from_cpp].size()) - 1), true]);
    }
  func dfs(now: dynamic, t: dynamic, f: dynamic)
  {
      if ((now == t))
      {
        return f;
      }
      used[now] = true;
      {
        var i = 0;
        while ((i < graph[now].size()))
        {
          var e = graph[now][i];
          if (((!used[e.to]) && (e.cap > 0)))
          {
            var d = dfs(e.to, t, min(f, e.cap));
            if ((d > 0))
            {
              e.cap -= d;
              graph[e.to][e.rev].cap += d;
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
          {
            var i = 0;
            while ((i < graph.size()))
            {
              used[i] = 0;
              i += 1;
            }
          }
          var f = dfs(s, t, INF);
          if ((f > 0))
          {
            flow += f;
          } else
          {
            return flow;
          }
        }
      }
    }
}

var N = 2000;

var INF = 1e9;

func main()
{
  var R: dynamic;
  var C: dynamic;
  read(R, C);
  rep(i, R);
  read(S[i]);
  var s = (R * C);
  var t = (s + 1);
  var cnt = (t + 1);
  {
    var i = 0;
    while ((i < R))
    {
      {
        var j = 0;
        while ((j < C))
        {
          if ((S[i][j] == cpp_char(".")))
          {
            j += 1;
            continue;
          }
          graph.add_edge(s, ((i * C) + j), 1);
          graph.add_edge(((i * C) + j), t, 1);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < R))
    {
      {
        var j = 0;
        while ((j < C))
        {
          if ((S[i][j] == cpp_char(".")))
          {
            j += 1;
            continue;
          }
          if ((((i + 1) < R) && (S[(i + 1)][j] == cpp_char("#"))))
          {
            ans -= 1;
            graph.add_edge(cnt, t, 1);
            graph.add_edge(s, cnt, 0);
            graph.add_edge(((i * C) + j), cnt, INF);
            graph.add_edge(((((i + 1)) * C) + j), cnt, INF);
            cnt += 1;
          }
          if ((((j + 1) < C) && (S[i][(j + 1)] == cpp_char("#"))))
          {
            ans -= 1;
            graph.add_edge(cnt, t, 0);
            graph.add_edge(s, cnt, 1);
            graph.add_edge(cnt, ((i * C) + j), INF);
            graph.add_edge(cnt, (((i * C) + j) + 1), INF);
            cnt += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  ans += graph.max_flow(s, t);
  write(ans, "\n");
  return 0;
}
