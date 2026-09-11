// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ma: dynamic = cpp_uninitialized();
  read(n, ma);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      if ((ma > x))
      {
        ans += (ma - x);
      }
      ma = max(ma, x);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
