// Translated from solution.cpp.

var x: dynamic;

var y: dynamic;

var a: dynamic;

var b: dynamic;

var n: dynamic;

func init()
{
  scanf("%d", (&n));
  puts("YES");
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d%d%d", (&x), (&y), (&a), (&b));
      printf("%d\n", (((2 * abs((x % 2))) + abs((y % 2))) + 1));
      i += 1;
    }
  }
}

func main()
{
  init();
  return 0;
}
