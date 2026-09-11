// Translated from solution.cpp.

func main() -> dynamic
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
  write("1 ", a[0], "\n");
  if ((a[(n - 1)] > 0))
  {
    write("1 ", a[(n - 1)], "\n");
    write((n - 2), "\n");
    {
      var i: dynamic = 1;
      while ((i < (n - 1)))
      {
        write(a[i], " ");
        i += 1;
      }
    }
    write("\n");
  } else
  {
    write("2 ", a[1], " ", a[2], "\n");
    write((n - 3), " ");
    {
      var i: dynamic = 3;
      while ((i < n))
      {
        write(a[i], " ");
        i += 1;
      }
    }
  }
}
