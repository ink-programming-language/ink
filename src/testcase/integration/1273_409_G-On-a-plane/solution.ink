// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_array(1010);
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var all: dynamic = 0;
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(x, y[i]);
      all += y[i];
      i += 1;
    }
  }
  ans = (5 + ((all * 1.0) / n));
  printf("%.3lf", ans);
  return 0;
}
