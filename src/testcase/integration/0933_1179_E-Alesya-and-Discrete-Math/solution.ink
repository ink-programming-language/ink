// Translated from solution.cpp.

func query(i: dynamic, x: dynamic)
{
  write("? ", (i + 1), cpp_char(" "), x, "\n");
  var r: dynamic;
  read(r);
  return r;
}

var ans = cpp_array(1000);

var mrand = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var log_fac = cpp_array(2000);

func log_ncr(n: dynamic, k: dynamic)
{
  if ((((n < 0) || (k < 0)) || (k > n)))
  {
    return -1e9;
  }
  return ((log_fac[n] - log_fac[k]) - log_fac[(n - k)]);
}

func prob(a: dynamic, b: dynamic, m: dynamic, n: dynamic)
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
  var k = (a + b);
  var x = m;
  var r = 0;
  {
    var i = 0;
    while ((i < a))
    {
      r += exp(((log_ncr(x, i) + log_ncr((n - x), (k - i))) - log_ncr(n, k)));
      i += 1;
    }
  }
  return r;
}

func recSplit(xl: dynamic, xr: dynamic, ym: dynamic, sl: dynamic, v: dynamic)
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
  var vl: dynamic;
  var vr: dynamic;
  var vm: dynamic;
  var xm = ((xl + xr) >> 1);
  var assume = -9;
  for (var i in v)
  {
    var y: dynamic;
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

var dy: dynamic;

func solve(xl: dynamic, xr: dynamic, yl: dynamic, yr: dynamic, v: dynamic)
{
  assert(((yr - yl) == (dy * v.size())));
  if ((v.size() <= 1))
  {
    for (var i in v)
    {
      ans[i] = [xl, xr];
    }
    return;
  }
  var sl = (v.size() / 2);
  var ym = (yl + (dy * sl));
  while (1)
  {
    var xm = recSplit(xl, xr, ym, sl, v);
    var vl: dynamic;
    var vr: dynamic;
    var vm: dynamic;
    for (var i in v)
    {
      var y = query(i, xm);
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

func main()
{
  {
    var i = 1;
    while ((i < 2000))
    {
      log_fac[i] = (log_fac[(i - 1)] + log(i));
      i += 1;
    }
  }
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var l: dynamic;
  read(n, l);
  dy = (l / n);
  var v: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      v.push_back(i);
      i += 1;
    }
  }
  solve(0, 1e18, 0, l, v);
  write("! ", "\n");
  {
    var i = 0;
    while ((i < n))
    {
      write(ans[i].first, cpp_char(" "), ans[i].second, "\n");
      i += 1;
    }
  }
}
