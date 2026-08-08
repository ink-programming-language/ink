// Translated from solution.cpp.

var INFL = 1152921504606846976;

var n: dynamic;

var t = cpp_array(((1 << 11)));

var c = cpp_array(((1 << 11)));

var dp = cpp_array(((1 << 11)));

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      dp[i] = INFL;
      i += 1;
    }
  }
  dp[0] = 0;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&t[i]), (&c[i]));
      {
        var j = (n - 1);
        while ((j >= 0))
        {
          var tmp: dynamic;
          if ((((j + t[i]) + 1) > n))
          {
            tmp = n;
          } else
          {
            tmp = ((j + t[i]) + 1);
          }
          dp[tmp] = cpp_cast(min(cpp_cast(dp[tmp]), (cpp_cast(dp[j]) + cpp_cast(c[i]))));
          j -= 1;
        }
      }
      i += 1;
    }
  }
  printf("%I64d\n", dp[n]);
  return 0;
}
