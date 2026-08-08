// Translated from solution.cpp.

var maxn = 202;

var maxs = 1000004;

var data = cpp_array((maxn * maxn));

var n: dynamic;

var m: dynamic;

var q: dynamic;

var iscir = cpp_array((maxn * maxn));

var cir = cpp_array(10, (maxn * maxn));

var N = cpp_array((maxn * maxn));

var X = cpp_array(10);

var Y = cpp_array(10);

func readin()
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          var c = cpp_char("\n");
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
    var i = 0;
    while ((i < 10))
    {
      scanf("%d%d", (&X[i]), (&Y[i]));
      i += 1;
    }
  }
}

var vis = cpp_array((maxn * maxn));

func next(u: dynamic)
{
  var x = (u / m);
  var y = (u % m);
  x += X[data[u]];
  y += Y[data[u]];
  var v: dynamic;
  if (((((x >= n) || (y >= m)) || (x < 0)) || (y < 0)))
  {
    v = u;
  } else
  {
    v = ((x * m) + y);
  }
  return v;
}

func find_cir(u: dynamic)
{
  vis[u] = -1;
  var v = N[u];
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

func get_cir(i: dynamic, u: dynamic)
{
  cir[i][data[u]] = 1;
  vis[u] = 1;
  var v = N[u];
  if (vis[v])
  {
    return;
  }
  get_cir(i, v);
}

var s = cpp_array(maxs);

var is = cpp_array(10, maxs);

var ishead = cpp_array((maxn * maxn));

func head_dfs(u: dynamic)
{
  vis[u] = 1;
  if (vis[N[u]])
  {
    return;
  }
  head_dfs(N[u]);
}

func get_head()
{
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i = 0;
    while ((i < (n * m)))
    {
      vis[N[i]] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
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
    var i = 0;
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
    var i = 0;
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

func judge()
{
  var len = strlen(s);
  {
    var i = (len - 1);
    while ((i >= 0))
    {
      memset(is[i], false, cpp_sizeof((is[i])));
      if ((i != (len - 1)))
      {
        {
          var j = 0;
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
    var i = 0;
    while ((i < (n * m)))
    {
      if (ishead[i])
      {
        var u = i;
        var cur = 0;
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
        var flag = 1;
        {
          var j = 0;
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

func solve()
{
  memset(iscir, 0, cpp_sizeof((iscir)));
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i = 0;
    while ((i < (n * m)))
    {
      N[i] = next(i);
      {
        var j = 0;
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
    var i = 0;
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
    var i = 0;
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
    var i = 0;
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

func main()
{
  readin();
  solve();
  return 0;
}
