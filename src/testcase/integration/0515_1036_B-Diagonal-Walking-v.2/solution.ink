// Translated from solution.cpp.

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var bar: dynamic = cpp_uninitialized();
  var left: dynamic = cpp_uninitialized();
  var diag: dynamic = cpp_uninitialized();
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
