// Translated from solution.cpp.

var n: dynamic;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var e: dynamic;

func main()
{
  scanf("%lld%lld%lld%lld%lld", (&n), (&a), (&b), (&c), (&d));
  n -= 1;
  {
    var i = 0;
    while ((i <= n))
    {
      if (((((c * i) - (d * ((n - i)))) <= (a - b)) && ((a - b) <= ((d * i) - (c * ((n - i)))))))
      {
        e = 1;
      }
      i += 1;
    }
  }
  if (e)
  {
    printf("YES\n");
  } else
  {
    printf("NO\n");
  }
}
