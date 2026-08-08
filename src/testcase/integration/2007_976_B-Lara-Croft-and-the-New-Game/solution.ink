// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var d: dynamic;
  var fd: dynamic;
  scanf("%lld %lld %lld", (&n), (&m), (&k));
  if ((k < n))
  {
    printf("%lld 1", (k + 1));
  } else if ((k < ((n + m) - 1)))
  {
    printf("%lld %lld", n, ((1 + k) - ((n - 1))));
  } else
  {
    k -= (((n - 1)) + ((m - 1)));
    d = (k / ((m - 1)));
    fd = 0;
    if (((k % ((m - 1))) == 0))
    {
      d -= 1;
      fd = 1;
    }
    printf("%lld ", ((n - 1) - d));
    if ((d & 1))
    {
      if ((!fd))
      {
        printf("%lld", (1 + (k % ((m - 1)))));
      } else
      {
        printf("%lld", m);
      }
    } else
    {
      if ((!fd))
      {
        printf("%lld", (m - (((k % ((m - 1))) - 1))));
      } else
      {
        printf("%lld", 2);
      }
    }
  }
  return 0;
}
