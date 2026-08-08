// Translated from solution.cpp.

func solve()
{
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  s = ("#" + s);
  t = ("#" + t);
  var n = (s.length() - 1);
  var m = (t.length() - 1);
  var go = cpp_construct((m + 2), vector(26));
  var pi = cpp_construct((m + 2));
  var dp = cpp_construct((n + 2), vector((m + 2)));
  if ((t.length() > s.length()))
  {
    write(0, "\n");
    return;
  }
  var k = 0;
  pi[1] = 0;
  {
    var i = 2;
    while ((i <= m))
    {
      while (((k > 0) && (t[(k + 1)] != t[i])))
      {
        k = pi[k];
      }
      if ((t[(k + 1)] == t[i]))
      {
        k += 1;
      }
      pi[i] = k;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= m))
    {
      {
        var ch = 0;
        while ((ch < 26))
        {
          go[i][ch] = 0;
          ch += 1;
        }
      }
      {
        var k = i;
        while (true)
        {
          if (((k + 1) < t.length()))
          {
            var ch = (t[(k + 1)] - cpp_char("a"));
            if ((go[i][ch] == 0))
            {
              go[i][ch] = (k + 1);
            }
          }
          if ((k == 0))
          {
            break;
          }
          k = pi[k];
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (n + 1)))
    {
      {
        var j = 0;
        while ((j <= m))
        {
          dp[i][j] = -1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[1][0] = 0;
  {
    var i = 1;
    while ((i <= (n + 1)))
    {
      {
        var k = 0;
        while ((k <= m))
        {
          if ((dp[i][k] == -1))
          {
            k += 1;
            continue;
          }
          dp[i][k] += ((k == m));
          if ((i <= n))
          {
            if ((s[i] != cpp_char("?")))
            {
              dp[(i + 1)][go[k][(s[i] - cpp_char("a"))]] = max(dp[(i + 1)][go[k][(s[i] - cpp_char("a"))]], dp[i][k]);
            } else
            {
              {
                var ch = 0;
                while ((ch < 26))
                {
                  dp[(i + 1)][go[k][ch]] = max(dp[(i + 1)][go[k][ch]], dp[i][k]);
                  ch += 1;
                }
              }
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i <= m))
    {
      ans = max(ans, dp[(n + 1)][i]);
      i += 1;
    }
  }
  write(ans, "\n");
}

func main()
{
  var tests = 1;
  {
    while (tests)
    {
      solve();
      tests -= 1;
    }
  }
}
