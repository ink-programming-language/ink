// Translated from solution.cpp.

func main()
{
  var q: dynamic;
  var s: dynamic;
  var arr = [cpp_char("q"), cpp_char("w"), cpp_char("e"), cpp_char("r"), cpp_char("t"), cpp_char("y"), cpp_char("u"), cpp_char("i"), cpp_char("o"), cpp_char("p"), cpp_char("a"), cpp_char("s"), cpp_char("d"), cpp_char("f"), cpp_char("g"), cpp_char("h"), cpp_char("j"), cpp_char("k"), cpp_char("l"), cpp_char(";"), cpp_char("z"), cpp_char("x"), cpp_char("c"), cpp_char("v"), cpp_char("b"), cpp_char("n"), cpp_char("m"), cpp_char(","), cpp_char("."), cpp_char("/")];
  read(q, s);
  var k: dynamic;
  var l: dynamic;
  {
    var i = 0;
    while ((i < s.length()))
    {
      {
        k = 0;
        while ((k < 3))
        {
          {
            l = 0;
            while ((l < 10))
            {
              if ((s[i] == arr[k][l]))
              {
                if ((q == "R"))
                {
                  write(arr[k][(l - 1)]);
                } else
                {
                  write(arr[k][(l + 1)]);
                }
              }
              l += 1;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  puts("");
}
