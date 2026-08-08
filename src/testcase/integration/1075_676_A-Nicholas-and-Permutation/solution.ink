// Translated from solution.cpp.

func main()
{
  var arr = cpp_array(101);
  var n: dynamic;
  var imin = 0;
  var imax = 0;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(arr[i]);
      if ((arr[i] == 1))
      {
        imin = i;
      } else if ((arr[i] == n))
      {
        imax = i;
      }
      i += 1;
    }
  }
  var a: dynamic;
  if ((imax < imin))
  {
    swap(imax, imin);
  }
  var rsp = (abs((imin - imax)) + max((n - imax), (imin - 1)));
  write(rsp);
  return 0;
}
