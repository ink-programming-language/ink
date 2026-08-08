// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var TESTS = 1;
  while (cpp_update(TESTS, "--"))
  {
    var n: dynamic;
    read(n);
    var s: dynamic;
    read(s);
    var ans = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if ((s[i] == cpp_char("-")))
        {
          ans -= 1;
        } else
        {
          ans += 1;
        }
        var c = 0;
        ans = max(c, ans);
        i += 1;
      }
    }
    write(ans);
  }
  return 0;
}
