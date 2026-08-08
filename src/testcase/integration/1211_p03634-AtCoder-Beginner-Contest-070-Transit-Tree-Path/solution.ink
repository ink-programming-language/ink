// Translated from solution.cpp.

func rep(i: dynamic, x: dynamic, n: dynamic)
{
  cpp_macro("for (int i = x; i < n; i++)");
}

var INF = (1e9 + 7);

var tree = cpp_construct(100001);

var cost = cpp_construct(100001);

func dfs(c: dynamic, p: dynamic, v: dynamic)
{
  cost[c] = v;
  for (var tmp in tree[c])
  {
    if ((tmp.first != p))
    {
      dfs(tmp.first, c, (v + tmp.second));
    }
  }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  rep(i, 0, (n - 1));
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    read(a, b, c);
    tree[a].push_back(make_pair(b, c));
    tree[b].push_back(make_pair(a, c));
  }
  var q: dynamic;
  var k: dynamic;
  read(q, k);
  dfs(k, -1, 0);
  rep(i, 0, q);
  {
    var x: dynamic;
    var y: dynamic;
    read(x, y);
    write((cost[x] + cost[y]), "\n");
  }
  return 0;
}
