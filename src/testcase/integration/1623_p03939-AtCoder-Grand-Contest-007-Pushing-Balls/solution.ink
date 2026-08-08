// Translated from solution.cpp.

func main()
{
  var ans = 0;
  var sum = 0;
  var n: dynamic;
  var x: dynamic;
  var d: dynamic;
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
