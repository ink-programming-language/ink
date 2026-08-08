// Translated from solution.cpp.

var r = cpp_array(105);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var right = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      if (((a <= right) && (b > right)))
      {
        right = b;
      }
      i += 1;
    }
  }
  if ((right == m))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
