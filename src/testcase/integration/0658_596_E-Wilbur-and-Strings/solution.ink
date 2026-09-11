// Translated from solution.cpp.

var maxn: dynamic = 202;

var maxs: dynamic = 1000004;

var data: dynamic = cpp_array((maxn * maxn));

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var iscir: dynamic = cpp_array((maxn * maxn));

var cir: dynamic = cpp_array(10, (maxn * maxn));

var N: dynamic = cpp_array((maxn * maxn));

var X: dynamic = cpp_array(10);

var Y: dynamic = cpp_array(10);

func readin() -> dynamic
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var c: dynamic = cpp_char("\n");
          while ((c == cpp_char("\n")))
          {
            scanf("%c", (&c));
          }
          data[((i * m) + j)] = (c - cpp_char("0"));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 10))
    {
      scanf("%d%d", (&X[i]), (&Y[i]));
      i += 1;
    }
  }
}

var vis: dynamic = cpp_array((maxn * maxn));

func next(u: dynamic) -> dynamic
{
  var x: dynamic = (u / m);
  var y: dynamic = (u % m);
  x += X[data[u]];
  y += Y[data[u]];
  var v: dynamic = cpp_uninitialized();
  if (((((x >= n) || (y >= m)) || (x < 0)) || (y < 0)))
  {
    v = u;
  } else
  {
    v = ((x * m) + y);
  }
  return v;
}

func find_cir(u: dynamic) -> dynamic
{
  vis[u] = -1;
  var v: dynamic = N[u];
  if ((vis[v] != 1))
  {
    if ((vis[v] == -1))
    {
      iscir[v] = 1;
    } else
    {
      find_cir(v);
    }
  }
  vis[u] = 1;
}

func get_cir(i: dynamic, u: dynamic) -> dynamic
{
  cir[i][data[u]] = 1;
  vis[u] = 1;
  var v: dynamic = N[u];
  if (vis[v])
  {
    return;
  }
  get_cir(i, v);
}

var s: dynamic = cpp_array(maxs);

var is: dynamic = cpp_array(10, maxs);

var ishead: dynamic = cpp_array((maxn * maxn));

func head_dfs(u: dynamic) -> dynamic
{
  vis[u] = 1;
  if (vis[N[u]])
  {
    return;
  }
  head_dfs(N[u]);
}

func get_head() -> dynamic
{
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      vis[N[i]] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      if ((!vis[i]))
      {
        ishead[i] = true;
      }
      i += 1;
    }
  }
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      if (ishead[i])
      {
        head_dfs(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      if ((iscir[i] && (!vis[i])))
      {
        ishead[i] = true;
      }
      i += 1;
    }
  }
}

func judge() -> dynamic
{
  var len: dynamic = strlen(s);
  {
    var i: dynamic = (len - 1);
    while ((i >= 0))
    {
      memset(is[i], false, cpp_sizeof((is[i])));
      if ((i != (len - 1)))
      {
        {
          var j: dynamic = 0;
          while ((j < 10))
          {
            is[i][j] = is[(i + 1)][j];
            j += 1;
          }
        }
      }
      is[i][(s[i] - cpp_char("0"))] = true;
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      if (ishead[i])
      {
        var u: dynamic = i;
        var cur: dynamic = 0;
        while ((!iscir[u]))
        {
          while (((s[cur] - cpp_char("0")) != data[u]))
          {
            u = N[u];
            if (iscir[u])
            {
              break;
            }
          }
          if ((!iscir[u]))
          {
            u = N[u];
            cur += 1;
            if ((cur == len))
            {
              return true;
            }
          }
        }
        var flag: dynamic = 1;
        {
          var j: dynamic = 0;
          while ((j < 10))
          {
            if ((is[cur][j] && (!cir[u][j])))
            {
              flag = 0;
            }
            j += 1;
          }
        }
        if (flag)
        {
          return true;
        }
      }
      i += 1;
    }
  }
  return false;
}

func solve() -> dynamic
{
  memset(iscir, 0, cpp_sizeof((iscir)));
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      N[i] = next(i);
      {
        var j: dynamic = 0;
        while ((j < 10))
        {
          cir[i][j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      if ((vis[i] != 1))
      {
        find_cir(i);
      }
      i += 1;
    }
  }
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i: dynamic = 0;
    while ((i < (n * m)))
    {
      if (iscir[i])
      {
        get_cir(i, i);
      }
      i += 1;
    }
  }
  get_head();
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      scanf("%s", s);
      if (judge())
      {
        printf("YES\n");
      } else
      {
        printf("NO\n");
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  readin();
  solve();
  return 0;
}
