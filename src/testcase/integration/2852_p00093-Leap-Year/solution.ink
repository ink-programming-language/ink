// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var y: dynamic;
  var y2: dynamic;
  var i: dynamic;
  var flg: dynamic;
  var flg2: dynamic;
  flg = 0;
  scanf("%d %d", (&y), (&y2));
  while (((y != 0) && (y2 != 0)))
  {
    if ((flg == 0))
    {
      flg = 1;
    } else
    {
      printf("\n");
    }
    flg2 = 0;
    {
      i = y;
      while ((i <= y2))
      {
        if (((y % 400) == 0))
        {
          printf("%d\n", y);
          flg2 = 1;
        }
        if (((y % 4) == 0))
        {
          if (((y % 100) != 0))
          {
            printf("%d\n", y);
            flg2 = 1;
          }
        }
        y += 1;
        i += 1;
      }
    }
    if ((flg2 == 0))
    {
      printf("NA\n");
    }
    scanf("%d %d", (&y), (&y2));
  }
  return 0;
}
