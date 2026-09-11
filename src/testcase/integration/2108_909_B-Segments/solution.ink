// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
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
