// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  write(fixed, setprecision(20));
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  if ((n == 1))
  {
    write(k, "\n");
    return 0;
  }
  var now = 1;
  var cnt = 0;
  while ((now < k))
  {
    now *= 2;
    cnt += 1;
  }
  n = min(n, cnt);
  var low = -1;
  var up = (k + 1);
  var mid: dynamic;
  while (((up - low) > 1))
  {
    mid = (((up + low)) / 2);
    var now = mid;
    var sum = 0;
    {
      var i = 0;
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
