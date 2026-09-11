// Translated from solution.cpp.

var CMAX: dynamic = 100005;

var Cfd: dynamic = "";

var Cfr: dynamic = "";

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  {
    var i: dynamic = 0;
    while ((i < s.length()))
    {
      if ((s[i] == cpp_char("*")))
      {
        {
          var j: dynamic = 1;
          while (((i + (4 * j)) < s.length()))
          {
            if (((((s[(i + j)] == cpp_char("*")) && (s[(i + (2 * j))] == cpp_char("*"))) && (s[(i + (3 * j))] == cpp_char("*"))) && (s[(i + (4 * j))] == cpp_char("*"))))
            {
              write("yes");
              return 0;
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  write("no");
  return 0;
}
