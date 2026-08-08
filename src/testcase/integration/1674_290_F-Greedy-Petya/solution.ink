// Translated from solution.cpp.

var mod = (1e6 + 3);

var INF = 2e15;

var a = cpp_array(1234, 1234);

var adj = cpp_array(32);

var deg = cpp_array(34);

var vis = cpp_array(34);

func cmp(x: dynamic, y: dynamic)
{
  if ((deg[x] != deg[y]))
  {
    return (deg[x] < deg[y]);
  }
  return (x < y);
}

func dfs(u: dynamic)
{
  vis[u] = 1;
  {
    var i = 0;
    while ((i < cpp_cast(adj[u].size())))
    {
      var nxt = adj[u][i];
      if ((nxt == u))
      {
        i += 1;
        continue;
      }
      if (vis[nxt])
      {
        i += 1;
        continue;
      }
      dfs(nxt);
      break;
      i += 1;
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 1;
    while ((i <= m))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      if ((x != y))
      {
        a[x][y] = cpp_assign(a[y][x], "=", true);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          if ((a[i][j] == true))
          {
            deg[i] += 1;
            adj[i].push_back(j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sort(adj[i].begin(), adj[i].end(), cmp);
      i += 1;
    }
  }
  var f = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      memset(vis, 0, cpp_sizeof((vis)));
      dfs(i);
      if ((count((vis + 1), ((vis + n) + 1), true) == n))
      {
        f = 1;
        break;
      }
      i += 1;
    }
  }
  if (f)
  {
    write("Yes", cpp_char("\n"));
  } else
  {
    write("No", cpp_char("\n"));
  }
  return 0;
}
