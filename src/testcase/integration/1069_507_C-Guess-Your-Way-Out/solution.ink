// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var ans: dynamic;

func main()
{
  scanf("%d%lld", (&n), (&m));
  m -= 1;
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      var ka = ((((m >> i)) & 1));
      var kb = ((((m >> ((i + 1)))) & 1));
      if ((i == (n - 1)))
      {
        if (ka)
        {
          ans += ((1 << ((i + 1))));
        } else
        {
          ans += 1;
        }
      } else
      {
        if ((ka == kb))
        {
          ans += ((1 << ((i + 1))));
        } else
        {
          ans += 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
