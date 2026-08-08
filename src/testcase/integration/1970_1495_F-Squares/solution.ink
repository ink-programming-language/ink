// Translated from solution.cpp.

var int_cpp = dynamic;

var ull = dynamic;

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var mpr = cpp_expression("#include");

var dingyi = cpp_expression("#include <iostream> #include <cstdio> #include <al");

var y0 = cpp_expression("#include <io");

var y1 = cpp_expression("#include <io");

func rep(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i = x; i <= y; ++i)");
}

func per(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i = x; i >= y; --i)");
}

func repg(i: dynamic, u: dynamic)
{
  cpp_macro("for(int i = head[u]; i; i = e[i].nxt)");
}

func read()
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
    x = ((x * 10) + ((ch ^ 48)));
    ch = getchar();
  }
  return (x * f);
}

var N = 200010;

class edge
{
  var v: dynamic;
  var nxt: dynamic;
}

var e = cpp_array((N << 1));

var head = cpp_array(N);

var cnt: dynamic;

var n: dynamic;

var m: dynamic;

var p = cpp_array(N);

var a = cpp_array(N);

var b = cpp_array(N);

var dis = cpp_array(N);

var dep = cpp_array(N);

var f = cpp_array(20, N);

var c = cpp_array(N);

var sta = cpp_array(N);

var len: dynamic;

var num = cpp_array(N);

var ans: dynamic;

var st: dynamic;

var it: dynamic;

var vis = cpp_array(N);

func add(u: dynamic, v: dynamic)
{
  e[cpp_update(cnt, "++")].v = v;
  e[cnt].nxt = head[u];
  head[u] = cnt;
}

func dfs(u: dynamic)
{
  dep[u] = (dep[f[u][0]] + 1);
  rep(j, 1, 18)[u][j] = f[f[u][(j - 1)]][(j - 1)];
  repg(i, u);
  dfs(e[i].v);
}

func lca(x: dynamic, y: dynamic)
{
  if ((dep[x] < dep[y]))
  {
    swap(x, y);
  }
  per(i, 18, 0);
  if ((dep[f[x][i]] >= dep[y]))
  {
    x = f[x][i];
  }
  if ((x == y))
  {
    return x;
  }
  per(i, 18, 0);
  if ((f[x][i] != f[y][i]))
  {
    x = f[x][i];
    y = f[y][i];
  }
  return f[x][0];
}

func pat(x: dynamic, y: dynamic)
{
  return ((dis[x] + dis[y]) - (2 * dis[lca(x, y)]));
}

func Insert(x: dynamic)
{
  it = st.insert(x).fi;
  var y = (*(cpp_update(it, "--")));
  it += 1;
  it += 1;
  var z = (if (((it == st.end()))) (*st.begin()) else (*it));
  ans += ((pat(y, x) + pat(x, z)) - pat(y, z));
}

func Delete(x: dynamic)
{
  it = st.find(x);
  var y = (*(cpp_update(it, "--")));
  it += 1;
  it += 1;
  var z = (if (((it == st.end()))) (*st.begin()) else (*it));
  ans -= ((pat(y, x) + pat(x, z)) - pat(y, z));
  it -= 1;
  st.erase(it);
}

func mian()
{
  n = read();
  m = read();
  rep(i, 1, n);
  {
    p[i] = read();
    while (((p[sta[len]] < p[i]) && len))
    {
      len -= 1;
    }
    f[i][0] = sta[len];
    add(sta[len], i);
    sta[cpp_update(len, "++")] = i;
  }
  dfs(0);
  rep(i, 1, n)[i] = read();
  c[i] += a[i];
  rep(i, 1, n)[i] = read();
  c[i] -= b[i];
  c[f[i][0]] += b[i];
  per(i, n, 1)[i] += c[i];
  dis[f[i][0]] += min(dis[i], 0);
  dis[i] -= min(0, dis[i]);
  dis[0] += c[0];
  rep(i, 1, n)[i] += dis[f[i][0]];
  num[0] = 1;
  st.insert(0);
  while (cpp_update(m, "--"))
  {
    var x = read();
    if (vis[x])
    {
      vis[x] = 0;
      if ((!(cpp_update(num[f[x][0]], "--"))))
      {
        Delete(f[x][0]);
      }
    } else
    {
      vis[x] = 1;
      if ((!(cpp_update(num[f[x][0]], "++"))))
      {
        Insert(f[x][0]);
      }
    }
    printf("%lld\n", ((ans / 2) + dis[0]));
  }
}

func main()
{
  var qwq = 1;
  while (cpp_update(qwq, "--"))
  {
    mian();
  }
  return 0;
}
