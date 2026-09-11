// Translated from solution.cpp.

var maxn: dynamic = (2000 + 10);

var maxE: dynamic = (2000000 + 10);

var DEBUG: dynamic = 0;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var val: dynamic = cpp_array(maxn);

var S: dynamic = cpp_array(maxn);

class anode
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  func anode() -> dynamic
  {
    }
  func anode(u: dynamic, v: dynamic, w: dynamic) -> dynamic
  {
      self->u = cpp_construct(u);
      self->v = cpp_construct(v);
      self->w = cpp_construct(w);
    }
}

var a: dynamic = cpp_array(maxE);

var h: dynamic = cpp_array(maxn);

var ecnt: dynamic = cpp_uninitialized();

var sfa: dynamic = cpp_array(maxn);

class enode
{
  var v: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  func enode() -> dynamic
  {
    }
  func enode(v: dynamic, n: dynamic) -> dynamic
  {
      self->v = cpp_construct(v);
      self->n = cpp_construct(n);
    }
}

var e: dynamic = cpp_array((maxE << 1));

func addedge(u: dynamic, v: dynamic) -> dynamic
{
  ecnt += 1;
  e[ecnt] = enode(v, h[u]);
  h[u] = ecnt;
}

func findfa(u: dynamic) -> dynamic
{
  return  (((sfa[u] == u))) ? (u) : (cpp_assign(sfa[u], "=", findfa(sfa[u])));
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.w > b.w);
}

var flag: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(maxn, maxn);

var all: dynamic = cpp_uninitialized();

func dfs(u: dynamic, fa: dynamic, k: dynamic) -> dynamic
{
  if (vis[k][u])
  {
    all += 1;
  }
  vis[k][u] = 0;
  {
    var i: dynamic = h[u];
    while ((~i))
    {
      var v: dynamic = e[i].v;
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

func dfs_ans(u: dynamic, fa: dynamic) -> dynamic
{
  {
    var i: dynamic = h[u];
    while ((~i))
    {
      var v: dynamic = e[i].v;
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

func solve() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      val[i].reset();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%s", (S + 1));
      var cnt: dynamic = 0;
      {
        var j: dynamic = 1;
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
        var j: dynamic = 1;
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
        var j: dynamic = 1;
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
  var tot: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= n))
        {
          var w: dynamic = ((val[i] & val[j])).count();
          a[cpp_update(tot, "++")] = anode(i, j, w);
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort((a + 1), ((a + tot) + 1), cmp);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sfa[i] = i;
      h[i] = -1;
      i += 1;
    }
  }
  ecnt = 0;
  {
    var i: dynamic = 1;
    while ((i <= tot))
    {
      var u: dynamic = a[i].u;
      var v: dynamic = a[i].v;
      var f1: dynamic = findfa(u);
      var f2: dynamic = findfa(v);
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
    var i: dynamic = 1;
    while ((i <= m))
    {
      all = 0;
      var now: dynamic = 0;
      {
        var j: dynamic = 1;
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
        var j: dynamic = 1;
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

func main() -> dynamic
{
  var cas: dynamic = cpp_uninitialized();
  scanf("%d", (&cas));
  while (cpp_update(cas, "--"))
  {
    solve();
  }
  return 0;
}
