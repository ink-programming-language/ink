// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var mp: dynamic = cpp_uninitialized();
  var mx: dynamic = 1;
  var st: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      mp[x] += 1;
      mx = max(mx, mp[x]);
      st.insert(x);
      i += 1;
    }
  }
  var ct: dynamic = 0;
  for (var it: dynamic in mp)
  {
    if ((it.second == mx))
    {
      ct += 1;
    }
  }
  var tt: dynamic = (((n - (mx * ct)) + mx) - 1);
  var dvd: dynamic = (tt / ((mx - 1)));
  write(((((dvd - 1) + ct) - 1)), "\n");
  return 0;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
