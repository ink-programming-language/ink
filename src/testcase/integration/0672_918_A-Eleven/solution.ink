// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  var x: dynamic = 1;
  var y: dynamic = 1;
  s.insert(1);
  {
    var z: dynamic = 2;
    while ((z <= 1000))
    {
      s.insert(z);
      x = y;
      y = z;
      z = (x + y);
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  init();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((s.find(i) != s.end()))
      {
        write(cpp_char("O"));
      } else
      {
        write(cpp_char("o"));
      }
      i += 1;
    }
  }
  return 0;
}
