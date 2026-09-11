// Translated from solution.cpp.

func compute(ds: dynamic, dt: dynamic, ans: dynamic, flg: dynamic) -> dynamic
{
  var swp: dynamic = 0;
  var totlens: dynamic = 0;
  var totlent: dynamic = 0;
  for (var x: dynamic in ds)
  {
    totlens += x.second;
  }
  for (var x: dynamic in dt)
  {
    totlent += x.second;
  }
  var perform: dynamic = __cpp_lambda_1;
  if ((ds[0].first == dt[0].first))
  {
    if ((ds.size() > dt.size()))
    {
      swap(totlens, totlent);
      swap(ds, dt);
      swp ^= 1;
    }
    var r: dynamic = (((int_cpp(dt.size()) - int_cpp(ds.size()))) / 2);
    if ((r & 1))
    {
      perform(0, r);
    } else
    {
      perform(0, (r + 1));
    }
  }
  while (((ds.size() > 1) || (dt.size() > 1)))
  {
    if ((ds.size() > dt.size()))
    {
      swap(totlens, totlent);
      swap(ds, dt);
      swp ^= 1;
    }
    if ((ds.size() == 1))
    {
      if ((dt.size() == 2))
      {
        perform(1, 1);
        return;
      }
      if ((dt.size() == 3))
      {
        perform(1, 1);
        perform(1, 1);
        return;
      }
      perform(1, 3);
    } else if ((dt.size() <= 3))
    {
      perform(1, 1);
    } else if ((ds.size() == 2))
    {
      perform(1, 3);
    } else
    {
      perform(1, 1);
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  write(setprecision(32));
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var ds: dynamic = cpp_uninitialized();
  var dt: dynamic = cpp_uninitialized();
  for (var ch: dynamic in s)
  {
    if ((ds.empty() || (ds.back().first != ch)))
    {
      ds.push_back([ch, 1]);
    } else
    {
      ds.back().second += 1;
    }
  }
  for (var ch: dynamic in t)
  {
    if ((dt.empty() || (dt.back().first != ch)))
    {
      dt.push_back([ch, 1]);
    } else
    {
      dt.back().second += 1;
    }
  }
  var ans: dynamic = cpp_uninitialized();
  if ((ds.front().first != dt.front().first))
  {
    compute(ds, dt, ans, false);
  } else if ((ds.back().first != dt.back().first))
  {
    reverse(ds.begin(), ds.end());
    reverse(dt.begin(), dt.end());
    compute(ds, dt, ans, true);
    {
      var i: dynamic = 1;
      while ((i < ans.size()))
      {
        swap(ans[i][0], ans[i][1]);
        i += 2;
      }
    }
  } else
  {
    compute(ds, dt, ans, false);
  }
  write(ans.size(), cpp_char("\n"));
  for (var x: dynamic in ans)
  {
    write(x[0], " ", x[1], cpp_char("\n"));
  }
  return 0;
}

func __cpp_lambda_1(blks: dynamic, blkt: dynamic) -> dynamic
{
  var lens: dynamic = 0;
  var lent: dynamic = 0;
  var vs: dynamic = cpp_uninitialized();
  var vt: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < blks))
    {
      vs.push_back(ds.front());
      lens += ds[0].second;
      ds.pop_front();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < blkt))
    {
      vt.push_back(dt.front());
      lent += dt[0].second;
      dt.pop_front();
      i += 1;
    }
  }
  if (flg)
  {
    if (swp)
    {
      ans.push_back([(totlent - lent), (totlens - lens)]);
    } else
    {
      ans.push_back([(totlens - lens), (totlent - lent)]);
    }
  } else
  {
    if (swp)
    {
      ans.push_back([lent, lens]);
    } else
    {
      ans.push_back([lens, lent]);
    }
  }
  totlent = ((totlent - lent) + lens);
  totlens = ((totlens - lens) + lent);
  while ((!vt.empty()))
  {
    if ((ds.empty() || (ds.front().first != vt.back().first)))
    {
      ds.push_front(vt.back());
    } else
    {
      ds.front().second += vt.back().second;
    }
    vt.pop_back();
  }
  while ((!vs.empty()))
  {
    if ((dt.empty() || (dt.front().first != vs.back().first)))
    {
      dt.push_front(vs.back());
    } else
    {
      dt.front().second += vs.back().second;
    }
    vs.pop_back();
  }
}
