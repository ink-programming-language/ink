// Translated from solution.cpp.

var MaxN = 200002;

func main(argument_0: dynamic)
{
  var test: dynamic;
  var tests = 1;
  scanf("%d", (&tests));
  {
    test = 0;
    while ((test < tests))
    {
      Run_Case();
      test += 1;
    }
  }
}

var dp = cpp_array(2, MaxN);

var b = cpp_array(MaxN);

func Run_Case(argument_0: dynamic)
{
  var s: dynamic;
  read(s);
  var n = s.size();
  {
    var i = 0;
    while ((i < n))
    {
      b[i] = 0;
      dp[i][0] = 0;
      dp[i][1] = 0;
      i += 1;
    }
  }
  if ((s[0] == cpp_char("0")))
  {
    dp[0][0] = 1;
    dp[0][1] = 0;
  } else if ((s[0] == cpp_char("1")))
  {
    dp[0][0] = 0;
    dp[0][1] = 1;
  } else
  {
    dp[0][0] = 1;
    dp[0][1] = 1;
  }
  var answer = max(dp[0][0], dp[0][1]);
  b[0] = answer;
  {
    var i = 1;
    while ((i < n))
    {
      if ((s[i] == cpp_char("0")))
      {
        dp[i][1] = 0;
        dp[i][0] = (1 + dp[(i - 1)][1]);
        b[i] = max(dp[i][0], dp[i][1]);
      } else if ((s[i] == cpp_char("1")))
      {
        dp[i][0] = 0;
        dp[i][1] = (1 + dp[(i - 1)][0]);
        b[i] = max(dp[i][0], dp[i][1]);
      } else
      {
        dp[i][0] = (1 + dp[(i - 1)][1]);
        dp[i][1] = (1 + dp[(i - 1)][0]);
        b[i] = (b[(i - 1)] + 1);
      }
      answer += b[i];
      i += 1;
    }
  }
  printf("%lld\n", answer);
}
