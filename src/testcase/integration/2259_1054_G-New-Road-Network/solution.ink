// Translated from solution.cpp.

var maxn = (2000 + 10);

var maxE = (2000000 + 10);

var DEBUG = 0;

var n: dynamic;

var m: dynamic;

var val = cpp_array(maxn);

var S = cpp_array(maxn);

class anode
{
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
  func anode()
  {
    }
  func anode(u: dynamic, v: dynamic, w: dynamic)
  {
      this->u = cpp_construct(u);
      this->v = cpp_construct(v);
      this->w = cpp_construct(w);
    }
}

var a = cpp_array(maxE);

var h = cpp_array(maxn);

var ecnt: dynamic;

var sfa = cpp_array(maxn);

class enode
{
  var v: dynamic;
  var n: dynamic;
  func enode()
  {
    }
  func enode(v: dynamic, n: dynamic)
  {
      this->v = cpp_construct(v);
      this->n = cpp_construct(n);
    }
}

var e = cpp_array((maxE << 1));

func addedge(u: dynamic, v: dynamic)
{
  ecnt += 1;
  e[ecnt] = enode(v, h[u]);
  h[u] = ecnt;
}

func findfa(u: dynamic)
{
  return if (((sfa[u] == u))) (u) else (cpp_assign(sfa[u], "=", findfa(sfa[u])));
}

func cmp(a: dynamic, b: dynamic)
{
  return (a.w > b.w);
}

var flag: dynamic;

var vis = cpp_array(maxn, maxn);

var all: dynamic;

func dfs(u: dynamic, fa: dynamic, k: dynamic)
{
  if (vis[k][u])
  {
    all += 1;
  }
  vis[k][u] = 0;
  {
    var i = h[u];
    while ((~i))
    {
      var v = e[i].v;
      if ((v == fa))
      {
        i = e[i].n;
        continue;
      }
      if ((!vis[k][v]))
      {
        i = e[i].n;
        continue;
      }
      dfs(v, u, k);
      i = e[i].n;
    }
  }
}

func dfs_ans(u: dynamic, fa: dynamic)
{
  {
    var i = h[u];
    while ((~i))
    {
      var v = e[i].v;
      if ((v == fa))
      {
        i = e[i].n;
        continue;
      }
      printf("%d %d\n", u, v);
      dfs_ans(v, u);
      i = e[i].n;
    }
  }
}

func solve()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      val[i].reset();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%s", (S + 1));
      var cnt = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if ((S[j] == cpp_char("1")))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      {
        var j = 1;
        while ((j <= n))
        {
          vis[i][j] = 0;
          j += 1;
        }
      }
      if ((cnt <= 1))
      {
        i += 1;
        continue;
      }
      {
        var j = 1;
        while ((j <= n))
        {
          if ((S[j] == cpp_char("1")))
          {
            val[j][i] = 1;
            vis[i][j] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var tot = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = (i + 1);
        while ((j <= n))
        {
          var w = ((val[i] & val[j])).count();
          a[cpp_update(tot, "++")] = anode(i, j, w);
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort((a + 1), ((a + tot) + 1), cmp);
  {
    var i = 1;
    while ((i <= n))
    {
      sfa[i] = i;
      h[i] = -1;
      i += 1;
    }
  }
  ecnt = 0;
  {
    var i = 1;
    while ((i <= tot))
    {
      var u = a[i].u;
      var v = a[i].v;
      var f1 = findfa(u);
      var f2 = findfa(v);
      if ((f1 == f2))
      {
        i += 1;
        continue;
      }
      if (DEBUG)
      {
        printf("%d %d %d\n", u, v, a[i].w);
      }
      addedge(u, v);
      addedge(v, u);
      sfa[f1] = f2;
      i += 1;
    }
  }
  flag = 1;
  {
    var i = 1;
    while ((i <= m))
    {
      all = 0;
      var now = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if (vis[i][j])
          {
            now += 1;
          }
          j += 1;
        }
      }
      {
        var j = 1;
        while ((j <= n))
        {
          if (vis[i][j])
          {
            dfs(j, 0, i);
            break;
          }
          j += 1;
        }
      }
      if ((now != all))
      {
        flag = 0;
      }
      i += 1;
    }
  }
  if (flag)
  {
    puts("YES");
    dfs_ans(1, 0);
    return;
  }
  puts("NO");
}

func main()
{
  var cas: dynamic;
  scanf("%d", (&cas));
  while (cpp_update(cas, "--"))
  {
    solve();
  }
  return 0;
}
