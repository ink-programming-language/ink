// Translated from solution.cpp.

var n: dynamic;

func EXEC()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      var s = 0;
      var t: dynamic;
      {
        var j = 0;
        while ((j < n))
        {
          read(t);
          if ((t != -1))
          {
            s |= t;
          }
          j += 1;
        }
      }
      write(s, cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func main()
{
  EXEC();
  return 0;
}
