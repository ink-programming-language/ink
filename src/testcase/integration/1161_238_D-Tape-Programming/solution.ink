// Translated from solution.cpp.

var n: dynamic;

var q: dynamic;

var l: dynamic;

var r: dynamic;

var s = cpp_array(110);

var tmp = cpp_array(110);

var vis = cpp_array(110);

var num = cpp_array(110);

func doit()
{
  var dp = 1;
  var cp = l;
  var tp: dynamic;
  while (((cp >= l) && (cp <= r)))
  {
    if (((tmp[cp] == cpp_char("<")) || (tmp[cp] == cpp_char(">"))))
    {
      if ((tmp[cp] == cpp_char("<")))
      {
        dp = -1;
        tp = (cp - 1);
        while (((tp >= l) && vis[tp]))
        {
          tp -= 1;
        }
        if ((tp < l))
        {
          break;
        }
        if (((tmp[tp] == cpp_char("<")) || (tmp[tp] == cpp_char(">"))))
        {
          vis[cp] = true;
        }
        cp = tp;
        continue;
      } else
      {
        dp = 1;
        tp = (cp + 1);
        while (((tp <= r) && vis[tp]))
        {
          tp += 1;
        }
        if ((tp > r))
        {
          break;
        }
        if (((tmp[tp] == cpp_char("<")) || (tmp[tp] == cpp_char(">"))))
        {
          vis[cp] = true;
        }
        cp = tp;
        continue;
      }
    } else
    {
      if ((tmp[cp] == cpp_char("0")))
      {
        num[0] += 1;
        vis[cp] = true;
        cp += dp;
        while ((((cp >= l) && (cp <= r)) && vis[cp]))
        {
          cp += dp;
        }
      } else
      {
        num[(tmp[cp] - cpp_char("0"))] += 1;
        tmp[cp] -= 1;
        cp += dp;
        while ((((cp >= l) && (cp <= r)) && vis[cp]))
        {
          cp += dp;
        }
      }
    }
  }
}

func main()
{
  scanf("%d%d", (&n), (&q));
  scanf("%s", (s + 1));
  while (cpp_update(q, "--"))
  {
    scanf("%d%d", (&l), (&r));
    memset(num, 0, cpp_sizeof((num)));
    memcpy(tmp, s, cpp_sizeof((s)));
    memset(vis, 0, cpp_sizeof((vis)));
    doit();
    {
      var i = 0;
      while ((i < 9))
      {
        printf("%d ", num[i]);
        i += 1;
      }
    }
    printf("%d\n", num[9]);
  }
  return 0;
}
