// Translated from solution.cpp.

var N: dynamic = (5000 + 7);

var M: dynamic = (1e4 + 7);

var inf: dynamic = 0x3f3f3f3f;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var mod: dynamic = (1e9 + 7);

var f: dynamic = cpp_array(N, N);

var a: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(N, N);

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      f[i][i] = a[i];
      dp[i][i] = a[i];
      i += 1;
    }
  }
  {
    var len: dynamic = 2;
    while ((len <= n))
    {
      {
        var i: dynamic = 1;
        while ((((i + len) - 1) <= n))
        {
          var j: dynamic = ((i + len) - 1);
          f[i][j] = (f[(i + 1)][j] ^ f[i][(j - 1)]);
          i += 1;
        }
      }
      len += 1;
    }
  }
  {
    var len: dynamic = 2;
    while ((len <= n))
    {
      {
        var i: dynamic = 1;
        while ((((i + len) - 1) <= n))
        {
          var j: dynamic = ((i + len) - 1);
          dp[i][j] = max(dp[(i + 1)][j], dp[i][(j - 1)]);
          dp[i][j] = max(dp[i][j], (f[(i + 1)][j] ^ f[i][(j - 1)]));
          i += 1;
        }
      }
      len += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    scanf("%d%d", (&l), (&r));
    printf("%d\n", dp[l][r]);
  }
  return 0;
}
