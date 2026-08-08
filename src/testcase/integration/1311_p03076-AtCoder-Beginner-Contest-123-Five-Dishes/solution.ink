// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var sum = 0;
  var add = 0;
  {
    var i = 0;
    while ((i < 5))
    {
      scanf("%d", (&a));
      var m = (a % 10);
      if ((m == 0))
      {
      } else
      {
        add = if (((add > (10 - m)))) add else ((10 - m));
        a += ((10 - m));
      }
      sum += a;
      i += 1;
    }
  }
  printf("%d\n", (sum - add));
}
