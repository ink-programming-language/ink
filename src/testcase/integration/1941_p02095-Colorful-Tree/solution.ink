// Translated from solution.cpp.

var pb = cpp_expression("#include<");

var mp = cpp_expression("#include<");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func chkmax(x: dynamic, y: dynamic)
{
  return if ((x < y)) cpp_comma(cpp_assign(x, "=", y), true) else false;
}

func chkmin(x: dynamic, y: dynamic)
{
  return if ((x > y)) cpp_comma(cpp_assign(x, "=", y), true) else false;
}

func readint()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var n: dynamic;

var m: dynamic;

var tot: dynamic;

var now: dynamic;

var v = cpp_array(200005);

var nxt = cpp_array(200005);

var h = cpp_array(100005);

var siz = cpp_array(100005);

var f = cpp_array(200005);

var son = cpp_array(100005);

var col = cpp_array(200005);

var a = cpp_array(100005);

var b = cpp_array(100005);

var ans = cpp_array(100005);

var vis = cpp_array(200005);

var gar: dynamic;

func getf(x: dynamic)
{
  return if ((x == f[x])) x else cpp_assign(f[x], "=", getf(f[x]));
}

func addedge(x: dynamic, y: dynamic)
{
  v[cpp_update(tot, "++")] = y;
  nxt[tot] = h[x];
  h[x] = tot;
  v[cpp_update(tot, "++")] = x;
  nxt[tot] = h[y];
  h[y] = tot;
}

func dfs1(u: dynamic, fa: dynamic)
{
  siz[u] = 1;
  {
    var p = h[u];
    while (p)
    {
      if ((v[p] == fa))
      {
        p = nxt[p];
        continue;
      }
      dfs1(v[p], u);
      if ((siz[v[p]] > siz[son[u]]))
      {
        son[u] = v[p];
      }
      siz[u] += siz[v[p]];
      p = nxt[p];
    }
  }
}

func change(x: dynamic, y: dynamic)
{
  if ((!col[x]))
  {
    gar.pb(x);
    col[x] = 1;
  }
  if ((!col[y]))
  {
    gar.pb(y);
    col[y] = 1;
  }
  var fx = getf(x);
  var fy = getf(y);
  if ((fx == fy))
  {
    if ((!vis[fx]))
    {
      now += 1;
      vis[fx] = 1;
    }
    return;
  }
  f[fx] = fy;
  if (((!vis[fx]) && (!vis[fy])))
  {
    now += 1;
    return;
  }
  if ((vis[fx] && vis[fy]))
  {
    return;
  }
  vis[fy] = 1;
  now += 1;
}

func add(u: dynamic, fa: dynamic)
{
  change(a[u], b[u]);
  {
    var p = h[u];
    while (p)
    {
      if ((v[p] == fa))
      {
        p = nxt[p];
        continue;
      }
      add(v[p], u);
      p = nxt[p];
    }
  }
}

func dfs2(u: dynamic, fa: dynamic, kp: dynamic)
{
  {
    var p = h[u];
    while (p)
    {
      if (((v[p] != fa) && (v[p] != son[u])))
      {
        dfs2(v[p], u, 0);
      }
      p = nxt[p];
    }
  }
  if (son[u])
  {
    dfs2(son[u], u, 1);
  }
  {
    var p = h[u];
    while (p)
    {
      if (((v[p] != fa) && (v[p] != son[u])))
      {
        add(v[p], u);
      }
      p = nxt[p];
    }
  }
  change(a[u], b[u]);
  ans[u] = now;
  if ((!kp))
  {
    for (var x in gar)
    {
      f[x] = x;
      vis[x] = 0;
      col[x] = 0;
    }
    gar.clear();
    now = 0;
  }
}

func main()
{
  n = readint();
  m = readint();
  {
    var i = 1;
    while ((i <= m))
    {
      f[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      addedge(readint(), readint());
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = readint();
      b[i] = readint();
      i += 1;
    }
  }
  dfs1(1, -1);
  dfs2(1, -1, 1);
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
