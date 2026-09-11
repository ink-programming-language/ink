// Translated from solution.cpp.

func solve(l: dynamic, n: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  var cumx: dynamic = cpp_construct((n + 1), 0);
  var cumy: dynamic = cpp_construct((n + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      cumx[(i + 1)] = (cumx[i] + x[i]);
      cumy[(i + 1)] = (cumy[i] + y[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var tmp: dynamic = 0;
      var remain: dynamic = (n - i);
      var remx: dynamic = (((remain + 1)) / 2);
      var remy: dynamic = (remain - remx);
      tmp += (((cumx[(remx + i)] - cumx[i])) * 2);
      tmp += ((cumy[remy]) * 2);
      if ((remain & 1))
      {
        tmp -= x[((remx + i) - 1)];
      } else
      {
        tmp -= y[(remy - 1)];
      }
      ret = max(ret, tmp);
      i += 1;
    }
  }
  return ret;
}

func main() -> dynamic
{
  var l: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(l, n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      y[i] = (l - x[i]);
      i += 1;
    }
  }
  reverse(y.begin(), y.end());
  var ans1: dynamic = solve(l, n, x, y);
  var ans2: dynamic = solve(l, n, y, x);
  write(max(ans1, ans2), "\n");
  return 0;
}
