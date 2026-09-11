// Translated from solution.cpp.

var g: dynamic = cpp_array(300010);

var ans: dynamic = cpp_uninitialized();

var rec: dynamic = cpp_uninitialized();

var edge: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var dis: dynamic = cpp_array(300010);

var fa: dynamic = cpp_array(300010);

var in_cpp: dynamic = cpp_array(300010);

var cls: dynamic = cpp_array(300010);

var vis: dynamic = cpp_array(300010);

func dfs(u: dynamic) -> dynamic
{
  rec.push_back(u);
  vis[u] = 1;
  for (var v: dynamic in g[u])
  {
    if ((!vis[v]))
    {
      dfs(v);
    }
  }
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d %d", (&u), (&v));
      g[u].push_back(v);
      g[v].push_back(u);
      edge.insert(make_pair(u, v));
      edge.insert(make_pair(v, u));
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      dis[i] = 1e9;
      i += 1;
    }
  }
  var que: dynamic = cpp_uninitialized();
  que.push(1);
  while ((!que.empty()))
  {
    var u: dynamic = que.front();
    que.pop();
    in_cpp[u] = 0;
    for (var v: dynamic in g[u])
    {
      if ((dis[v] > (dis[u] + 1)))
      {
        dis[v] = (dis[u] + 1);
        fa[v] = u;
        if ((!in_cpp[v]))
        {
          in_cpp[v] = 1;
          que.push(v);
        }
      }
    }
  }
  if ((dis[n] != 1e9))
  {
    {
      var u: dynamic = n;
      while (u)
      {
        ans.push_back(u);
        u = fa[u];
      }
    }
    reverse(ans.begin(), ans.end());
  }
  if ((ans.empty() || (ans.size() > 4)))
  {
    for (var u: dynamic in g[1])
    {
      cls[u] = 1;
    }
    cls[1] = 1;
    var ok: dynamic = false;
    for (var u: dynamic in g[1])
    {
      for (var v: dynamic in g[u])
      {
        if ((!cls[v]))
        {
          ans = [1, u, v, 1, n];
          ok = true;
          break;
        }
      }
      if (ok)
      {
        break;
      }
    }
  }
  if ((ans.empty() || (ans.size() > 5)))
  {
    vis[1] = 1;
    var ok: dynamic = false;
    for (var u: dynamic in g[1])
    {
      if (vis[u])
      {
        continue;
      }
      rec.clear();
      dfs(u);
      if ((g[u].size() != rec.size()))
      {
        var cur: dynamic = 1;
        while (((cur < rec.size()) && edge.count([u, rec[cur]])))
        {
          cur += 1;
        }
        {
          var i: dynamic = 1;
          while ((i < cur))
          {
            if (edge.count([rec[i], rec[cur]]))
            {
              ans = [1, u, rec[i], rec[cur], u, n];
              ok = true;
            }
            i += 1;
          }
        }
      } else
      {
        {
          var i: dynamic = 0;
          while ((i < g[u].size()))
          {
            {
              var j: dynamic = (i + 1);
              while ((j < g[u].size()))
              {
                if ((((g[u][i] != 1) && (g[u][j] != 1)) && (!edge.count([g[u][i], g[u][j]]))))
                {
                  ans = [1, g[u][i], u, g[u][j], g[u][i], n];
                  ok = true;
                  break;
                }
                j += 1;
              }
            }
            i += 1;
          }
        }
        if (ok)
        {
          break;
        }
      }
      if (ok)
      {
        break;
      }
    }
  }
  if (ans.empty())
  {
    puts("-1");
  } else
  {
    printf("%d\n", (cpp_cast(ans.size()) - 1));
    for (var u: dynamic in ans)
    {
      printf("%d ", u);
    }
    puts("");
  }
  return 0;
}
