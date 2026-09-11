// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var x: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var remaining: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      var cur: dynamic = min(remaining, (x / 2));
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
