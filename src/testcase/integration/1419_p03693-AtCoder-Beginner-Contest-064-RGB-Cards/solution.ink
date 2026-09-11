// Translated from solution.cpp.

func main() -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(r, g, b);
  var a: dynamic = ((10 * g) + b);
  if ((0 == (a % 4)))
  {
    write("YES");
  } else
  {
    write("NO");
  }
}
