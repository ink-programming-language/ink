// Translated from solution.cpp.

func mego()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func main()
{
  var n: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var x: dynamic;
  var s: dynamic;
  a = cpp_assign(b, "=", cpp_assign(c, "=", cpp_assign(d, "=", 0)));
  read(n);
  var ma: dynamic;
  var mb: dynamic;
  var v: dynamic;
  var se: dynamic;
  var ans: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(s, x);
      if (ma.count(s))
      {
        ma[s] = max(x, ma[s]);
      } else
      {
        ma[s] = x;
      }
      i += 1;
    }
  }
  for (var it in ma)
  {
    mb.push_back([it.second, it.first]);
    se.insert(it.second);
  }
  for (var it in se)
  {
    v.push_back(it);
  }
  sort(mb.begin(), mb.end());
  x = 0;
  var idx = 0;
  {
    var i = 0;
    while ((i < mb.size()))
    {
      a = i;
      {
        var j = i;
        while ((j < mb.size()))
        {
          if ((mb[j].first == mb[i].first))
          {
            a = j;
          } else
          {
            break;
          }
          j += 1;
        }
      }
      a = ((((((a + 1)) * 100)) / mb.size()));
      if ((a >= 99))
      {
        ans[mb[i].second] = "pro";
      } else if ((a >= 90))
      {
        ans[mb[i].second] = "hardcore";
      } else if ((a >= 80))
      {
        ans[mb[i].second] = "average";
      } else if ((a >= 50))
      {
        ans[mb[i].second] = "random";
      } else
      {
        ans[mb[i].second] = "noob";
      }
      x += 1;
      i += 1;
    }
  }
  write(ans.size(), "\n");
  for (var it in ans)
  {
    write(it.first, " ", it.second, "\n");
  }
  return 0;
}
