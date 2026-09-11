// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var ull: dynamic = dynamic;

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var mpr: dynamic = cpp_expression("#include");

var dingyi: dynamic = cpp_expression("#include <iostream> #include <cstdio> #include <al");

var y0: dynamic = cpp_expression("#include <io");

var y1: dynamic = cpp_expression("#include <io");

func rep(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i = x; i <= y; ++i)");
}

func per(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i = x; i >= y; --i)");
}

func repg(i: dynamic, u: dynamic) -> dynamic
{
  cpp_macro("for(int i = head[u]; i; i = e[i].nxt)");
}

func read() -> dynamic
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
    x = ((x * 10) + ((ch ^ 48)));
    ch = getchar();
  }
  return (x * f);
}

var N: dynamic = 200010;

class edge
{
  var v: dynamic = cpp_uninitialized();
  var nxt: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((N << 1));

var head: dynamic = cpp_array(N);

var cnt: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var dis: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var f: dynamic = cpp_array(20, N);

var c: dynamic = cpp_array(N);

var sta: dynamic = cpp_array(N);

var len: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

var it: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

func add(u: dynamic, v: dynamic) -> dynamic
{
  e[cpp_update(cnt, "++")].v = v;
  e[cnt].nxt = head[u];
  head[u] = cnt;
}

func dfs(u: dynamic) -> dynamic
{
  dep[u] = (dep[f[u][0]] + 1);
  rep(j, 1, 18)[u][j] = f[f[u][(j - 1)]][(j - 1)];
  repg(i, u);
  dfs(e[i].v);
}

func lca(x: dynamic, y: dynamic) -> dynamic
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

func pat(x: dynamic, y: dynamic) -> dynamic
{
  return ((dis[x] + dis[y]) - (2 * dis[lca(x, y)]));
}

func Insert(x: dynamic) -> dynamic
{
  it = st.insert(x).fi;
  var y: dynamic = (*(cpp_update(it, "--")));
  it += 1;
  it += 1;
  var z: dynamic = ( (((it == st.end()))) ? (*st.begin()) : (*it));
  ans += ((pat(y, x) + pat(x, z)) - pat(y, z));
}

func Delete(x: dynamic) -> dynamic
{
  it = st.find(x);
  var y: dynamic = (*(cpp_update(it, "--")));
  it += 1;
  it += 1;
  var z: dynamic = ( (((it == st.end()))) ? (*st.begin()) : (*it));
  ans -= ((pat(y, x) + pat(x, z)) - pat(y, z));
  it -= 1;
  st.erase(it);
}

func mian() -> dynamic
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
    var x: dynamic = read();
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

func main() -> dynamic
{
  var qwq: dynamic = 1;
  while (cpp_update(qwq, "--"))
  {
    mian();
  }
  return 0;
}
