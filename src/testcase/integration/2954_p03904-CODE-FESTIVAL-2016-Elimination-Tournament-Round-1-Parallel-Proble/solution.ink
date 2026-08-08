// Translated from solution.cpp.

var s: dynamic;

var K: dynamic;

var L: dynamic;

var N: dynamic;

var dp = cpp_array(101, (1 << 17));

func main()
{
  read(K, s);
  N = s.size();
  L = (N / ((K + 1)));
  if (((N % ((K + 1))) == 0))
  {
    var ans = s.substr(0, L);
    {
      var i = L;
      while ((i < N))
      {
        ans = max(ans, s.substr(i, L));
        i += L;
      }
    }
    write(ans, "\n");
  } else if ((K < 100))
  {
    var A = (N % ((K + 1)));
    {
      var i = 0;
      while ((i <= N))
      {
        {
          var j = 0;
          while ((j <= A))
          {
            dp[i][j] = -1;
            j += 1;
          }
        }
        i += 1;
      }
    }
    dp[0][A] = -2;
    {
      var i = 0;
      while ((i < N))
      {
        {
          var j = 0;
          while ((j <= A))
          {
            if ((dp[i][j] == -1))
            {
              j += 1;
              continue;
            }
            if (((i + L) <= N))
            {
              if ((j == A))
              {
                dp[(i + L)][j] = -2;
              } else if ((dp[(i + L)][j] == -1))
              {
                dp[(i + L)][j] = dp[i][j];
              } else
              {
                if ((s.substr(dp[(i + L)][j], (L + 1)) > s.substr(dp[i][j], (L + 1))))
                {
                  dp[(i + L)][j] = dp[i][j];
                }
              }
            }
            if (((j > 0) && (((i + L) + 1) <= N)))
            {
              var now: dynamic;
              if ((j == A))
              {
                now = i;
              } else
              {
                if ((s.substr(i, (L + 1)) < s.substr(dp[i][j], (L + 1))))
                {
                  now = dp[i][j];
                } else
                {
                  now = i;
                }
              }
              if ((dp[((i + L) + 1)][(j - 1)] == -1))
              {
                dp[((i + L) + 1)][(j - 1)] = now;
              } else
              {
                if ((s.substr(dp[((i + L) + 1)][(j - 1)], (L + 1)) > s.substr(now, (L + 1))))
                {
                  dp[((i + L) + 1)][(j - 1)] = now;
                }
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(s.substr(dp[N][0], (L + 1)), "\n");
  } else
  {
    var ans = cpp_construct((L + 1), cpp_char("9"));
    {
      var i = 0;
      while ((i <= L))
      {
        var l = 0;
        var r = 9;
        while (((r - l) > 1))
        {
          var m = ((l + r) >> 1);
          ans[i] = (m + cpp_char("0"));
          var flag = true;
          var id = 0;
          {
            var j = 0;
            while ((j <= K))
            {
              if ((s.substr(id, (L + 1)) <= ans))
              {
                id += (L + 1);
              } else
              {
                id += L;
              }
              if ((id >= N))
              {
                break;
              }
              j += 1;
            }
          }
          if ((id >= N))
          {
            r = m;
          } else
          {
            l = m;
          }
        }
        ans[i] = (r + cpp_char("0"));
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
