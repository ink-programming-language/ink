// Translated from solution.cpp.

func main() -> dynamic
{
  var arr: dynamic = cpp_array(101);
  var n: dynamic = cpp_uninitialized();
  var imin: dynamic = 0;
  var imax: dynamic = 0;
  read(n);
  {
    var i: dynamic = 1;
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
  var a: dynamic = cpp_uninitialized();
  if ((imax < imin))
  {
    swap(imax, imin);
  }
  var rsp: dynamic = (abs((imin - imax)) + max((n - imax), (imin - 1)));
  write(rsp);
  return 0;
}
