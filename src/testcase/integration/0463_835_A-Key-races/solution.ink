// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var v1: dynamic = cpp_uninitialized();
  var v2: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  read(s, v1, v2, t1, t2);
  var st: dynamic = ((s * v1) + (t1 * 2));
  var nd: dynamic = ((s * v2) + (t2 * 2));
  if ((st < nd))
  {
    write("First");
  } else if ((st > nd))
  {
    write("Second");
  } else
  {
    write("Friendship");
  }
  return 0;
}
