// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  read(n);
  var mp: dynamic;
  var mx = 1;
  var st: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      mp[x] += 1;
      mx = max(mx, mp[x]);
      st.insert(x);
      i += 1;
    }
  }
  var ct = 0;
  for (var it in mp)
  {
    if ((it.second == mx))
    {
      ct += 1;
    }
  }
  var tt = (((n - (mx * ct)) + mx) - 1);
  var dvd = (tt / ((mx - 1)));
  write(((((dvd - 1) + ct) - 1)), "\n");
  return 0;
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
