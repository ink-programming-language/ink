// Translated from solution.cpp.

var N: dynamic = (1e7 + 5);

var LINF: dynamic = 1e18;

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var f: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var t0: dynamic = cpp_uninitialized();
  read(f, t, t0);
  var a1: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var p1: dynamic = cpp_uninitialized();
  read(a1, t1, p1);
  var a2: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  read(a2, t2, p2);
  var bs: dynamic = __cpp_lambda_1;
  var ans: dynamic = LINF;
  {
    var usage: dynamic = 0;
    while ((usage < (f + 1)))
    {
      var lfdata: dynamic = (f - usage);
      var lftime: dynamic = (t - (usage * t1));
      var price: dynamic = ((((usage + ((a1 - 1)))) / a1) * p1);
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
  write(( ((ans != LINF)) ? ans : -1), "\n");
}

func __cpp_lambda_1(data: dynamic, time: dynamic) -> dynamic
{
  if ((((data * t0) > time) && ((data * t2) > time)))
  {
    return LINF;
  }
  var l: dynamic = 0;
  var r: dynamic = (((data + ((a2 - 1)))) / a2);
  while ((l < r))
  {
    var mid: dynamic = (((l + r)) >> 1);
    var usage: dynamic = min((mid * a2), data);
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
