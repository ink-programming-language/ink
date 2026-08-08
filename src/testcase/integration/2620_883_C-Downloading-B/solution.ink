// Translated from solution.cpp.

var N = (1e7 + 5);

var LINF = 1e18;

func main()
{
  ios.sync_with_stdio(false);
  var f: dynamic;
  var t: dynamic;
  var t0: dynamic;
  read(f, t, t0);
  var a1: dynamic;
  var t1: dynamic;
  var p1: dynamic;
  read(a1, t1, p1);
  var a2: dynamic;
  var t2: dynamic;
  var p2: dynamic;
  read(a2, t2, p2);
  var bs = __cpp_lambda_1;
  var ans = LINF;
  {
    var usage = 0;
    while ((usage < (f + 1)))
    {
      var lfdata = (f - usage);
      var lftime = (t - (usage * t1));
      var price = ((((usage + ((a1 - 1)))) / a1) * p1);
      if ((t0 <= t2))
      {
        if (((lfdata * t0) <= lftime))
        {
          ans = min(ans, price);
        }
      } else
      {
        ans = min(ans, (price + bs(lfdata, lftime)));
      }
      usage += 1;
    }
  }
  write((if ((ans != LINF)) ans else -1), "\n");
}

func __cpp_lambda_1(data: dynamic, time: dynamic)
{
  if ((((data * t0) > time) && ((data * t2) > time)))
  {
    return LINF;
  }
  var l = 0;
  var r = (((data + ((a2 - 1)))) / a2);
  while ((l < r))
  {
    var mid = (((l + r)) >> 1);
    var usage = min((mid * a2), data);
    if ((((usage * t2) + (((data - usage)) * t0)) <= time))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  return (l * p2);
}
