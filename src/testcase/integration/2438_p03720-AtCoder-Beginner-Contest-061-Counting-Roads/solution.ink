// Translated from solution.cpp.

var s = cpp_array(110);

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  {
    var i = 1;
    while ((i <= b))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      if ((x > y))
      {
        swap(x, y);
      }
      s[x] += 1;
      s[y] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= a))
    {
      write(s[i], cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
