// Translated from solution.cpp.

var int_cpp = dynamic;

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); ++i)");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func main()
{
  var n: dynamic;
  read(n);
  REP(i, n);
  read(a[i]);
  sort(begin(a), end(a));
  var dp = cpp_construct((n + 1), vector((n + 1)));
  REP(i, n)[i][n] = 1;
  var ans = 0;
  write(ans, "\n");
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    {
      var j = (i + 1);
      while ((j < n))
      {
        chmax(dp[j][i], (dp[i][n] + 1));
        j += 1;
      }
    }
    REP(j, (i + 1));
    {
      var d = (a[i] - a[j]);
      var itr = lower_bound(begin(a), end(a), (a[i] + d));
      if (((itr == end(a)) || ((*itr) != (a[i] + d))))
      {
        continue;
      }
      var idx = (itr - begin(a));
      chmax(dp[idx][i], (dp[i][j] + 1));
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      chmax(ans, dp[i][j]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
  }
