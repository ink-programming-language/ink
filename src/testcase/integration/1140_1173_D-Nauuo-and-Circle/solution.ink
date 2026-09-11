// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var LINF: dynamic = 0x3f3f3f3f3f3f3f3f;

var EPS: dynamic = 1e-8;

var MOD: dynamic = 998244353;

var dy: dynamic = [1, 0, -1, 0];

var dx: dynamic = [0, -1, 0, 1];

var edge: dynamic = cpp_array(200000);

var fact: dynamic = cpp_array(200001);

func init() -> dynamic
{
  fact[0] = cpp_assign(fact[1], "=", 1);
  {
    var i: dynamic = (2);
    while ((i < (200001)))
    {
      fact[i] = (((fact[(i - 1)] * i)) % MOD);
      i += 1;
    }
  }
}

var ans: dynamic = 1;

func dfs(par: dynamic, ver: dynamic) -> dynamic
{
  (cpp_assign(ans, "*=", fact[edge[ver].size()])) %= MOD;
  for (var e: dynamic in edge[ver])
  {
    if ((e != par))
    {
      dfs(ver, e);
    }
  }
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  init();
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = (0);
    while ((i < ((n - 1))))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
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
