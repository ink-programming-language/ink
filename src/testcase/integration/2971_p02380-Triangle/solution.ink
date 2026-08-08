// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var cr: dynamic;
  var S: dynamic;
  var L: dynamic;
  var H: dynamic;
  read(a, b, c);
  cr = (((acos(-1.0) * c)) / 180.0);
  S = (((0.5 * a) * b) * sin(cr));
  L = ((a + b) + sqrt((((a * a) + (b * b)) - (((2 * a) * b) * cos(cr)))));
  H = (b * sin(cr));
  printf("%lf\n%lf\n%lf\n", S, L, H);
  return 0;
}
