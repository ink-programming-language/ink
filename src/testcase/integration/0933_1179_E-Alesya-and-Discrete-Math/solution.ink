// Translated from solution.cpp.

func query(i: dynamic, x: dynamic) -> dynamic
{
  write("? ", (i + 1), cpp_char(" "), x, "\n");
  var r: dynamic = cpp_uninitialized();
  read(r);
  return r;
}

var ans: dynamic = cpp_array(1000);

var mrand: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var log_fac: dynamic = cpp_array(2000);

func log_ncr(n: dynamic, k: dynamic) -> dynamic
{
  if ((((n < 0) || (k < 0)) || (k > n)))
  {
    return -1e9;
  }
  return ((log_fac[n] - log_fac[k]) - log_fac[(n - k)]);
}

func prob(a: dynamic, b: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  assert((((((a >= 0) && (b >= 0)) && (m >= 0)) && (m <= n)) && (n >= 0)));
  if ((a > m))
  {
    return 1;
  }
  if (((n - b) <= m))
  {
    return 0;
  }
  var k: dynamic = (a + b);
  var x: dynamic = m;
  var r: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      r += exp(((log_ncr(x, i) + log_ncr((n - x), (k - i))) - log_ncr(n, k)));
      i += 1;
    }
  }
  return r;
}

func recSplit(xl: dynamic, xr: dynamic, ym: dynamic, sl: dynamic, v: dynamic) -> dynamic
{
  if ((xr <= xl))
  {
    return xl;
  }
  assert(((sl >= 0) && (sl <= v.size())));
  if ((sl == 0))
  {
    return xl;
  }
  if ((sl == v.size()))
  {
    return xr;
  }
  shuffle(v.begin(), v.end(), mrand);
  var vl: dynamic = cpp_uninitialized();
  var vr: dynamic = cpp_uninitialized();
  var vm: dynamic = cpp_uninitialized();
  var xm: dynamic = ((xl + xr) >> 1);
  var assume: dynamic = -9;
  for (var i: dynamic in v)
  {
    var y: dynamic = cpp_uninitialized();
    if ((assume == -9))
    {
      y = query(i, xm);
    } else
    {
      y = assume;
    }
    if ((y > ym))
    {
      vl.push_back(i);
    } else if ((y < ym))
    {
      vr.push_back(i);
    } else
    {
      vm.push_back(i);
    }
    if (((assume == -9) && (prob(vl.size(), (vm.size() + vr.size()), sl, v.size()) > (1 - 1e-2))))
    {
      assume = (ym + 1);
    }
    if (((assume == -9) && (prob(vr.size(), (vm.size() + vl.size()), (cpp_cast(v.size()) - sl), v.size()) > (1 - 1e-2))))
    {
      assume = (ym - 1);
    }
  }
  if ((vl.size() > sl))
  {
    return recSplit(xl, (xm - 1), ym, sl, vl);
  } else if ((vr.size() > (cpp_cast(v.size()) - sl)))
  {
    return recSplit((xm + 1), xr, ym, (sl - int_cpp((vl.size() + vm.size()))), vr);
  } else
  {
    return xm;
  }
}

var dy: dynamic = cpp_uninitialized();

func solve(xl: dynamic, xr: dynamic, yl: dynamic, yr: dynamic, v: dynamic) -> dynamic
{
  assert(((yr - yl) == (dy * v.size())));
  if ((v.size() <= 1))
  {
    for (var i: dynamic in v)
    {
      ans[i] = [xl, xr];
    }
    return;
  }
  var sl: dynamic = (v.size() / 2);
  var ym: dynamic = (yl + (dy * sl));
  while (1)
  {
    var xm: dynamic = recSplit(xl, xr, ym, sl, v);
    var vl: dynamic = cpp_uninitialized();
    var vr: dynamic = cpp_uninitialized();
    var vm: dynamic = cpp_uninitialized();
    for (var i: dynamic in v)
    {
      var y: dynamic = query(i, xm);
      if ((y > ym))
      {
        vl.push_back(i);
      } else if ((y < ym))
      {
        vr.push_back(i);
      } else
      {
        vm.push_back(i);
      }
    }
    if (((vl.size() > sl) || ((vl.size() + vm.size()) < sl)))
    {
      continue;
    }
    while ((vl.size() < sl))
    {
      vl.push_back(vm.back());
      vm.pop_back();
    }
    while (vm.size())
    {
      vr.push_back(vm.back());
      vm.pop_back();
    }
    solve(xl, xm, yl, ym, vl);
    solve(xm, xr, ym, yr, vr);
    break;
  }
}

func main() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i < 2000))
    {
      log_fac[i] = (log_fac[(i - 1)] + log(i));
      i += 1;
    }
  }
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  read(n, l);
  dy = (l / n);
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      v.push_back(i);
      i += 1;
    }
  }
  solve(0, 1e18, 0, l, v);
  write("! ", "\n");
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(ans[i].first, cpp_char(" "), ans[i].second, "\n");
      i += 1;
    }
  }
}
