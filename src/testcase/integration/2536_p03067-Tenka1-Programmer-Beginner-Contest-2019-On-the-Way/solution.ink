// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a, b, c);
  if (((((a - c)) * ((b - c))) < 0))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
}
