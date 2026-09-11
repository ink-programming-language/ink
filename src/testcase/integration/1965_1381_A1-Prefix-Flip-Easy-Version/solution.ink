// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    read(a);
    read(b);
    var k: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((a[i] != b[i]))
        {
          k += 1;
        }
        i += 1;
      }
    }
    k *= 3;
    write(k);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((a[i] != b[i]))
        {
          write(" ", (i + 1), " 1 ", (i + 1));
        }
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
