// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  write(fixed, setprecision(20));
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  if ((n == 1))
  {
    write(k, "\n");
    return 0;
  }
  var now: dynamic = 1;
  var cnt: dynamic = 0;
  while ((now < k))
  {
    now *= 2;
    cnt += 1;
  }
  n = min(n, cnt);
  var low: dynamic = -1;
  var up: dynamic = (k + 1);
  var mid: dynamic = cpp_uninitialized();
  while (((up - low) > 1))
  {
    mid = (((up + low)) / 2);
    var now: dynamic = mid;
    var sum: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        sum += now;
        now /= 2;
        i += 1;
      }
    }
    if ((sum <= k))
    {
      low = mid;
    } else
    {
      up = mid;
    }
  }
  write(low, "\n");
}
