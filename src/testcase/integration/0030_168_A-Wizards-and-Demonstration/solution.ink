// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  read(n, x, y);
  p = (y / 100);
  q = (n * p);
  r = (q - x);
  a = r;
  if ((r > a))
  {
    write((a + 1));
  } else if ((a < 0))
  {
    write("0");
  } else
  {
    write(a);
  }
}
