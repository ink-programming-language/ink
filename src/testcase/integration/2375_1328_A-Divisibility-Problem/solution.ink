// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      if (((a % b) == 0))
      {
        write("0\n");
      } else
      {
        write((b - ((a % b))), "\n");
      }
      i += 1;
    }
  }
}
