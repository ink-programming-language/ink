// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var n: dynamic;
  while ((scanf("%d", (&n)) != EOF))
  {
    var d = cpp_new();
    var count = 0;
    {
      var i = 0;
      while ((i <= n))
      {
        d[i] = true;
        i += 1;
      }
    }
    {
      var i = 2;
      while ((i <= n))
      {
        if (d[i])
        {
          count += 1;
          {
            var j = (i * 2);
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
