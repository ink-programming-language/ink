// Translated from solution.cpp.

var dp = cpp_array(600009);

func main()
{
  var n: dynamic;
  var d: dynamic;
  var i: dynamic;
  var j: dynamic;
  var sum: dynamic;
  var tmp: dynamic;
  var now: dynamic;
  var back: dynamic;
  var day: dynamic;
  while ((scanf("%d %d", (&n), (&d)) != EOF))
  {
    memset(dp, 0, cpp_sizeof((dp)));
    dp[0] = 1;
    sum = 0;
    {
      i = 1;
      while ((i <= n))
      {
        scanf("%d", (&tmp));
        sum += tmp;
        {
          j = sum;
          while ((j >= tmp))
          {
            if ((dp[(j - tmp)] == 1))
            {
              dp[j] = 1;
            }
            j -= 1;
          }
        }
        i += 1;
      }
    }
    now = 0;
    back = 0;
    day = 0;
    while (1)
    {
      now += d;
      back = d;
      {
        i = 0;
        while ((i < d))
        {
          if ((dp[(now - i)] == 1))
          {
            back = i;
            break;
          }
          i += 1;
        }
      }
      if ((back == d))
      {
        now -= back;
        break;
      }
      now -= back;
      day += 1;
    }
    printf("%d %d\n", now, day);
  }
}
