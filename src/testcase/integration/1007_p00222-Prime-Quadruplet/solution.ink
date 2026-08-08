// Translated from solution.cpp.

var prim = cpp_array(10000000);

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  {
    i = 2;
    while ((i < (10000000 / 2)))
    {
      {
        j = 2;
        while ((j < (10000000 / 2)))
        {
          if ((((i * j) > 10000000) || (prim[i] == 1)))
          {
            break;
          }
          prim[(i * j)] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  while (1)
  {
    scanf("%d", (&n));
    if ((n == 0))
    {
      break;
    }
    {
      i = n;
      while ((n >= 0))
      {
        if ((prim[i] == 0))
        {
          if ((((prim[(i - 2)] == 0) && (prim[(i - 6)] == 0)) && (prim[(i - 8)] == 0)))
          {
            printf("%d\n", i);
            break;
          }
        }
        i -= 1;
      }
    }
  }
  return 0;
}
