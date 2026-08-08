// Translated from solution.cpp.

var maxn = 112345;

var n: dynamic;

var m: dynamic;

var l: dynamic;

var r: dynamic;

var x: dynamic;

var d = cpp_array(maxn);

var vis = cpp_array(maxn);

var G = cpp_array(maxn);

func dfs(u: dynamic, dep: dynamic)
{
  vis[u] = 1;
  d[u] = dep;
  {
    var i = 0;
    while ((i < cpp_cast(G[u].size())))
    {
      var v = G[u][i].first;
      var w = G[u][i].second;
      if ((vis[v] && ((d[u] + w) != d[v])))
      {
        return false;
      }
      if (((!vis[v]) && (!dfs(v, (dep + w)))))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d%d%d", (&l), (&r), (&x));
      G[l].push_back(pii(r, x));
      G[r].push_back(pii(l, (-x)));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (((!vis[i]) && (!dfs(i, 0))))
      {
        return (0 * puts("No"));
      }
      i += 1;
    }
  }
  return (0 * puts("Yes"));
}
