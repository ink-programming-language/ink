// Translated from solution.cpp.

func main() -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(r, d);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var r1: dynamic = cpp_uninitialized();
      read(x, y, r1);
      var c: dynamic = sqrt(((x * x) + (y * y)));
      if ((((r - c) >= r1) && ((r - c) <= (d - r1))))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
