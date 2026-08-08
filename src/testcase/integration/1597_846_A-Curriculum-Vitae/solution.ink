// Translated from solution.cpp.

var a = cpp_array(105);

var dp = cpp_array(105);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var cnt = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      dp[i] = (dp[(i - 1)] + ((a[i] == 0)));
      cnt += a[i];
      i += 1;
    }
  }
  var res = cnt;
  var one = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      one += a[i];
      res = max(res, (dp[i] + ((cnt - one))));
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
