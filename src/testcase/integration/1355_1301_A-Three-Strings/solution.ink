// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(a, b, c);
    var v: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < a.size()))
      {
        if (((a[i] != c[i]) && (b[i] != c[i])))
        {
          v = 1;
          break;
        }
        i += 1;
      }
    }
    if ((v == 1))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
    }
  }
}
