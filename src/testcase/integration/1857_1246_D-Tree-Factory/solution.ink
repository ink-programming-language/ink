// Translated from solution.cpp.

var INT_INF: dynamic = cpp_cast((2e9));

var LL_INF: dynamic = (ll)(2e18);

var NIL: dynamic = -1;

func randint(a: dynamic, b: dynamic) -> dynamic
{
  var w: dynamic = (((g() << 31)) ^ g());
  return (a + (w % (((b - a) + 1))));
}

func fast_io() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
}

func __cpp_top_level_1() -> dynamic
{
}

func sign(x: dynamic) -> dynamic
{
  return (T((x > 0)) - T((x < 0)));
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  write("[", p.first, ";", p.second, "]");
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  for (var el: dynamic in v)
  {
    write(el, " ");
  }
  return os;
}

func fetch() -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  read(ret);
  return ret;
}

func fetch_vec(sz: dynamic) -> dynamic
{
  for (var elem: dynamic in ret)
  {
    read(elem);
  }
  return ret;
}

var MAXN: dynamic = (cpp_cast(1e5) + 77);

var g: dynamic = cpp_array(MAXN);

var answ: dynamic = cpp_uninitialized();

var ord: dynamic = cpp_uninitialized();

var par: dynamic = cpp_array(MAXN);

var q: dynamic = cpp_array(MAXN);

var depth: dynamic = cpp_array(MAXN);

var max_depth: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

func prepare(v: dynamic, p: dynamic) -> dynamic
{
  depth[v] = ( ((p == NIL)) ? 0 : (depth[p] + 1));
  max_depth[v] = depth[v];
  for (var u: dynamic in g[v])
  {
    if ((u != p))
    {
      prepare(u, v);
      max_depth[v] = max(max_depth[v], max_depth[u]);
    }
  }
}

func dfs(v: dynamic, p: dynamic) -> dynamic
{
  ord.push_back(v);
  sort(g[v].begin(), g[v].end(), __cpp_lambda_2);
  for (var u: dynamic in g[v])
  {
    if ((u != p))
    {
      dfs(u, v);
    }
  }
}

func solve() -> dynamic
{
  read(n);
  par[0] = NIL;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var p: dynamic = fetch();
      g[i].push_back(p);
      g[p].push_back(i);
      par[i] = p;
      i += 1;
    }
  }
  prepare(0, NIL);
  dfs(0, NIL);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      q[ord[i]] = ord[(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      while ((q[ord[i]] != par[ord[i]]))
      {
        answ.push_back(ord[i]);
        q[ord[i]] = par[q[ord[i]]];
      }
      i += 1;
    }
  }
  write(ord, cpp_char("\n"));
  write((cpp_cast((answ).size())), cpp_char("\n"));
  write(answ, cpp_char("\n"));
}

func main() -> dynamic
{
  fast_io();
  solve();
  return 0;
}

func __cpp_lambda_2(v: dynamic, u: dynamic) -> dynamic
{
  return (max_depth[v] < max_depth[u]);
}
