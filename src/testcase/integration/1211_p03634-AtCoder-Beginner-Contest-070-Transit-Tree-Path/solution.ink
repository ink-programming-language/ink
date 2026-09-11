// Translated from solution.cpp.

func rep(i: dynamic, x: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = x; i < n; i++)");
}

var INF: dynamic = (1e9 + 7);

var tree: dynamic = cpp_construct(100001);

var cost: dynamic = cpp_construct(100001);

func dfs(c: dynamic, p: dynamic, v: dynamic) -> dynamic
{
  cost[c] = v;
  for (var tmp: dynamic in tree[c])
  {
    if ((tmp.first != p))
    {
      dfs(tmp.first, c, (v + tmp.second));
    }
  }
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  rep(i, 0, (n - 1));
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(a, b, c);
    tree[a].push_back(make_pair(b, c));
    tree[b].push_back(make_pair(a, c));
  }
  var q: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(q, k);
  dfs(k, -1, 0);
  rep(i, 0, q);
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(x, y);
    write((cost[x] + cost[y]), "\n");
  }
  return 0;
}
