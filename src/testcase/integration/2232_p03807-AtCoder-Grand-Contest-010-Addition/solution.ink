// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var k: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      if ((a % 2))
      {
        k += 1;
      }
      i += 1;
    }
  }
  if ((k % 2))
  {
    write("NO", "\n");
  } else
  {
    write("YES", "\n");
  }
}
