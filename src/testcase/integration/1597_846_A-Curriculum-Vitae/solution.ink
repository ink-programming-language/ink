// Translated from solution.cpp.

var a: dynamic = cpp_array(105);

var dp: dynamic = cpp_array(105);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dp[i] = (dp[(i - 1)] + ((a[i] == 0)));
      cnt += a[i];
      i += 1;
    }
  }
  var res: dynamic = cnt;
  var one: dynamic = 0;
  {
    var i: dynamic = 1;
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
