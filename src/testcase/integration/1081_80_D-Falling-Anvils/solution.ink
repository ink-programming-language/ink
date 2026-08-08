// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var i: dynamic;
  var a: dynamic;
  var b: dynamic;
  scanf("%d", (&t));
  {
    i = 0;
    while ((i < t))
    {
      scanf("%lf%lf", (&a), (&b));
      if (a)
      {
        if (((a / 4) > b))
        {
          printf("%.6lf\n", (((a - b)) / a));
        } else
        {
          printf("%.6lf\n", (((((a / 4) + b)) / ((4 * b))) + 0.25));
        }
      } else if (b)
      {
        printf("0.5\n");
      } else
      {
        printf("1\n");
      }
      i += 1;
    }
  }
  return 0;
}
