// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a);
  read(b);
  read(c);
  if (((((a + b) == c) || ((b + c) == a)) || ((c + a) == b)))
  {
    write("Yes");
  } else
  {
    write("No");
  }
  return 0;
}
