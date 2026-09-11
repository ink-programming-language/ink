// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(a, b);
  m = b;
  {
    var i: dynamic = 1;
    while ((i <= (a - 1)))
    {
      b *= ((m - 1));
      i += 1;
    }
  }
  write(b);
  return 0;
}
