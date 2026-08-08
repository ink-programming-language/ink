// Translated from solution.cpp.

var N = 5050;

var min_depth = cpp_array(N);

var max_depth = cpp_array(N);

func prec()
{
  {
    var n = 1;
    while ((n <= 5000))
    {
      var nn = (n - 1);
      var h = 0;
      var cnt = 1;
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

var timer = 0;

var p: dynamic;

func solver(root: dynamic, n: dynamic, d: dynamic)
{
  if ((n == 1))
  {
    return;
  }
  d -= ((n - 1));
  {
    var l = 0;
    while ((l <= (n - 1)))
    {
      var r = (((n - 1)) - l);
      if ((!((((min_depth[l] + min_depth[r]) <= d) && (d <= (max_depth[l] + max_depth[r]))))))
      {
        l += 1;
        continue;
      }
      var flag = 0;
      for (var depth_l in [min_depth[l], max_depth[l]])
      {
        if (((min_depth[r] <= (d - depth_l)) && ((d - depth_l) <= max_depth[r])))
        {
          if (l)
          {
            var lv = cpp_update(timer, "++");
            p.push_back(root);
            solver(lv, l, depth_l);
          }
          if (r)
          {
            var rv = cpp_update(timer, "++");
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

func solve()
{
  var n: dynamic;
  var d: dynamic;
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
  for (var x in p)
  {
    write(x, " ");
  }
  write("\n");
}

func main()
{
  ios.sync_with_stdio(null);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(20);
  prec();
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
