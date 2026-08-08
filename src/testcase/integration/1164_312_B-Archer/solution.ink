// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var ans = 0;

func main()
{
  read(a, b, c, d);
  ans = (((a / b)) / ((1 - (((1 - (a / b))) * ((1 - (c / d)))))));
  printf("%.12lf\n", ans);
}
