// Translated from solution.cpp.

var rng = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var c: dynamic;
      read(c);
      if (((((c - cpp_char("0"))) % 2) == 0))
      {
        ans += i;
      }
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}
