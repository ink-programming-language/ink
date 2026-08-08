// Translated from solution.cpp.

var Maxn = 100;

var n: dynamic;

var t = cpp_array((Maxn + 5));

var vis = cpp_array((Maxn + 5));

class Bit
{
  var a: dynamic;
  var id: dynamic;
  func count()
  {
      {
        var i = n;
        while ((i >= 0))
        {
          if (a[i])
          {
            return i;
          }
          i -= 1;
        }
      }
      return -1;
    }
}

var f = cpp_array((Maxn + 5));

class Edge
{
  var u: dynamic;
  var v: dynamic;
}

var edge = cpp_array((Maxn + 5));

var col: dynamic;

var q: dynamic;

var g = cpp_array((Maxn + 5));

func check(a: dynamic)
{
  if (a.empty())
  {
    return 1;
  }
  {
    var it = a.begin();
    while ((it != a.end()))
    {
      q.push((*it));
      it += 1;
    }
  }
  {
    var i = n;
    while ((i >= 0))
    {
      if ((t[i] == 0))
      {
        i -= 1;
        continue;
      }
      var u = q.top();
      q.pop();
      var now = u.count();
      if ((now > i))
      {
        break;
      } else if ((now == i))
      {
        u.a[now] = 0;
        q.push(u);
      }
      if (q.empty())
      {
        return 1;
      }
      i -= 1;
    }
  }
  while ((!q.empty()))
  {
    q.pop();
  }
  return 0;
}

func re_build(x: dynamic, a: dynamic)
{
  if (a.empty())
  {
    return;
  }
  {
    var it = a.begin();
    while ((it != a.end()))
    {
      q.push((*it));
      it += 1;
    }
  }
  {
    var i = n;
    while ((i >= 0))
    {
      if ((!t[i]))
      {
        i -= 1;
        continue;
      }
      var u = q.top();
      q.pop();
      var now = u.count();
      if ((now == i))
      {
        u.a[now] = 0;
        q.push(u);
      } else
      {
        col[make_pair(u.id, x)] = cpp_assign(col[make_pair(x, u.id)], "=", i);
      }
      if (q.empty())
      {
        return;
      }
      i -= 1;
    }
  }
  while ((!q.empty()))
  {
    q.pop();
  }
}

func init_dfs(u: dynamic, fa: dynamic)
{
  var a: dynamic;
  f[u].id = u;
  {
    var i = 0;
    while ((i < cpp_cast(g[u].size())))
    {
      var v = g[u][i];
      if ((v == fa))
      {
        i += 1;
        continue;
      }
      init_dfs(v, u);
      a.push_back(f[v]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      t[i] = 1;
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 0))
    {
      t[i] = 0;
      if ((!check(a)))
      {
        t[i] = 1;
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      f[u].a[i] = t[i];
      i += 1;
    }
  }
  re_build(u, a);
}

var maxn: dynamic;

var id: dynamic;

func work_dfs(u: dynamic, fa: dynamic)
{
  {
    var i = 0;
    while ((i < cpp_cast(g[u].size())))
    {
      var v = g[u][i];
      if ((vis[v] || (v == fa)))
      {
        i += 1;
        continue;
      }
      var k = col[make_pair(u, v)];
      if ((k > maxn))
      {
        maxn = k;
        id = make_pair(u, v);
      }
      work_dfs(v, u);
      i += 1;
    }
  }
}

func solve(u: dynamic, fa: dynamic)
{
  maxn = -1;
  work_dfs(u, fa);
  if ((maxn == -1))
  {
    printf("! %d\n", u);
    fflush(stdout);
    return;
  }
  printf("? %d %d\n", id.first, id.second);
  fflush(stdout);
  var r: dynamic;
  scanf("%d", (&r));
  if ((r == id.first))
  {
    vis[id.second] = 1;
    solve(id.first, id.first);
  } else
  {
    vis[id.first] = 1;
    solve(id.second, id.second);
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      g[u].push_back(v);
      g[v].push_back(u);
      i += 1;
    }
  }
  init_dfs(1, 1);
  solve(1, 1);
  return 0;
}
