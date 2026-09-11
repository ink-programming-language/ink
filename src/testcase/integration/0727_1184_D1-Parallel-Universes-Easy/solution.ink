// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  scanf("%d%d%d%d", (&n), (&k), (&m), (&t));
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
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
