// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var a: dynamic = cpp_array(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    var i: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < (n - 1)))
      {
        if (((a[(i + 1)] - a[i]) > 1))
        {
          write("NO", "\n");
          break;
        }
        i += 1;
      }
    }
    if ((i == (n - 1)))
    {
      write("YES", "\n");
    }
  }
}
