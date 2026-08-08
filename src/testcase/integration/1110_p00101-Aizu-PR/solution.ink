// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var len: dynamic;
  var a: dynamic;
  read(n);
  getline(cin, a);
  {
    var i = 0;
    while ((i < n))
    {
      getline(cin, a);
      len = a.size();
      {
        var o = 0;
        while ((o <= len))
        {
          if ((a[o] == cpp_char("H")))
          {
            if ((a[(o + 1)] == cpp_char("o")))
            {
              if ((a[(o + 2)] == cpp_char("s")))
              {
                if ((a[(o + 3)] == cpp_char("h")))
                {
                  if ((a[(o + 4)] == cpp_char("i")))
                  {
                    if ((a[(o + 5)] == cpp_char("n")))
                    {
                      if ((a[(o + 6)] == cpp_char("o")))
                      {
                        a[(o + 6)] = cpp_char("a");
                      }
                    }
                  }
                }
              }
            }
          }
          o += 1;
        }
      }
      write(a, "\n");
      i += 1;
    }
  }
  return 0;
}
