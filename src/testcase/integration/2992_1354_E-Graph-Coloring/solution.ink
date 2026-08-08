// Translated from solution.cpp.

class graph
{
  var v: dynamic;
  var adj: dynamic;
  var par: dynamic;
  var n: dynamic;
  var depth: dynamic;
  var visited: dynamic;
  var flag: dynamic;
  var vis: dynamic;
  var clr: dynamic;
  var odd: dynamic;
  var even: dynamic;
  func graph(v: dynamic)
  {
      this->v = v;
      flag = true;
      odd = 0;
      even = 0;
      adj = cpp_new();
      par = cpp_new();
      depth = cpp_new();
      visited = cpp_new();
      vis = cpp_new();
      clr = cpp_new();
      {
        var i = 0;
        while ((i < v))
        {
          clr[i] = -1;
          par[i] = -1;
          depth[i] = 0;
          visited[i] = false;
          vis[i] = false;
          i += 1;
        }
      }
    }
  func add(a: dynamic, b: dynamic)
  {
      adj[a].push_back(b);
      adj[b].push_back(a);
    }
}

func dfs(i: dynamic, d: dynamic)
{
  visited[i] = true;
  depth[i] = d;
  if ((d & 1))
  {
    odd += 1;
  } else
  {
    even += 1;
  }
  for (var t in adj[i])
  {
    if (((t != par[i]) && visited[t]))
    {
      if (((((depth[t] - depth[i])) % 2) == 0))
      {
        flag = false;
      }
    }
    if ((!visited[t]))
    {
      par[t] = i;
      dfs(t, (d + 1));
    }
  }
}

func dfs1(i: dynamic, d: dynamic, cl: dynamic, k: dynamic)
{
  vis[i] = true;
  if ((d & 1))
  {
    if ((cl.first == 1))
    {
      clr[i] = 2;
    } else
    {
      if ((k >= n))
      {
        clr[i] = 3;
      } else
      {
        clr[i] = 1;
        k += 1;
      }
    }
  } else
  {
    if ((cl.second == 1))
    {
      clr[i] = 2;
    } else
    {
      if ((k >= n))
      {
        clr[i] = 3;
      } else
      {
        clr[i] = 1;
        k += 1;
      }
    }
  }
  for (var t in adj[i])
  {
    if ((!vis[t]))
    {
      dfs1(t, (d + 1), cl, k);
    }
  }
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var n1: dynamic;
  var n2: dynamic;
  var n3: dynamic;
  read(n1, n2, n3);
  g.n = n1;
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      u -= 1;
      v -= 1;
      g.add(u, v);
      i += 1;
    }
  }
  var v: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if ((!g.visited[i]))
      {
        g.dfs(i, 0);
        v.push_back([g.odd, g.even]);
        g.odd = 0;
        g.even = 0;
      }
      i += 1;
    }
  }
  if ((!g.flag))
  {
    write("NO", "\n");
    return;
  }
  var dp = cpp_array((n + 1), (v.size() + 1));
  memset(dp, -1, cpp_sizeof(dp));
  dp[0][0] = 1;
  {
    var i = 1;
    while ((i <= v.size()))
    {
      {
        var j = 0;
        while ((j <= n))
        {
          if ((((j - v[(i - 1)].first) >= 0) && (dp[(i - 1)][(j - v[(i - 1)].first)] >= 0)))
          {
            dp[i][j] = 0;
          }
          if ((((j - v[(i - 1)].second) >= 0) && (dp[(i - 1)][(j - v[(i - 1)].second)] >= 0)))
          {
            dp[i][j] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((dp[v.size()][n2] == -1))
  {
    write("NO", "\n");
  } else
  {
    write("YES", "\n");
    var e = n2;
    var o = (n1 + n2);
    var cl: dynamic;
    {
      var i = v.size();
      while ((i > 0))
      {
        if ((dp[i][e] == 1))
        {
          cl.push_back([0, 1]);
          e -= v[(i - 1)].second;
        } else
        {
          cl.push_back([1, 0]);
          e -= v[(i - 1)].first;
        }
        i -= 1;
      }
    }
    var k = 0;
    var idx = (cl.size() - 1);
    {
      var i = 0;
      while ((i < n))
      {
        if ((!g.vis[i]))
        {
          g.dfs1(i, 0, cl[idx], k);
          idx -= 1;
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        write(g.clr[i]);
        i += 1;
      }
    }
  }
}

func main()
{
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
