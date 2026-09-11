// Translated from solution.cpp.

var dp: dynamic = cpp_array(600009);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  var now: dynamic = cpp_uninitialized();
  var back: dynamic = cpp_uninitialized();
  var day: dynamic = cpp_uninitialized();
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
