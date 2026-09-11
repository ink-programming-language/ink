// Translated from solution.cpp.

var maxn: dynamic = (5e3 + 7);

var maxa: dynamic = (1e5 + 7);

var dp: dynamic = cpp_array(maxn, maxn);

var num: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

func get_ans() -> dynamic
{
  memset(dp, 0, cpp_sizeof((dp)));
  var max_mod: dynamic = cpp_array(7);
  var max_num: dynamic = cpp_array(maxa);
  memset(max_mod, 0, cpp_sizeof((max_mod)));
  memset(max_num, 0, cpp_sizeof((max_num)));
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
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
        var j: dynamic = 1;
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

func main() -> dynamic
{
  while ((cin >> n))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%d", (num + i));
        i += 1;
      }
    }
    var ans: dynamic = get_ans();
    write(ans, "\n");
  }
  return 0;
}
