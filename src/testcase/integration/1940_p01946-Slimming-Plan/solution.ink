// Translated from solution.cpp.

var inf: dynamic = 1e9;

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(s, t, d);
  var aim: dynamic = (s - t);
  {
    var i: dynamic = 0;
    while ((i < d))
    {
      read(w[i]);
      i += 1;
    }
  }
  var sumw: dynamic = w[0];
  var dp: dynamic = cpp_construct(d, 0);
  dp[0] = w[0];
  {
    var i: dynamic = 1;
    while ((i < d))
    {
      sumw += w[i];
      dp[i] = (dp[(i - 1)] + w[i]);
      i += 1;
    }
  }
  var mi: dynamic = inf;
  {
    var i: dynamic = 0;
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
  var res: dynamic = (((aim - mi)) / ((-sumw)));
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
