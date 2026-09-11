// Translated from solution.cpp.

func main() -> dynamic
{
  var cn1: dynamic = cpp_uninitialized();
  var cn2: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(cn1, cn2, x, y);
  var lo: dynamic = 1;
  var hi: dynamic = 0x7FFFFFFF;
  var mid: dynamic = cpp_uninitialized();
  while ((lo < hi))
  {
    mid = (lo + (((hi - lo)) / 2));
    var rex: dynamic = (mid - ((mid / x)));
    var rey: dynamic = (mid - ((mid / y)));
    var total: dynamic = (mid - ((mid / ((x * y)))));
    if ((((cn1 <= rex) && (cn2 <= rey)) && (((cn1 + cn2)) <= total)))
    {
      hi = mid;
    } else
    {
      lo = (mid + 1);
    }
  }
  write(hi, "\n");
  return 0;
}
