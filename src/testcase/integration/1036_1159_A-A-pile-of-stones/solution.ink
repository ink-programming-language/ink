// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var TESTS: dynamic = 1;
  while (cpp_update(TESTS, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var s: dynamic = cpp_uninitialized();
    read(s);
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((s[i] == cpp_char("-")))
        {
          ans -= 1;
        } else
        {
          ans += 1;
        }
        var c: dynamic = 0;
        ans = max(c, ans);
        i += 1;
      }
    }
    write(ans);
  }
  return 0;
}
