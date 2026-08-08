// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var pos = 0;
  var neg = 0;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      pos += ((a[i] > 0));
      neg += ((a[i] < 0));
      i += 1;
    }
  }
  var lim = (((n + 1)) / 2);
  if ((pos >= lim))
  {
    write("1", "\n");
  } else if ((neg >= lim))
  {
    write("-1", "\n");
  } else
  {
    write("0", "\n");
  }
  return 0;
}
