// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func main()
{
  {
    var n: dynamic;
    while (cpp_comma(scanf("%d", (&n)), n))
    {
      var ans = [];
      var tot = [];
      var s = ["lunch", "dinner", "midnight"];
      rep(i, 3);
      {
        if ((tot[i] == 0))
        {
          printf("%s no guest\n", s[i]);
        } else
        {
          printf("%s %d\n", s[i], ((100 * ans[i]) / tot[i]));
        }
      }
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var h: dynamic;
        var m: dynamic;
        var m2: dynamic;
        scanf("%d:%d%d", (&h), (&m), (&m2));
        if ((m2 < m))
        {
          m2 += 60;
        }
        var t = ((60 * h) + m);
        if (((11 <= h) && (h < 15)))
        {
          tot[0] += 1;
          if (((m2 - m) <= 8))
          {
            ans[0] += 1;
          }
        } else if (((18 <= h) && (h < 21)))
        {
          tot[1] += 1;
          if (((m2 - m) <= 8))
          {
            ans[1] += 1;
          }
        } else if (((21 <= h) || (h < 2)))
        {
          tot[2] += 1;
          if (((m2 - m) <= 8))
          {
            ans[2] += 1;
          }
        }
      }
