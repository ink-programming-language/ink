// Translated from solution.cpp.

func main()
{
  var cn1: dynamic;
  var cn2: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(cn1, cn2, x, y);
  var lo = 1;
  var hi = 0x7FFFFFFF;
  var mid: dynamic;
  while ((lo < hi))
  {
    mid = (lo + (((hi - lo)) / 2));
    var rex = (mid - ((mid / x)));
    var rey = (mid - ((mid / y)));
    var total = (mid - ((mid / ((x * y)))));
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
