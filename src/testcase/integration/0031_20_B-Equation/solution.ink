// Translated from solution.cpp.

func main()
{
  var A: dynamic;
  var B: dynamic;
  var C: dynamic;
  var D: dynamic;
  read(A, B, C);
  var x1: dynamic;
  var x2: dynamic;
  if ((!A))
  {
    if ((!B))
    {
      if ((!C))
      {
        printf("-1");
        return 0;
      } else
      {
        printf("0");
        return 0;
      }
    } else
    {
      printf("1\n");
      printf("%lf", ((-C) / B));
    }
  } else
  {
    D = (pow(B, 2) - (((4 * A) * C)));
    if ((D < 0))
    {
      printf("0");
      return 0;
    } else
    {
      D = sqrt(D);
    }
    x1 = ((((-B) + D)) / ((2 * A)));
    x2 = ((((-B) - D)) / ((2 * A)));
    if ((x1 == x2))
    {
      write("1", "\n");
      printf("%lf", x1);
    } else
    {
      if (((x1 > x2))) printf("2\n%lf\n%lf", x2, x1) else printf("2\n%lf\n%lf", x1, x2);
    }
  }
}
