// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var N: dynamic;
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      var c: dynamic;
      var sum: dynamic;
      read(c, sum);
      var x = (sum / c);
      var big = (sum - (c * x));
      var small = (c - big);
      var ans = (((small * x) * x) + ((big * ((x + 1))) * ((x + 1))));
      write(ans, cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
