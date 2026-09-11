// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var i: dynamic = 1;
  read(n);
  read(a);
  while (true)
  {
    read(b);
    if ((a < b))
    {
      c = b;
      b = a;
      a = c;
    }
    while ((a % b))
    {
      c = b;
      b = (a % b);
      a = c;
    }
    a = b;
    i += 1;
    if (!(((i < n))))
    {
      break;
    }
  }
  write(a);
  return 0;
}
