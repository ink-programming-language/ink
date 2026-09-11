// Translated from solution.cpp.

var maxN: dynamic = ((100 * 1000) + 100);

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(maxN);

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(s);
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      t[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      a = max(a, ((s[i] - cpp_char("0"))));
      i += 1;
    }
  }
  write(a, "\n");
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      while ((t[i] > 0))
      {
        b = 0;
        {
          var j: dynamic = 0;
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
