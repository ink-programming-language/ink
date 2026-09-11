// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  if (((((a * b)) % 2) == 0))
  {
    write("Even");
  } else
  {
    write("Odd");
  }
  return 0;
}
