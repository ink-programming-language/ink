// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f: dynamic;
  var x: dynamic;
  var y: dynamic;
  while (((((((cin >> a) >> b) >> c) >> d) >> e) >> f))
  {
    y = ((((c * d) - (a * f))) / (((b * d) - (a * e))));
    x = (((c - (b * y))) / a);
    printf("%.3f %.3f\n", x, y);
  }
  return 0;
}
