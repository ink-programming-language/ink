// Translated from solution.cpp.

func main() -> dynamic
{
  var ans: dynamic = 0;
  var sum: dynamic = 0;
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&x), (&d));
  sum = (((x * 2) * n) + (((1.0 * n) * (((2 * n) - 1))) * d));
  while (n)
  {
    ans += ((sum * 0.5) / n);
    sum -= ((sum / n) / n);
    n -= 1;
  }
  printf("%.10lf\n", ans);
  return 0;
}
