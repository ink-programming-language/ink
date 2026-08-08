// Translated from solution.cpp.

var N = 1e4;

var n: dynamic;

var c = cpp_array((N + 10));

var ans: dynamic;

var g = cpp_array((N + 10));

func dfs(x: dynamic, C: dynamic)
{
  var flag = C;
  if ((C != c[x]))
  {
    ans += 1;
    flag = c[x];
  }
  {
    var i = 0;
    while ((i < cpp_cast(g[x].size())))
    {
      var y = g[x][i];
      dfs(y, flag);
      i += 1;
    }
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i = 2;
    while ((i <= n))
    {
      var fa: dynamic;
      read(fa);
      g[fa].push_back(i);
      i += 1;
    }
  }
  {
    var i = 1;
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
