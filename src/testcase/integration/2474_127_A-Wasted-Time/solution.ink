// Translated from solution.cpp.

func absolute(a: dynamic, b: dynamic) -> dynamic
{
  write(setprecision(20));
  return sqrt(((a * a) + (b * b)));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var pa: dynamic = 0;
  var pb: dynamic = 0;
  var len: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a, b);
      if ((i != 0))
      {
        len += absolute(abs((a - pa)), abs((b - pb)));
      }
      pa = a;
      pb = b;
      i += 1;
    }
  }
  write((((len * k)) / 50));
}
