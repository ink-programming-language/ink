// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  {
    while ((!isdigit(ch)))
    {
      if ((ch == cpp_char("-")))
      {
        f = -1;
      }
      ch = getchar();
    }
  }
  {
    while (isdigit(ch))
    {
      x = (((x * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  return (x * f);
}

var N = 2005;

var mod = (1e9 + 7);

var dp = cpp_array(N, N);

var sum = cpp_array(N);

var s = cpp_array(N);

func add(x: dynamic, y: dynamic)
{
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
}

func main()
{
  var n = read();
  var k = read();
  scanf("%s", (s + 1));
  sum[0] = cpp_assign(dp[0][0], "=", 1);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= k))
        {
          {
            var l = (i - 1);
            while (((l >= 0) && ((((i - l)) * (((n - i) + 1))) <= j)))
            {
              add(dp[i][j], dp[l][(j - (((i - l)) * (((n - i) + 1))))]);
              l -= 1;
            }
          }
          dp[i][j] = (((1 * ((cpp_char("z") - s[i]))) * dp[i][j]) % mod);
          add(dp[i][j], (((1 * sum[j]) * ((s[i] - cpp_char("a")))) % mod));
          add(sum[j], dp[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i <= n))
    {
      add(ans, dp[i][k]);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
