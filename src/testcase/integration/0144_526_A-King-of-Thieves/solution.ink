// Translated from solution.cpp.

var CMAX = 100005;

var Cfd = "";

var Cfr = "";

func main()
{
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  {
    var i = 0;
    while ((i < s.length()))
    {
      if ((s[i] == cpp_char("*")))
      {
        {
          var j = 1;
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
