// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); ++i)");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  REP(i, n);
  read(a[i]);
  sort(begin(a), end(a));
  var dp: dynamic = cpp_construct((n + 1), vector((n + 1)));
  REP(i, n)[i][n] = 1;
  var ans: dynamic = 0;
  write(ans, "\n");
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    {
      var j: dynamic = (i + 1);
      while ((j < n))
      {
        chmax(dp[j][i], (dp[i][n] + 1));
        j += 1;
      }
    }
    REP(j, (i + 1));
    {
      var d: dynamic = (a[i] - a[j]);
      var itr: dynamic = lower_bound(begin(a), end(a), (a[i] + d));
      if (((itr == end(a)) || ((*itr) != (a[i] + d))))
      {
        continue;
      }
      var idx: dynamic = (itr - begin(a));
      chmax(dp[idx][i], (dp[i][j] + 1));
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      chmax(ans, dp[i][j]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
  }
