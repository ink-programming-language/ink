// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var result: dynamic;
  scanf("%d %d", (&n), (&k));
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      scanf("%d", (&a));
      if (((k % a) == 0))
      {
        result = min(result, (k / a));
      }
      i += 1;
    }
  }
  printf("%d\n", result);
  return 0;
}
