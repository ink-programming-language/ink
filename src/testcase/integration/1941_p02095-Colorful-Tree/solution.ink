// Translated from solution.cpp.

var pb: dynamic = cpp_expression("#include<");

var mp: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func chkmax(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x < y)) ? cpp_comma(cpp_assign(x, "=", y), true) : false;
}

func chkmin(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x > y)) ? cpp_comma(cpp_assign(x, "=", y), true) : false;
}

func readint() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var now: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(200005);

var nxt: dynamic = cpp_array(200005);

var h: dynamic = cpp_array(100005);

var siz: dynamic = cpp_array(100005);

var f: dynamic = cpp_array(200005);

var son: dynamic = cpp_array(100005);

var col: dynamic = cpp_array(200005);

var a: dynamic = cpp_array(100005);

var b: dynamic = cpp_array(100005);

var ans: dynamic = cpp_array(100005);

var vis: dynamic = cpp_array(200005);

var gar: dynamic = cpp_uninitialized();

func getf(x: dynamic) -> dynamic
{
  return  ((x == f[x])) ? x : cpp_assign(f[x], "=", getf(f[x]));
}

func addedge(x: dynamic, y: dynamic) -> dynamic
{
  v[cpp_update(tot, "++")] = y;
  nxt[tot] = h[x];
  h[x] = tot;
  v[cpp_update(tot, "++")] = x;
  nxt[tot] = h[y];
  h[y] = tot;
}

func dfs1(u: dynamic, fa: dynamic) -> dynamic
{
  siz[u] = 1;
  {
    var p: dynamic = h[u];
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

func change(x: dynamic, y: dynamic) -> dynamic
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
  var fx: dynamic = getf(x);
  var fy: dynamic = getf(y);
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

func add(u: dynamic, fa: dynamic) -> dynamic
{
  change(a[u], b[u]);
  {
    var p: dynamic = h[u];
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

func dfs2(u: dynamic, fa: dynamic, kp: dynamic) -> dynamic
{
  {
    var p: dynamic = h[u];
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
    var p: dynamic = h[u];
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
    for (var x: dynamic in gar)
    {
      f[x] = x;
      vis[x] = 0;
      col[x] = 0;
    }
    gar.clear();
    now = 0;
  }
}

func main() -> dynamic
{
  n = readint();
  m = readint();
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      f[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      addedge(readint(), readint());
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
