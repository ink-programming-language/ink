// Translated from solution.cpp.

var a: dynamic = cpp_array(10000);

var b: dynamic = cpp_array(10000);

func main() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= 3))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= 3))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= 3))
    {
      if ((a[i] == b[i]))
      {
        write("YES");
        return 0;
      }
      i += 1;
    }
  }
  write("NO");
}
