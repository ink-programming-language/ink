// Translated from solution.cpp.

var M: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var dis: dynamic = cpp_array(100100);

var ans: dynamic = 1;

var mark: dynamic = cpp_array(100100);

var g: dynamic = cpp_array(100100);

func dfs(a: dynamic) -> dynamic
{
  mark[a] = true;
  {
    var i: dynamic = 0;
    while ((i < g[a].size()))
    {
      if ((!mark[g[a][i].first]))
      {
        dis[g[a][i].first] = (dis[a] + g[a][i].second);
        dfs(g[a][i].first);
      } else
      {
        if (((((dis[g[a][i].first] & 1)) == ((dis[a] & 1))) && g[a][i].second))
        {
          ans = 0;
        }
        if (((((dis[g[a][i].first] & 1)) != ((dis[a] & 1))) && (!g[a][i].second)))
        {
          ans = 0;
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(v, u, t);
      t = (!t);
      g[v].push_back(make_pair(u, t));
      g[u].push_back(make_pair(v, t));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!mark[i]))
      {
        dfs(i);
        cnt += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < cnt))
    {
      ans = (((ans * 2)) % M);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
