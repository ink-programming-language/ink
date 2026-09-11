// Translated from solution.cpp.

var rng: dynamic = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var c: dynamic = cpp_uninitialized();
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
