// Translated from solution.cpp.

var MAXN = (1e3 + 5);

var MOD = (1e9 + 7);

var sum = cpp_array(30, MAXN);

var M = cpp_array(MAXN, MAXN);

var dp = cpp_array(MAXN);

var maxx = cpp_array(30);

var dp2 = cpp_array(MAXN);

var a: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var ans = 0;
  read(n);
  read(a);
  {
    i = 1;
    while ((i <= 26))
    {
      read(maxx[i]);
      i += 1;
    }
  }
  var flag = 1;
  var len = 0;
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = i;
        while ((j <= n))
        {
          flag = 1;
          len = ((j - i) + 1);
          {
            k = i;
            while ((k <= j))
            {
              if ((len > maxx[(((a[(k - 1)] - cpp_char("a")) + 1))]))
              {
                flag = 0;
                break;
              }
              k += 1;
            }
          }
          M[i][j] = flag;
          if (flag)
          {
            ans = max(len, ans);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0] = 1;
  memset(dp2, 0x3f, cpp_sizeof(dp2));
  dp2[0] = 0;
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = i;
        while ((j >= 1))
        {
          if (M[j][i])
          {
            dp[i] = (((dp[i] + dp[(j - 1)])) % MOD);
            dp2[i] = min(dp2[i], (dp2[(j - 1)] + 1));
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  write(dp[n], "\n", ans, "\n", dp2[n], "\n");
  return 0;
}
