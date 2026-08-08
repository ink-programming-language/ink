// Translated from solution.cpp.

var inf = 1e9;

func main()
{
  var s: dynamic;
  var t: dynamic;
  var d: dynamic;
  read(s, t, d);
  var aim = (s - t);
  {
    var i = 0;
    while ((i < d))
    {
      read(w[i]);
      i += 1;
    }
  }
  var sumw = w[0];
  var dp = cpp_construct(d, 0);
  dp[0] = w[0];
  {
    var i = 1;
    while ((i < d))
    {
      sumw += w[i];
      dp[i] = (dp[(i - 1)] + w[i]);
      i += 1;
    }
  }
  var mi = inf;
  {
    var i = 0;
    while ((i < d))
    {
      if (((aim + dp[i]) <= 0))
      {
        write((i + 1), "\n");
        return 0;
      }
      mi = min(mi, dp[i]);
      i += 1;
    }
  }
  if ((sumw >= 0))
  {
    write(-1, "\n");
    return 0;
  }
  mi = abs(mi);
  var res = (((aim - mi)) / ((-sumw)));
  aim += (res * sumw);
  res *= d;
  while ((aim > 0))
  {
    aim += w[(res % d)];
    res += 1;
  }
  write(res, "\n");
  return 0;
}
