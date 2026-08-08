// Translated from solution.cpp.

var i: dynamic;

var n: dynamic;

var a: dynamic;

var m: dynamic;

var g: dynamic;

var b = cpp_array(100000);

func main(argument_0: dynamic)
{
  while (1)
  {
    scanf("%d", (&n));
    if ((n == 0))
    {
      break;
    }
    m = 0;
    g = 0;
    {
      i = 0;
      while ((i < n))
      {
        scanf("%d", (&a));
        m += a;
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < (n - 1)))
      {
        scanf("%d", (&b[i]));
        i += 1;
      }
    }
    sort(b, (b + ((n - 1))));
    b[(n - 1)] = 0;
    {
      i = (n - 1);
      while ((i >= 0))
      {
        if ((g < (((m + b[i])) * ((i + 1)))))
        {
          g = (((m + b[i])) * ((i + 1)));
        }
        m += b[i];
        i -= 1;
      }
    }
    printf("%lld\n", g);
  }
  return 0;
}
