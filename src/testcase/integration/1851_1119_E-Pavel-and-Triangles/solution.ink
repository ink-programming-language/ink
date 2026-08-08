// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var x: dynamic;
  var ans = 0;
  var remaining = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      var cur = min(remaining, (x / 2));
      ans += cur;
      x -= (2 * cur);
      remaining -= cur;
      ans += (x / 3);
      x %= 3;
      remaining += x;
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}
