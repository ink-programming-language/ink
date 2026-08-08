// Translated from solution.cpp.

var N = (5000 + 7);

var M = (1e4 + 7);

var inf = 0x3f3f3f3f;

var INF = 0x3f3f3f3f3f3f3f3f;

var mod = (1e9 + 7);

var f = cpp_array(N, N);

var a = cpp_array(N);

var n: dynamic;

var dp = cpp_array(N, N);

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      f[i][i] = a[i];
      dp[i][i] = a[i];
      i += 1;
    }
  }
  {
    var len = 2;
    while ((len <= n))
    {
      {
        var i = 1;
        while ((((i + len) - 1) <= n))
        {
          var j = ((i + len) - 1);
          f[i][j] = (f[(i + 1)][j] ^ f[i][(j - 1)]);
          i += 1;
        }
      }
      len += 1;
    }
  }
  {
    var len = 2;
    while ((len <= n))
    {
      {
        var i = 1;
        while ((((i + len) - 1) <= n))
        {
          var j = ((i + len) - 1);
          dp[i][j] = max(dp[(i + 1)][j], dp[i][(j - 1)]);
          dp[i][j] = max(dp[i][j], (f[(i + 1)][j] ^ f[i][(j - 1)]));
          i += 1;
        }
      }
      len += 1;
    }
  }
  var q: dynamic;
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var l: dynamic;
    var r: dynamic;
    scanf("%d%d", (&l), (&r));
    printf("%d\n", dp[l][r]);
  }
  return 0;
}
