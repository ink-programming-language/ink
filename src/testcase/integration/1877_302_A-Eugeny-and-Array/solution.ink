// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d%d", (&n), (&m));
  var a: dynamic;
  var b: dynamic;
  var one = 0;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a));
      if ((a == 1))
      {
        one += 1;
      }
      i += 1;
    }
  }
  var pone = (n - one);
  while (cpp_update(m, "--"))
  {
    scanf("%d%d", (&a), (&b));
    var tmp = ((b - a) + 1);
    if ((tmp & 1))
    {
      puts("0");
    } else
    {
      if ((((tmp / 2) <= one) && ((tmp / 2) <= pone)))
      {
        puts("1");
      } else
      {
        puts("0");
      }
    }
  }
  return 0;
}
