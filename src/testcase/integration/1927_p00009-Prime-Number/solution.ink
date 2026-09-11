// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((scanf("%d", (&n)) != EOF))
  {
    var d: dynamic = cpp_new();
    var count: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i <= n))
      {
        d[i] = true;
        i += 1;
      }
    }
    {
      var i: dynamic = 2;
      while ((i <= n))
      {
        if (d[i])
        {
          count += 1;
          {
            var j: dynamic = (i * 2);
            while ((j <= n))
            {
              d[j] = false;
              j += i;
            }
          }
        }
        i += 1;
      }
    }
    printf("%d\n", count);
  }
  return 0;
}
