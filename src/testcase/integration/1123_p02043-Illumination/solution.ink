// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

class Dinic
{
  var INF: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var G: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var level: dynamic = cpp_uninitialized();
  var iter: dynamic = cpp_uninitialized();
  func Dinic() -> dynamic
  {
    }
  func Dinic(sz: dynamic) -> dynamic
  {
      self->n = cpp_construct(sz);
      self->G = cpp_construct(n);
      self->M = cpp_construct(n);
      self->level = cpp_construct(n);
      self->iter = cpp_construct(n);
    }
  func add_edge(from_cpp: dynamic, to: dynamic, cap: dynamic) -> dynamic
  {
      M[from_cpp][to] = G[from_cpp].size();
      M[to][from_cpp] = G[to].size();
      G[from_cpp].push_back(edge(to, cap, G[to].size()));
      G[to].push_back(edge(from_cpp, 0, (G[from_cpp].size() - 1)));
    }
  func bfs(s: dynamic) -> dynamic
  {
      fill(level.begin(), level.end(), -1);
      var que: dynamic = cpp_uninitialized();
      level[s] = 0;
      que.push(s);
      while ((!que.empty()))
      {
        var v: dynamic = que.front();
        que.pop();
        {
          var i: dynamic = 0;
          while ((i < cpp_cast(G[v].size())))
          {
            var e: dynamic = G[v][i];
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
  func dfs(v: dynamic, t: dynamic, f: dynamic) -> dynamic
  {
      if ((v == t))
      {
        return f;
      }
      {
        var i: dynamic = iter[v];
        while ((i < cpp_cast(G[v].size())))
        {
          var e: dynamic = G[v][i];
          if (((e.cap > 0) && (level[v] < level[e.to])))
          {
            var d: dynamic = dfs(e.to, t, min(f, e.cap));
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
  func flow(s: dynamic, t: dynamic, lim: dynamic) -> dynamic
  {
      var fl: dynamic = 0;
      {
        while (true)
        {
          bfs(s);
          if (((level[t] < 0) || (lim == 0)))
          {
            return fl;
          }
          fill(iter.begin(), iter.end(), 0);
          var f: dynamic = cpp_uninitialized();
          while (((cpp_assign(f, "=", dfs(s, t, lim))) > 0))
          {
            fl += f;
            lim -= f;
          }
        }
      }
    }
  func flow(s: dynamic, t: dynamic) -> dynamic
  {
      return flow(s, t, INF);
    }
  func back_edge(s: dynamic, t: dynamic, from_cpp: dynamic, to: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < cpp_cast(G[from_cpp].size())))
        {
          var e: dynamic = G[from_cpp][i];
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

var b: dynamic = cpp_array(55, 55);

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(h, w, c);
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          read(b[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var G: dynamic = cpp_construct((((h * w) / 2) + 2));
  var S: dynamic = ((h * w) / 2);
  var T: dynamic = (S + 1);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while (((i + 1) < h))
    {
      if ((i & 1))
      {
        {
          var j: dynamic = 1;
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
          var j: dynamic = 0;
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
