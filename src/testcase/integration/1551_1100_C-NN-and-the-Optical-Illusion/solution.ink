// Translated from solution.cpp.

var INF = 999999999999999999;

var PI = acos(-1.0);

func stop()
{
  exit(0);
}

func main()
{
  var n: dynamic;
  var r: dynamic;
  read(n, r);
  var sn = sin((PI / n));
  var x = ((((2.0 * r) * sn)) / ((2.0 - (2.0 * sn))));
  printf("%.9lf", x);
  stop();
}
