// Translated from solution.cpp.

var N: dynamic = 5050;

var min_depth: dynamic = cpp_array(N);

var max_depth: dynamic = cpp_array(N);

func prec() -> dynamic
{
  {
    var n: dynamic = 1;
    while ((n <= 5000))
    {
      var nn: dynamic = (n - 1);
      var h: dynamic = 0;
      var cnt: dynamic = 1;
      while (nn)
      {
        h += 1;
        cnt *= 2;
        min_depth[n] += (min(nn, cnt) * h);
        nn -= min(nn, cnt);
      }
      max_depth[n] = ((n * ((n - 1))) / 2);
      n += 1;
    }
  }
}

var timer: dynamic = 0;

var p: dynamic = cpp_uninitialized();

func solver(root: dynamic, n: dynamic, d: dynamic) -> dynamic
{
  if ((n == 1))
  {
    return;
  }
  d -= ((n - 1));
  {
    var l: dynamic = 0;
    while ((l <= (n - 1)))
    {
      var r: dynamic = (((n - 1)) - l);
      if ((!((((min_depth[l] + min_depth[r]) <= d) && (d <= (max_depth[l] + max_depth[r]))))))
      {
        l += 1;
        continue;
      }
      var flag: dynamic = 0;
      for (var depth_l: dynamic in [min_depth[l], max_depth[l]])
      {
        if (((min_depth[r] <= (d - depth_l)) && ((d - depth_l) <= max_depth[r])))
        {
          if (l)
          {
            var lv: dynamic = cpp_update(timer, "++");
            p.push_back(root);
            solver(lv, l, depth_l);
          }
          if (r)
          {
            var rv: dynamic = cpp_update(timer, "++");
            p.push_back(root);
            solver(rv, r, (d - depth_l));
          }
          flag = 1;
          break;
        }
      }
      if (flag)
      {
        break;
      }
      l += 1;
    }
  }
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, d);
  if ((!(((min_depth[n] <= d) && (d <= max_depth[n])))))
  {
    write("NO\n");
    return;
  }
  write("YES\n");
  p.clear();
  timer = 2;
  solver(1, n, d);
  for (var x: dynamic in p)
  {
    write(x, " ");
  }
  write("\n");
}

func main() -> dynamic
{
  ios.sync_with_stdio(null);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(20);
  prec();
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
