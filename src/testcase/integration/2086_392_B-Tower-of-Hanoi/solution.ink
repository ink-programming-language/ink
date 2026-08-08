// Translated from solution.cpp.

var dp = [0];

var t = [0];

var dped = [0];

func move(n: dynamic, from_cpp: dynamic, thr: dynamic, to: dynamic)
{
  if (dped[from_cpp][to][n])
  {
    return dp[from_cpp][to][n];
  }
  dped[from_cpp][to][n] = 1;
  if ((n == 0))
  {
    return cpp_assign(dp[from_cpp][to][n], "=", 0);
  }
  return cpp_assign(dp[from_cpp][to][n], "=", min(((move((n - 1), from_cpp, to, thr) + t[from_cpp][to]) + move((n - 1), thr, from_cpp, to)), (((((move((n - 1), from_cpp, thr, to) << 1)) + t[from_cpp][thr]) + t[thr][to]) + move((n - 1), to, thr, from_cpp))));
}

func main()
{
  {
    var i = 1;
    while ((i <= (3)))
    {
      {
        var j = 1;
        while ((j <= (3)))
        {
          scanf("%I64d", (&t[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var n: dynamic;
  scanf("%d", (&n));
  printf("%I64d", move(n, 1, 2, 3));
}
