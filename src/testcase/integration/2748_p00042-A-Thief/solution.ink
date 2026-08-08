// Translated from solution.cpp.

var dp = [0];

func main()
{
  var W: dynamic;
  var N: dynamic;
  var t = 1;
  while (cpp_comma((cin >> W), W))
  {
    read(N);
    {
      var i = 0;
      while ((i < N))
      {
        scanf("%d,%d", (&v[i]), (&w[i]));
        i += 1;
      }
    }
    fill((&dp[0][0]), ((&dp[0][0]) + (1001 * 1001)), 0);
    {
      var i = 0;
      while ((i < N))
      {
        {
          var j = 0;
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
    var aw = 0;
    var av = 0;
    {
      var i = 0;
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
