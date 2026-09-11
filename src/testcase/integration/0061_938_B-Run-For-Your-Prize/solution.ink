// Translated from solution.cpp.

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  scanf("%d", (&k));
  var maxn: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      if ((x >= 500001))
      {
        maxn = max(maxn, (1000000 - x));
      }
      if ((x <= 500000))
      {
        maxn = max(maxn, (x - 1));
      }
      i += 1;
    }
  }
  printf("%d\n", maxn);
}
