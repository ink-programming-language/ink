// Translated from solution.cpp.

var s: dynamic = cpp_array(110);

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  {
    var i: dynamic = 1;
    while ((i <= b))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
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
    var i: dynamic = 1;
    while ((i <= a))
    {
      write(s[i], cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
