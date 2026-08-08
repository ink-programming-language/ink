// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var now = 0;
  var nxt = 1;
  var dp = cpp_array(10, 2);
  memset(dp, -1, cpp_sizeof((dp)));
  dp[0][0] = 0;
  while (cpp_update(n, "--"))
  {
    var k: dynamic;
    read(k);
    var v = cpp_array(4);
    {
      var i = 0;
      while ((i < k))
      {
        var c: dynamic;
        var d: dynamic;
        read(c, d);
        v[c].push_back(d);
        i += 1;
      }
    }
    sort(v[1].begin(), v[1].end(), greater());
    sort(v[2].begin(), v[2].end(), greater());
    sort(v[3].begin(), v[3].end(), greater());
    memcpy(dp[nxt], dp[now], cpp_sizeof((dp[now])));
    {
      var i = 0;
      while ((i < 10))
      {
        if ((dp[now][i] == -1))
        {
          i += 1;
          continue;
        }
        {
          var j = 1;
          while ((j <= 3))
          {
            if ((cpp_cast(v[j].size()) != 0))
            {
              dp[nxt][(((i + 1)) % 10)] = max(dp[nxt][(((i + 1)) % 10)], (dp[now][i] + (v[j][0] * ((1 + ((((i + 1)) == 10)))))));
            }
            j += 1;
          }
        }
        if ((cpp_cast(v[1].size()) >= 3))
        {
          dp[nxt][(((i + 3)) % 10)] = max(dp[nxt][(((i + 3)) % 10)], (((dp[now][i] + (v[1][0] * ((1 + ((((i + 3)) >= 10)))))) + v[1][1]) + v[1][2]));
        }
        if ((cpp_cast(v[1].size()) >= 2))
        {
          dp[nxt][(((i + 2)) % 10)] = max(dp[nxt][(((i + 2)) % 10)], ((dp[now][i] + (v[1][0] * ((1 + ((((i + 2)) >= 10)))))) + v[1][1]));
        }
        if (((cpp_cast(v[2].size()) != 0) && (cpp_cast(v[1].size()) != 0)))
        {
          dp[nxt][(((i + 2)) % 10)] = max(dp[nxt][(((i + 2)) % 10)], (((dp[now][i] + v[2][0]) + v[1][0]) + (max(v[1][0], v[2][0]) * ((((i + 2)) >= 10)))));
        }
        i += 1;
      }
    }
    swap(now, nxt);
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < 10))
    {
      ans = max(ans, dp[now][i]);
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}
