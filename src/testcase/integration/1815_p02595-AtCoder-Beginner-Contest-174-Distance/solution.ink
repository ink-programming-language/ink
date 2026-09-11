// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n, d);
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x, y);
      if ((((x * x) + (y * y)) <= (d * d)))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
