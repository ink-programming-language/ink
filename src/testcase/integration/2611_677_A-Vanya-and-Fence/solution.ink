// Translated from solution.cpp.

func main()
{
  var width = 0;
  var n: dynamic;
  var h: dynamic;
  read(n, h);
  var arr = cpp_array(n);
  {
    var i = 0;
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
