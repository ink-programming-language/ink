// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i].first);
      v[i].second = i;
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct((n + 1));
  var p: dynamic = cpp_construct((n + 1), -1);
  sort(v.begin(), v.end());
  fill(dp.begin(), dp.end(), 2e15);
  dp[0] = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 2);
        while (((j < (i + 5)) && (j < n)))
        {
          var diff: dynamic = (v[j].first - v[i].first);
          if (((dp[i] + diff) < dp[(j + 1)]))
          {
            dp[(j + 1)] = (dp[i] + diff);
            p[(j + 1)] = i;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var tail: dynamic = n;
  var teamId: dynamic = 0;
  while ((tail > 0))
  {
    {
      var i: dynamic = (tail - 1);
      while ((i >= p[tail]))
      {
        ans[v[i].second] = (teamId + 1);
        i -= 1;
      }
    }
    tail = p[tail];
    teamId += 1;
  }
  write(dp[n], " ", teamId, "\n");
  for (var e: dynamic in ans)
  {
    write(e, " ");
  }
  write("\n");
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
