// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  var second: dynamic;
  var ans = 0;
  var k = 1;
  var i: dynamic;
  var a: dynamic;
  read(n, second);
  var mp: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      read(a);
      if (((i == second) && a))
      {
        ans += 1;
      } else
      {
        mp[a] += 1;
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < n))
    {
      if ((k >= n))
      {
        break;
      }
      if ((mp[i] == 0))
      {
        k += 1;
        ans += 1;
      } else
      {
        k += mp[i];
      }
      i += 1;
    }
  }
  write(ans);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
