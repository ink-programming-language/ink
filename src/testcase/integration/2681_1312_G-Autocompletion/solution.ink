// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var a = cpp_array(1000001);

var f = cpp_array(1000001);

var g = cpp_array(1000001);

var sz = cpp_array(1000001);

var fa: dynamic;

var d = cpp_array(1000001);

var ch: dynamic;

func dfs(x: dynamic, pre: dynamic)
{
  f[x] = (f[pre] + 1);
  g[x] = min(f[x], (g[pre] + sz[pre]));
  if (sz[x])
  {
    f[x] = min(f[x], (g[x] + 1));
  }
  sort(d[x].begin(), d[x].end());
  for (var nxt in d[x])
  {
    dfs(nxt.second, x);
    sz[x] += sz[nxt.second];
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d %c", (&fa), (&ch));
      d[fa].push_back(make_pair(ch, i));
      i += 1;
    }
  }
  scanf("%d", (&k));
  {
    var i = 1;
    while ((i <= k))
    {
      scanf("%d", (&a[i]));
      sz[a[i]] = 1;
      i += 1;
    }
  }
  f[0] = -1;
  dfs(0, 0);
  {
    var i = 1;
    while ((i <= k))
    {
      printf("%d%c", f[a[i]], if ((i < k)) cpp_char(" ") else cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
