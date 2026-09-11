// Translated from solution.cpp.

var maxn: dynamic = 112345;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(maxn);

var vis: dynamic = cpp_array(maxn);

var G: dynamic = cpp_array(maxn);

func dfs(u: dynamic, dep: dynamic) -> dynamic
{
  vis[u] = 1;
  d[u] = dep;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[u].size())))
    {
      var v: dynamic = G[u][i].first;
      var w: dynamic = G[u][i].second;
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

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d%d%d", (&l), (&r), (&x));
      G[l].push_back(pii(r, x));
      G[r].push_back(pii(l, (-x)));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
