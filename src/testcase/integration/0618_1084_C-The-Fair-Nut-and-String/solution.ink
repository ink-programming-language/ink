// Translated from solution.cpp.

var N = (1e5 + 4);

var mod = (1e9 + 7);

var dp = cpp_array(N);

var a = cpp_array(N);

func main()
{
  scanf("%s", (a + 1));
  var n = strlen((a + 1));
  var sum = 0;
  var ans = 0;
  var flag = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i] == cpp_char("a")))
      {
        dp[i] = (sum + 1);
        dp[i] %= mod;
        ans += dp[i];
        ans %= mod;
      } else if ((a[i] == cpp_char("b")))
      {
        sum += ans;
        sum %= mod;
        ans = 0;
        while (((a[i] != cpp_char("a")) && (i <= n)))
        {
          i += 1;
        }
        i -= 1;
      }
      i += 1;
    }
  }
  sum = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      sum = (sum + dp[i]);
      sum %= mod;
      i += 1;
    }
  }
  write(sum, "\n");
}
