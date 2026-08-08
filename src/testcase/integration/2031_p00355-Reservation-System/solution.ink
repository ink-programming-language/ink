// Translated from solution.cpp.

func main()
{
  var l: dynamic;
  var r: dynamic;
  var n: dynamic;
  read(l, r, n);
  var d = cpp_array(1111);
  memset(d, 0, cpp_sizeof((d)));
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      {
        var j = x;
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
    var i = l;
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
