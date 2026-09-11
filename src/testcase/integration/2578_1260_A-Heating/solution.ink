// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var N: dynamic = cpp_uninitialized();
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var c: dynamic = cpp_uninitialized();
      var sum: dynamic = cpp_uninitialized();
      read(c, sum);
      var x: dynamic = (sum / c);
      var big: dynamic = (sum - (c * x));
      var small: dynamic = (c - big);
      var ans: dynamic = (((small * x) * x) + ((big * ((x + 1))) * ((x + 1))));
      write(ans, cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
