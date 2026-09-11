// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000001);

var f: dynamic = cpp_array(1000001);

var g: dynamic = cpp_array(1000001);

var sz: dynamic = cpp_array(1000001);

var fa: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(1000001);

var ch: dynamic = cpp_uninitialized();

func dfs(x: dynamic, pre: dynamic) -> dynamic
{
  f[x] = (f[pre] + 1);
  g[x] = min(f[x], (g[pre] + sz[pre]));
  if (sz[x])
  {
    f[x] = min(f[x], (g[x] + 1));
  }
  sort(d[x].begin(), d[x].end());
  for (var nxt: dynamic in d[x])
  {
    dfs(nxt.second, x);
    sz[x] += sz[nxt.second];
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d %c", (&fa), (&ch));
      d[fa].push_back(make_pair(ch, i));
      i += 1;
    }
  }
  scanf("%d", (&k));
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= k))
    {
      printf("%d%c", f[a[i]],  ((i < k)) ? cpp_char(" ") : cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
