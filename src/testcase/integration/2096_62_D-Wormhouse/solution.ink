// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a = cpp_array(2020);

var b = cpp_array(2020);

var vis = cpp_array(128);

var e = cpp_array(2020);

var g = cpp_array(128);

func dfs(v: dynamic)
{
  var i: dynamic;
  var u: dynamic;
  {
    i = 0;
    while ((i < g[v].size()))
    {
      if ((e[g[v][i].second] != 0))
      {
        i += 1;
        continue;
      }
      u = g[v][i].first;
      if ((vis[u] == 0))
      {
        vis[u] = 1;
        dfs(u);
      }
      i += 1;
    }
  }
}

func rec(v: dynamic, sm: dynamic, pa: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var u: dynamic;
  var fl = 0;
  b[(pa - 1)] = v;
  if ((sm == 0))
  {
    {
      i = 0;
      while ((i < g[v].size()))
      {
        k = g[v][i].second;
        if ((e[k] == 1))
        {
          i += 1;
          continue;
        }
        u = g[v][i].first;
        if ((((sm == 0)) && ((u == a[pa]))))
        {
          e[k] = 1;
          if ((rec(u, 0, (pa + 1)) == 1))
          {
            return 1;
          }
          e[k] = 0;
        } else if ((u > a[pa]))
        {
          e[k] = 1;
          memset(vis, 0, cpp_sizeof((vis)));
          dfs(v);
          {
            j = 1;
            while ((j <= n))
            {
              if ((j == v))
              {
                j += 1;
                continue;
              }
              if ((vis[j] == 1))
              {
                break;
              }
              j += 1;
            }
          }
          if ((((j > n)) || ((vis[u] == 1))))
          {
            rec(u, 1, (pa + 1));
            return 1;
          } else
          {
            e[k] = 0;
          }
        }
        i += 1;
      }
    }
  } else
  {
    {
      i = 0;
      while ((i < g[v].size()))
      {
        k = g[v][i].second;
        u = g[v][i].first;
        if ((e[k] == 1))
        {
          i += 1;
          continue;
        }
        e[k] = 1;
        memset(vis, 0, cpp_sizeof((vis)));
        dfs(v);
        {
          j = 1;
          while ((j <= n))
          {
            if ((j == v))
            {
              j += 1;
              continue;
            }
            if ((vis[j] == 1))
            {
              break;
            }
            j += 1;
          }
        }
        if ((((j > n)) || ((vis[u] == 1))))
        {
          rec(u, 1, (pa + 1));
          return 1;
        } else
        {
          e[k] = 0;
        }
        i += 1;
      }
    }
  }
  return 0;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  scanf("%d %d", (&n), (&m));
  {
    i = 1;
    while ((i <= (m + 1)))
    {
      scanf("%d", (&a[i]));
      if ((i > 1))
      {
        g[a[i]].push_back(make_pair(a[(i - 1)], i));
        g[a[(i - 1)]].push_back(make_pair(a[i], i));
      }
      i += 1;
    }
  }
  memset(e, 0, cpp_sizeof((e)));
  {
    i = 1;
    while ((i <= n))
    {
      sort(g[i].begin(), g[i].end());
      i += 1;
    }
  }
  b[1] = a[1];
  if ((rec(a[1], 0, 2) == 0))
  {
    printf("No solution\n");
    return 0;
  }
  {
    i = 1;
    while ((i <= m))
    {
      printf("%d ", b[i]);
      i += 1;
    }
  }
  printf("%d\n", b[(m + 1)]);
  return 0;
}
