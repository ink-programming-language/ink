// Translated from solution.cpp.

func main()
{
  var k: dynamic;
  scanf("%d", (&k));
  var maxn = 0;
  {
    var i = 1;
    while ((i <= k))
    {
      var x: dynamic;
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
