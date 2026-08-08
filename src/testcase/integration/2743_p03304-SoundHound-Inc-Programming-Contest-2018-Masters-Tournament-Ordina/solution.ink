// Translated from solution.cpp.

var ld = dynamic;

var n: dynamic;

var m: dynamic;

var d: dynamic;

func main()
{
  read(n, m, d);
  var a = 2.0;
  if ((!d))
  {
    a = 1.0;
  }
  printf("%.7Lf", ((((a * ((n - d))) * ((m - 1)))) / ((n * n))));
  return 0;
}
