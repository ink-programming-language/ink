// Translated from solution.cpp.

var N: dynamic = 1e4;

var n: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array((N + 10));

var ans: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array((N + 10));

func dfs(x: dynamic, C: dynamic) -> dynamic
{
  var flag: dynamic = C;
  if ((C != c[x]))
  {
    ans += 1;
    flag = c[x];
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(g[x].size())))
    {
      var y: dynamic = g[x][i];
      dfs(y, flag);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      var fa: dynamic = cpp_uninitialized();
      read(fa);
      g[fa].push_back(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(c[i]);
      i += 1;
    }
  }
  dfs(1, 0);
  write(ans, "\n");
  return 0;
}
