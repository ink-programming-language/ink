// Translated from solution.cpp.

var dp: dynamic = [0];

func main() -> dynamic
{
  var W: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  var t: dynamic = 1;
  while (cpp_comma((cin >> W), W))
  {
    read(N);
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        scanf("%d,%d", (&v[i]), (&w[i]));
        i += 1;
      }
    }
    fill((&dp[0][0]), ((&dp[0][0]) + (1001 * 1001)), 0);
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        {
          var j: dynamic = 0;
          while ((j <= W))
          {
            if (((w[i] + j) <= W))
            {
              dp[(i + 1)][(w[i] + j)] = max(dp[(i + 1)][(w[i] + j)], (dp[i][j] + v[i]));
            }
            dp[(i + 1)][j] = max(dp[(i + 1)][j], dp[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var aw: dynamic = 0;
    var av: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i <= W))
      {
        if ((dp[N][i] > av))
        {
          av = dp[N][i];
          aw = i;
        }
        i += 1;
      }
    }
    write("Case ", t, ":", "\n");
    write(av, "\n");
    write(aw, "\n");
    t += 1;
  }
}
