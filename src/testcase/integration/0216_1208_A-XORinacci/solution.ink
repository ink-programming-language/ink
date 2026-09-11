// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    read(a, b, n);
    n %= 3;
    var pd: dynamic = cpp_array(3);
    pd[0] = a;
    pd[1] = b;
    pd[2] = (pd[0] ^ pd[1]);
    write(pd[n], "\n");
  }
  return 0;
}
