// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c = 0;

func main()
{
  scanf("%d%d", (&a), (&b));
  while (b)
  {
    c = ((c * 10) + (b % 10));
    b /= 10;
  }
  printf("%d\n", (a + c));
}
