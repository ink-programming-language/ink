// Translated from solution.cpp.

func solve() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  s = ("#" + s);
  t = ("#" + t);
  var n: dynamic = (s.length() - 1);
  var m: dynamic = (t.length() - 1);
  var go: dynamic = cpp_construct((m + 2), vector(26));
  var pi: dynamic = cpp_construct((m + 2));
  var dp: dynamic = cpp_construct((n + 2), vector((m + 2)));
  if ((t.length() > s.length()))
  {
    write(0, "\n");
    return;
  }
  var k: dynamic = 0;
  pi[1] = 0;
  {
    var i: dynamic = 2;
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
    var i: dynamic = 0;
    while ((i <= m))
    {
      {
        var ch: dynamic = 0;
        while ((ch < 26))
        {
          go[i][ch] = 0;
          ch += 1;
        }
      }
      {
        var k: dynamic = i;
        while (true)
        {
          if (((k + 1) < t.length()))
          {
            var ch: dynamic = (t[(k + 1)] - cpp_char("a"));
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
    var i: dynamic = 1;
    while ((i <= (n + 1)))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= (n + 1)))
    {
      {
        var k: dynamic = 0;
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
                var ch: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= m))
    {
      ans = max(ans, dp[(n + 1)][i]);
      i += 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  var tests: dynamic = 1;
  {
    while (tests)
    {
      solve();
      tests -= 1;
    }
  }
}
