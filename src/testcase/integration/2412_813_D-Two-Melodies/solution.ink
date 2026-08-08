// Translated from solution.cpp.

var maxn = (5e3 + 7);

var maxa = (1e5 + 7);

var dp = cpp_array(maxn, maxn);

var num = cpp_array(maxn);

var n: dynamic;

func get_ans()
{
  memset(dp, 0, cpp_sizeof((dp)));
  var max_mod = cpp_array(7);
  var max_num = cpp_array(maxa);
  memset(max_mod, 0, cpp_sizeof((max_mod)));
  memset(max_num, 0, cpp_sizeof((max_num)));
  var ans = 0;
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          if ((j < i))
          {
            dp[i][j] = dp[j][i];
            max_mod[(num[j] % 7)] = max(max_mod[(num[j] % 7)], dp[i][j]);
            if ((j != 0))
            {
              max_num[num[j]] = max(max_num[num[j]], dp[i][j]);
            }
          } else
          {
            dp[i][j] = (dp[i][0] + 1);
            dp[i][j] = max((max_mod[(num[j] % 7)] + 1), dp[i][j]);
            dp[i][j] = max((max_num[(num[j] + 1)] + 1), dp[i][j]);
            dp[i][j] = max((max_num[(num[j] - 1)] + 1), dp[i][j]);
            max_mod[(num[j] % 7)] = max(max_mod[(num[j] % 7)], dp[i][j]);
            max_num[num[j]] = max(max_num[num[j]], dp[i][j]);
          }
          if (((i != 0) && (j != 0)))
          {
            ans = max(ans, dp[i][j]);
          }
          j += 1;
        }
      }
      memset(max_mod, 0, cpp_sizeof((max_mod)));
      {
        var j = 1;
        while ((j <= n))
        {
          max_num[num[j]] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func main()
{
  while ((cin >> n))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%d", (num + i));
        i += 1;
      }
    }
    var ans = get_ans();
    write(ans, "\n");
  }
  return 0;
}
