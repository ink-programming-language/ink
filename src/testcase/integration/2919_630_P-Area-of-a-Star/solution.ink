// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var r: dynamic;
  var s: dynamic;
  var ans: dynamic;
  read(n, r);
  var a = ((atan(1) * 2) / n);
  ans = (((((r * r) * n) * sin((2 * a))) * sin(a)) / sin(((((n * 2) - 3)) * a)));
  printf("%.10f\n", ans);
  return 0;
}
