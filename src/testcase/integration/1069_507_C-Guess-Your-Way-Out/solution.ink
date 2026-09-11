// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%lld", (&n), (&m));
  m -= 1;
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      var ka: dynamic = ((((m >> i)) & 1));
      var kb: dynamic = ((((m >> ((i + 1)))) & 1));
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
