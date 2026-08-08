// Translated from solution.cpp.

func main()
{
  var x: dynamic;
  var y = cpp_array(1010);
  var n: dynamic;
  var ans: dynamic;
  var all = 0;
  read(n);
  {
    var i = 1;
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
