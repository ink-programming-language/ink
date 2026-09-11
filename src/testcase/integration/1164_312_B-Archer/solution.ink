// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var ans: dynamic = 0;

func main() -> dynamic
{
  read(a, b, c, d);
  ans = (((a / b)) / ((1 - (((1 - (a / b))) * ((1 - (c / d)))))));
  printf("%.12lf\n", ans);
}
