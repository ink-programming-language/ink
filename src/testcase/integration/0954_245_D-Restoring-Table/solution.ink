// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func EXEC() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var s: dynamic = 0;
      var t: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
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

func main() -> dynamic
{
  EXEC();
  return 0;
}
