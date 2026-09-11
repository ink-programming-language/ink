// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a, b, c);
  if (cpp_binary((a == b), "and", (b == c)))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
  return 0;
}
