// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(t);
  {
    i = 0;
    while ((i < t))
    {
      read(x, y, n);
      r = (((((n / x)) * x)) + y);
      if ((r > n))
      {
        r -= x;
      }
      write(r, "\n");
      i += 1;
    }
  }
}
