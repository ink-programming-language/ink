// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(a, v, b, w, t);
  var sum: dynamic = (t * ((v - w)));
  if ((sum >= ((max(a, b) - min(a, b)))))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
