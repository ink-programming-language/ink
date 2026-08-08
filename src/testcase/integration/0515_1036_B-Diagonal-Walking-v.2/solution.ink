// Translated from solution.cpp.

func main()
{
  var q: dynamic;
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var i: dynamic;
  var bar: dynamic;
  var left: dynamic;
  var diag: dynamic;
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    scanf("%lld %lld %lld", (&n), (&m), (&k));
    if ((n < m))
    {
      i = n;
      n = m;
      m = i;
    }
    if ((n > k))
    {
      printf("-1\n");
      continue;
    } else
    {
      bar = (n - m);
      if (((bar % 2) == 1))
      {
        diag = (k - 1);
      } else
      {
        diag = (n - 1);
        left = ((k - n) + 1);
        if (((left % 2) == 0))
        {
          diag += (left - 2);
        } else
        {
          diag += left;
        }
      }
      printf("%lld\n", diag);
    }
  }
  return 0;
}
