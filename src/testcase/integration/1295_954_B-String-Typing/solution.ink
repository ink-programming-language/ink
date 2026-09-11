// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var ans: dynamic = n;
  {
    var i: dynamic = 0;
    while ((i < (n / 2)))
    {
      var temp1: dynamic = s.substr(0, (i + 1));
      var temp2: dynamic = s.substr((i + 1), (i + 1));
      if ((temp1 == temp2))
      {
        ans = min(ans, (n - i));
      }
      i += 1;
    }
  }
  write(ans);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
