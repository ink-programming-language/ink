// Translated from solution.cpp.

var maxN = ((100 * 1000) + 100);

var a: dynamic;

var b: dynamic;

var c: dynamic;

var t = cpp_array(maxN);

var s: dynamic;

func main()
{
  read(s);
  {
    var i = 0;
    while ((i < s.size()))
    {
      t[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < s.size()))
    {
      a = max(a, ((s[i] - cpp_char("0"))));
      i += 1;
    }
  }
  write(a, "\n");
  {
    var i = 0;
    while ((i < s.size()))
    {
      while ((t[i] > 0))
      {
        b = 0;
        {
          var j = 0;
          while ((j < s.size()))
          {
            if ((t[j] > 0))
            {
              b = (((b * 10)) + 1);
            } else
            {
              b = (((b * 10)) + 0);
            }
            t[j] -= 1;
            j += 1;
          }
        }
        write(b, " ");
      }
      i += 1;
    }
  }
  write("\n");
  return 0;
}
