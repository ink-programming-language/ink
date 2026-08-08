// Translated from solution.cpp.

var s = cpp_array(800);

var dp = cpp_array(3, 3, 800, 800);

var match_cpp = cpp_array(800);

func find_match(n: dynamic)
{
  var stac = cpp_array(800);
  var top = 0;
  var i: dynamic;
  {
    i = 0;
    while ((i <= n))
    {
      if ((s[i] == cpp_char("(")))
      {
        stac[cpp_update(top, "++")] = i;
      } else
      {
        match_cpp[i] = stac[top];
        match_cpp[stac[top]] = i;
        top -= 1;
      }
      i += 1;
    }
  }
  return;
}

func dfs(l: dynamic, r: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var h: dynamic;
  if (((l + 1) == r))
  {
    dp[l][r][1][0] = 1;
    dp[l][r][2][0] = 1;
    dp[l][r][0][1] = 1;
    dp[l][r][0][2] = 1;
    return;
  }
  if ((match_cpp[l] == r))
  {
    dfs((l + 1), (r - 1));
    {
      i = 0;
      while ((i < 3))
      {
        {
          j = 0;
          while ((j < 3))
          {
            if ((j != 1))
            {
              dp[l][r][0][1] = (((dp[l][r][0][1] + dp[(l + 1)][(r - 1)][i][j])) % 1000000007);
            }
            if ((j != 2))
            {
              dp[l][r][0][2] = (((dp[l][r][0][2] + dp[(l + 1)][(r - 1)][i][j])) % 1000000007);
            }
            if ((i != 1))
            {
              dp[l][r][1][0] = (((dp[l][r][1][0] + dp[(l + 1)][(r - 1)][i][j])) % 1000000007);
            }
            if ((i != 2))
            {
              dp[l][r][2][0] = (((dp[l][r][2][0] + dp[(l + 1)][(r - 1)][i][j])) % 1000000007);
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    return;
  } else
  {
    var q: dynamic;
    q = match_cpp[l];
    dfs(l, q);
    dfs((q + 1), r);
    {
      i = 0;
      while ((i < 3))
      {
        {
          j = 0;
          while ((j < 3))
          {
            {
              k = 0;
              while ((k < 3))
              {
                {
                  h = 0;
                  while ((h < 3))
                  {
                    if (((!(((k == 1) && (h == 1)))) && (!(((k == 2) && (h == 2))))))
                    {
                      dp[l][r][i][j] = (((dp[l][r][i][j] + (((dp[l][q][i][k] * dp[(q + 1)][r][h][j])) % 1000000007))) % 1000000007);
                    }
                    h += 1;
                  }
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    return;
  }
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var len: dynamic;
  var k: dynamic;
  while ((scanf("%s", s) != EOF))
  {
    len = strlen(s);
    memset(dp, 0, cpp_sizeof((dp)));
    find_match((len - 1));
    dfs(0, (len - 1));
    k = 0;
    {
      i = 0;
      while ((i < 3))
      {
        {
          j = 0;
          while ((j < 3))
          {
            k = (((k + dp[0][(len - 1)][i][j])) % 1000000007);
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%I64d\n", k);
  }
  return 0;
}
