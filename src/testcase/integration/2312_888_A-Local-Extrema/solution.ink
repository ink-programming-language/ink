// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array((n + 5));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var coun: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      if (((a[i] < a[(i - 1)]) && (a[i] < a[(i + 1)])))
      {
        coun += 1;
      } else if (((a[i] > a[(i - 1)]) && (a[i] > a[(i + 1)])))
      {
        coun += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", coun);
}
