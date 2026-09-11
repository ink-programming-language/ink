// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(100);
  while (cpp_comma(scanf("%d", (&n)), (n != 0)))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&a[i]));
        i += 1;
      }
    }
    var c: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            if ((a[i] > a[j]))
            {
              c += 1;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", c);
  }
  return 0;
}
