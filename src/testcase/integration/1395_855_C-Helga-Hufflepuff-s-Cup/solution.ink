// Translated from solution.cpp.

var INV2 = 500000004;

var INV6 = 166666668;

func power(a: dynamic, b: dynamic, c: dynamic)
{
  var x = 1;
  var y = a;
  while ((b > 0))
  {
    if ((b & 1))
    {
      x = (((x * y)) % c);
    }
    y = (((y * y)) % c);
    b /= 2;
  }
  return (x % c);
}

var dx = [0, -1, 0, 1];

var dy = [-1, 0, 1, 0];

var dr = [1, 1, 0, -1, -1, -1, 0, 1];

var dc = [0, 1, 1, 1, 0, -1, -1, -1];

var N = (1e5 + 5);

var M = 15;

var n: dynamic;

var m: dynamic;

var dp = cpp_array(5, M, N);

var aux = cpp_array(5, M);

var k: dynamic;

var x: dynamic;

var v = cpp_array(N);

var vis = cpp_array(N);

func dfs(node: dynamic)
{
  vis[node] = 1;
  dp[node][0][0] = (k - 1);
  dp[node][1][1] = 1;
  dp[node][0][2] = (m - k);
  for (var u in v[node])
  {
    if ((!vis[u]))
    {
      dfs(u);
      {
        var i = 0;
        while ((i < 3))
        {
          {
            var j = 0;
            while ((j <= x))
            {
              aux[j][i] = 0;
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i = x;
        while ((i >= 0))
        {
          {
            var j = 0;
            while ((j <= i))
            {
              aux[i][0] = (((aux[i][0] + (((dp[node][(i - j)][0] * (((dp[u][j][0] + dp[u][j][1]) + dp[u][j][2])))) % 1000000007))) % 1000000007);
              aux[i][1] = (((aux[i][1] + (((dp[node][(i - j)][1] * dp[u][j][0])) % 1000000007))) % 1000000007);
              aux[i][2] = (((aux[i][2] + (((dp[node][(i - j)][2] * ((dp[u][j][0] + dp[u][j][2])))) % 1000000007))) % 1000000007);
              j += 1;
            }
          }
          i -= 1;
        }
      }
      {
        var i = 0;
        while ((i < 3))
        {
          {
            var j = 0;
            while ((j <= x))
            {
              dp[node][j][i] = aux[j][i];
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  }
  return;
}

func main()
{
  scanf("%lld%lld", (&n), (&m));
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var xx: dynamic;
      var yy: dynamic;
      scanf("%d%d", (&xx), (&yy));
      v[xx].push_back(yy);
      v[yy].push_back(xx);
      i += 1;
    }
  }
  scanf("%lld%lld", (&k), (&x));
  dfs(1);
  var ans = 0;
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j <= x))
        {
          ans = (((ans + dp[1][j][i])) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
