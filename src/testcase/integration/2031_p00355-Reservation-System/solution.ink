// Translated from solution.cpp.

func main() -> dynamic
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(l, r, n);
  var d: dynamic = cpp_array(1111);
  memset(d, 0, cpp_sizeof((d)));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      {
        var j: dynamic = x;
        while ((j < y))
        {
          d[j] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = l;
    while ((i < r))
    {
      if (d[i])
      {
        write(1, "\n");
        return 0;
      }
      i += 1;
    }
  }
  write(0, "\n");
  return 0;
}
