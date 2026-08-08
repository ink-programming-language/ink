// Translated from solution.cpp.

var en = cpp_array((100 + 100));

var e = cpp_array((100 + 100));

func main()
{
  var n: dynamic;
  var q: dynamic;
  while ((~scanf("%d%d", (&n), (&q))))
  {
    memset(en, -1, cpp_sizeof((en)));
    memset(e, -1, cpp_sizeof((e)));
    {
      var i = 1;
      while ((i <= q))
      {
        var T: dynamic;
        var K: dynamic;
        var D: dynamic;
        scanf("%d%d%d", (&T), (&K), (&D));
        var ans = 0;
        var cnt = 0;
        {
          var i = 1;
          while ((i <= n))
          {
            if (((en[i] == -1) || (en[i] < T)))
            {
              cnt += 1;
              e[i] = ((T + D) - 1);
              ans += i;
              if ((cnt == K))
              {
                break;
              }
            }
            i += 1;
          }
        }
        if ((cnt == K))
        {
          {
            var i = 1;
            while ((i <= n))
            {
              en[i] = e[i];
              i += 1;
            }
          }
          printf("%d\n", ans);
        } else
        {
          {
            var i = 1;
            while ((i <= n))
            {
              e[i] = en[i];
              i += 1;
            }
          }
          printf("-1\n");
        }
        i += 1;
      }
    }
  }
  return 0;
}
