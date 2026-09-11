// Translated from solution.cpp.

func main() -> dynamic
{
  var width: dynamic = 0;
  var n: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  read(n, h);
  var arr: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      if ((arr[i] <= h))
      {
        width += 1;
      } else
      {
        width += 2;
      }
      i += 1;
    }
  }
  write(width, "\n");
}
