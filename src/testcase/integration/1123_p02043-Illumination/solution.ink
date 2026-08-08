// Translated from solution.cpp.

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

class Dinic
{
  var INF: dynamic;
  var n: dynamic;
  var G: dynamic;
  var M: dynamic;
  var level: dynamic;
  var iter: dynamic;
  func Dinic()
  {
    }
  func Dinic(sz: dynamic)
  {
      this->n = cpp_construct(sz);
      this->G = cpp_construct(n);
      this->M = cpp_construct(n);
      this->level = cpp_construct(n);
      this->iter = cpp_construct(n);
    }
  func add_edge(from_cpp: dynamic, to: dynamic, cap: dynamic)
  {
      M[from_cpp][to] = G[from_cpp].size();
      M[to][from_cpp] = G[to].size();
      G[from_cpp].push_back(edge(to, cap, G[to].size()));
      G[to].push_back(edge(from_cpp, 0, (G[from_cpp].size() - 1)));
    }
  func bfs(s: dynamic)
  {
      fill(level.begin(), level.end(), -1);
      var que: dynamic;
      level[s] = 0;
      que.push(s);
      while ((!que.empty()))
      {
        var v = que.front();
        que.pop();
        {
          var i = 0;
          while ((i < cpp_cast(G[v].size())))
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
        while ((i < cpp_cast(G[v].size())))
        {
          var e = G[v][i];
          if (((e.cap > 0) && (level[v] < level[e.to])))
          {
            var d = dfs(e.to, t, min(f, e.cap));
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
  func flow(s: dynamic, t: dynamic, lim: dynamic)
  {
      var fl = 0;
      {
        while (true)
        {
          bfs(s);
          if (((level[t] < 0) || (lim == 0)))
          {
            return fl;
          }
          fill(iter.begin(), iter.end(), 0);
          var f: dynamic;
          while (((cpp_assign(f, "=", dfs(s, t, lim))) > 0))
          {
            fl += f;
            lim -= f;
          }
        }
      }
    }
  func flow(s: dynamic, t: dynamic)
  {
      return flow(s, t, INF);
    }
  func back_edge(s: dynamic, t: dynamic, from_cpp: dynamic, to: dynamic)
  {
      {
        var i = 0;
        while ((i < cpp_cast(G[from_cpp].size())))
        {
          var e = G[from_cpp][i];
          if ((e.to == to))
          {
            if (((e.cap == 0) && (flow(from_cpp, to, 1) == 0)))
            {
              flow(from_cpp, s, 1);
              flow(t, to, 1);
              return 1;
            }
          }
          i += 1;
        }
      }
      return 0;
    }
}

var b = cpp_array(55, 55);

func main()
{
  var h: dynamic;
  var w: dynamic;
  var c: dynamic;
  read(h, w, c);
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          read(b[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var G = cpp_construct((((h * w) / 2) + 2));
  var S = ((h * w) / 2);
  var T = (S + 1);
  var ans = 0;
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          ans += b[i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while (((i + 1) < h))
    {
      if ((i & 1))
      {
        {
          var j = 1;
          while ((j < (w / 2)))
          {
            G.add_edge(((i * ((w / 2))) + j), T, c);
            G.add_edge((((((i - 1)) * ((w / 2))) + j) - 1), ((i * ((w / 2))) + j), b[i][((j * 2) - 1)]);
            G.add_edge((((((i - 1)) * ((w / 2))) + j) - 0), ((i * ((w / 2))) + j), b[i][((j * 2) - 0)]);
            G.add_edge((((((i + 1)) * ((w / 2))) + j) - 1), ((i * ((w / 2))) + j), b[(i + 1)][((j * 2) - 1)]);
            G.add_edge((((((i + 1)) * ((w / 2))) + j) - 0), ((i * ((w / 2))) + j), b[(i + 1)][((j * 2) - 0)]);
            j += 1;
          }
        }
      } else
      {
        {
          var j = 0;
          while ((j < (w / 2)))
          {
            G.add_edge(S, ((i * ((w / 2))) + j), c);
            if ((i == 0))
            {
              G.add_edge(((i * ((w / 2))) + j), T, b[i][((j * 2) + 0)]);
              G.add_edge(((i * ((w / 2))) + j), T, b[i][((j * 2) + 1)]);
            }
            if (((i + 2) == h))
            {
              G.add_edge(((i * ((w / 2))) + j), T, b[(i + 1)][((j * 2) + 0)]);
              G.add_edge(((i * ((w / 2))) + j), T, b[(i + 1)][((j * 2) + 1)]);
            }
            j += 1;
          }
        }
        if ((i != 0))
        {
          G.add_edge((i * ((w / 2))), T, b[i][0]);
          G.add_edge((((i * ((w / 2))) + ((w / 2))) - 1), T, b[i][(w - 1)]);
        }
        if (((i + 2) != h))
        {
          G.add_edge((i * ((w / 2))), T, b[(i + 1)][0]);
          G.add_edge((((i * ((w / 2))) + ((w / 2))) - 1), T, b[(i + 1)][(w - 1)]);
        }
      }
      i += 1;
    }
  }
  write((ans - G.flow(S, T)), "\n");
  return 0;
}
