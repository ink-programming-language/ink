// Translated from solution.cpp.

func absi(i: dynamic)
{
  if ((i > 0))
  {
    return i;
  } else
  {
    return (-i);
  }
}

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var n: dynamic;
    var ans = 0;
    scanf("%d", (&n));
    var tag = cpp_construct((n + 1));
    var s = cpp_construct((n + 1));
    var d = cpp_construct((n + 1), 0);
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%d", (&tag[i]));
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%d", (&s[i]));
        i += 1;
      }
    }
    {
      var t = 2;
      while ((t <= n))
      {
        {
          var i = (t - 1);
          while ((i >= 1))
          {
            if ((tag[t] != tag[i]))
            {
              var di = d[i];
              var dt = d[t];
              var ss = absi((s[i] - s[t]));
              d[t] = max((di + ss), dt);
              d[i] = max((dt + ss), di);
              ans = max(ans, d[i]);
              ans = max(ans, d[t]);
            }
            i -= 1;
          }
        }
        t += 1;
      }
    }
    printf("%lld\n", ans);
  }
  return 0;
}
