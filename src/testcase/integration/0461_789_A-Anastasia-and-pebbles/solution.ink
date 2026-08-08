// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var t: dynamic;
  var icount: dynamic;
  while ((scanf("%d%d", (&n), (&k)) != EOF))
  {
    icount = 0;
    while (((cpp_update(n, "--")) > 0))
    {
      scanf("%d", (&t));
      icount += ((((t + k) - 1)) / k);
    }
    printf("%d\n", (((icount + 1)) / 2));
  }
  return 0;
}
