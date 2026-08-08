// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  n -= 43500;
  puts(if ((n > 0)) (if ((n > 2000)) "3" else "2") else "1");
  return 0;
}
