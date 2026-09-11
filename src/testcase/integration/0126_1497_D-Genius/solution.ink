// Translated from solution.cpp.

func absi(i: dynamic) -> dynamic
{
  if ((i > 0))
  {
    return i;
  } else
  {
    return (-i);
  }
}

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var ans: dynamic = 0;
    scanf("%d", (&n));
    var tag: dynamic = cpp_construct((n + 1));
    var s: dynamic = cpp_construct((n + 1));
    var d: dynamic = cpp_construct((n + 1), 0);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%d", (&tag[i]));
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%d", (&s[i]));
        i += 1;
      }
    }
    {
      var t: dynamic = 2;
      while ((t <= n))
      {
        {
          var i: dynamic = (t - 1);
          while ((i >= 1))
          {
            if ((tag[t] != tag[i]))
            {
              var di: dynamic = d[i];
              var dt: dynamic = d[t];
              var ss: dynamic = absi((s[i] - s[t]));
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
