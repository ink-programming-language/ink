// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var ans: dynamic;
  scanf("%d", (&n));
  if (((n % 2) == 0))
  {
    ans = (((floor((n / 2)) + 1)) * ceil((n / 2)));
  } else
  {
    ans = (((floor((n / 2)) + 1)) * (((n / 2) + 1)));
  }
  printf("%d\n", ans);
  return 0;
}
