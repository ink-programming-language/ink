// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(110);

var tmp: dynamic = cpp_array(110);

var vis: dynamic = cpp_array(110);

var num: dynamic = cpp_array(110);

func doit() -> dynamic
{
  var dp: dynamic = 1;
  var cp: dynamic = l;
  var tp: dynamic = cpp_uninitialized();
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

func main() -> dynamic
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
      var i: dynamic = 0;
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
