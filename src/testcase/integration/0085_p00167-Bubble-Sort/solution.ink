// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a = cpp_array(100);
  while (cpp_comma(scanf("%d", (&n)), (n != 0)))
  {
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%d", (&a[i]));
        i += 1;
      }
    }
    var c = 0;
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = (i + 1);
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
