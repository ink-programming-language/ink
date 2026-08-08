// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var ni = cpp_array(7);
  var i: dynamic;
  var to: dynamic;
  while ((0 <= scanf("%d", (&n))))
  {
    if ((n == 0))
    {
      break;
    }
    {
      i = 0;
      while ((i < 7))
      {
        ni[i] = 0;
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < n))
      {
        scanf("%d", (&to));
        if ((to < 10))
        {
          ni[0] += 1;
        } else if ((to < 20))
        {
          ni[1] += 1;
        } else if ((to < 30))
        {
          ni[2] += 1;
        } else if ((to < 40))
        {
          ni[3] += 1;
        } else if ((to < 50))
        {
          ni[4] += 1;
        } else if ((to < 60))
        {
          ni[5] += 1;
        } else
        {
          ni[6] += 1;
        }
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < 7))
      {
        printf("%d\n", ni[i]);
        i += 1;
      }
    }
  }
  return 0;
}
