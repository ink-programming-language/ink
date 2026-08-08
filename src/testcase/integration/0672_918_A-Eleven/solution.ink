// Translated from solution.cpp.

var n: dynamic;

var s: dynamic;

func init()
{
  var x = 1;
  var y = 1;
  s.insert(1);
  {
    var z = 2;
    while ((z <= 1000))
    {
      s.insert(z);
      x = y;
      y = z;
      z = (x + y);
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  init();
  read(n);
  {
    var i = 1;
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
