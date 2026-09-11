// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(a, b, c);
  {
    i = 0;
    while ((i <= (c / a)))
    {
      if (((((c - (i * a))) % b) == 0))
      {
        write("Yes");
        return 0;
      }
      i += 1;
    }
  }
  write("No");
}
