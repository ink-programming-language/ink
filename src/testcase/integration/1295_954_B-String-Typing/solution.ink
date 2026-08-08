// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  var ans = n;
  {
    var i = 0;
    while ((i < (n / 2)))
    {
      var temp1 = s.substr(0, (i + 1));
      var temp2 = s.substr((i + 1), (i + 1));
      if ((temp1 == temp2))
      {
        ans = min(ans, (n - i));
      }
      i += 1;
    }
  }
  write(ans);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
