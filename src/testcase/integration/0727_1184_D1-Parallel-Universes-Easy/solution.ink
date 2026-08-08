// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var m: dynamic;
  var t: dynamic;
  var i: dynamic;
  scanf("%d%d%d%d", (&n), (&k), (&m), (&t));
  var a: dynamic;
  var b: dynamic;
  {
    i = 0;
    while ((i < t))
    {
      scanf("%d %d", (&a), (&b));
      if ((a == 0))
      {
        if ((b < k))
        {
          n -= b;
          k -= b;
        } else
        {
          n = b;
        }
      }
      if ((a == 1))
      {
        if ((b <= k))
        {
          n += 1;
          k += 1;
        } else
        {
          n += 1;
        }
      }
      printf("%d %d\n", n, k);
      i += 1;
    }
  }
  return 0;
}
