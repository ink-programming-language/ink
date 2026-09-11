// Translated from solution.cpp.

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(106);
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  read(m);
  {
    i = 1;
    while ((i <= m))
    {
      read(x, y);
      a[(x - 1)] += (y - 1);
      a[(x + 1)] += (a[x] - y);
      a[x] = 0;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      write(a[i], "\n");
      i += 1;
    }
  }
  return 0;
}
