// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var x3: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var y3: dynamic = cpp_uninitialized();
  var ox: dynamic = cpp_uninitialized();
  var oy: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lf %lf %lf %lf %lf %lf", (&x1), (&y1), (&x2), (&y2), (&x3), (&y3));
      ox = ((((((y2 - y1)) * (((((y3 * y3) - (y1 * y1))) + (((x3 * x3) - (x1 * x1)))))) - (((y3 - y1)) * (((((y2 * y2) - (y1 * y1))) + (((x2 * x2) - (x1 * x1)))))))) / ((2 * (((((x3 - x1)) * ((y2 - y1))) - (((x2 - x1)) * ((y3 - y1))))))));
      if ((y2 != y1))
      {
        oy = ((((((2 * ((x1 - x2))) * ox) + (((y2 * y2) - (y1 * y1)))) + (((x2 * x2) - (x1 * x1))))) / ((2 * ((y2 - y1)))));
      } else
      {
        oy = ((((((2 * ((x1 - x3))) * ox) + (((y3 * y3) - (y1 * y1)))) + (((x3 * x3) - (x1 * x1))))) / ((2 * ((y3 - y1)))));
      }
      r = sqrt(((((ox - x1)) * ((ox - x1))) + (((oy - y1)) * ((oy - y1)))));
      printf("%.3lf %.3lf %.3lf\n", ox, oy, r);
      i += 1;
    }
  }
  return 0;
}
