// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var a = cpp_array((n + 5));
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var coun = 0;
  {
    var i = 1;
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
