// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var LINF = 0x3f3f3f3f3f3f3f3f;

var EPS = 1e-8;

var MOD = 998244353;

var dy = [1, 0, -1, 0];

var dx = [0, -1, 0, 1];

var edge = cpp_array(200000);

var fact = cpp_array(200001);

func init()
{
  fact[0] = cpp_assign(fact[1], "=", 1);
  {
    var i = (2);
    while ((i < (200001)))
    {
      fact[i] = (((fact[(i - 1)] * i)) % MOD);
      i += 1;
    }
  }
}

var ans = 1;

func dfs(par: dynamic, ver: dynamic)
{
  (cpp_assign(ans, "*=", fact[edge[ver].size()])) %= MOD;
  for (var e in edge[ver])
  {
    if ((e != par))
    {
      dfs(ver, e);
    }
  }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  init();
  var n: dynamic;
  read(n);
  {
    var i = (0);
    while ((i < ((n - 1))))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      u -= 1;
      v -= 1;
      edge[u].emplace_back(v);
      edge[v].emplace_back(u);
      i += 1;
    }
  }
  dfs(-1, 0);
  (cpp_assign(ans, "*=", n)) %= MOD;
  write(ans, cpp_char("\n"));
  return 0;
}
