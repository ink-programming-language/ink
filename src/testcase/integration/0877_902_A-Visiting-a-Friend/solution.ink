// Translated from solution.cpp.

var r: dynamic = cpp_array(105);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var right: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
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
