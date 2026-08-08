// Translated from solution.cpp.

func mo(a: dynamic)
{
  return (a % cpp_cast(((1e9 + 7))));
}

func po(x: dynamic, y: dynamic, p: dynamic)
{
  var res = 1;
  x = (x % p);
  while ((y > 0))
  {
    if ((y & 1))
    {
      res = (((res * x)) % p);
    }
    y >>= 1;
    x = (((x * x)) % p);
  }
  return (res % p);
}

var g: dynamic;

var vis: dynamic;

var col: dynamic;

var ty: dynamic;

func dfs(par: dynamic, anc: dynamic = -1, type_cpp: dynamic = -1)
{
  if ((anc == -1))
  {
    col[par] = 0;
    ty[col[par]].push_back(par);
  } else
  {
    if (type_cpp)
    {
      col[par] = (!col[anc]);
      ty[col[par]].push_back(par);
    } else
    {
      col[par] = col[anc];
      ty[col[par]].push_back(par);
    }
  }
  vis[par] = 1;
  for (var e in g[par])
  {
    if ((!vis[e.first]))
    {
      dfs(e.first, par, e.second);
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var r = cpp_construct(n, vector(2, -1));
    var c = cpp_construct(n, vector(2, -1));
    {
      var i = 0;
      while ((i < 2))
      {
        {
          var j = 0;
          while ((j < n))
          {
            var x: dynamic;
            read(x);
            x -= 1;
            if ((r[x][0] == -1))
            {
              r[x][0] = i;
            } else
            {
              r[x][1] = i;
            }
            if ((c[x][0] == -1))
            {
              c[x][0] = j;
            } else
            {
              c[x][1] = j;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var pos = 1;
    {
      var i = 0;
      while ((i < n))
      {
        if (((r[i][0] == -1) || (r[i][1] == -1)))
        {
          pos = 0;
        }
        i += 1;
      }
    }
    if ((!pos))
    {
      write(-1, "\n");
      continue;
    }
    g.assign(n, []);
    vis.assign(n, 0);
    col.assign(n, -1);
    ty.assign(2, []);
    {
      var i = 0;
      while ((i < n))
      {
        if (((r[i][0] == r[i][1]) && (c[i][0] != c[i][1])))
        {
          g[c[i][0]].push_back([c[i][1], 1]);
          g[c[i][1]].push_back([c[i][0], 1]);
        }
        if (((r[i][0] != r[i][1]) && (c[i][0] != c[i][1])))
        {
          g[c[i][1]].push_back([c[i][0], 0]);
          g[c[i][0]].push_back([c[i][1], 0]);
        }
        i += 1;
      }
    }
    var ans: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        if ((!vis[i]))
        {
          ty[0].clear();
          ty[1].clear();
          dfs(i);
          if ((ty[0].size() > ty[1].size()))
          {
            swap(ty[0], ty[1]);
          }
          for (var e in ty[0])
          {
            ans.push_back(e);
          }
        }
        i += 1;
      }
    }
    write(ans.size(), "\n");
    for (var e in ans)
    {
      write((e + 1), " ");
    }
    write("\n");
  }
  return 0;
}
