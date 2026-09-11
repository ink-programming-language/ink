// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var pos: dynamic = 0;
  var neg: dynamic = 0;
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      pos += ((a[i] > 0));
      neg += ((a[i] < 0));
      i += 1;
    }
  }
  var lim: dynamic = (((n + 1)) / 2);
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
