// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(a, b, c, d);
  {
    var i: dynamic = 0;
    while ((i <= 1e6))
    {
      var x: dynamic = ((((i * c)) + d) - b);
      if ((x < 0))
      {
        i += 1;
        continue;
      }
      if (((x % a) == 0))
      {
        write((x + b));
        return 0;
      }
      i += 1;
    }
  }
  write("-1");
}
