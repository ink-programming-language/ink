// Translated from solution.cpp.

func main()
{
  var str: dynamic;
  var code = [[cpp_char("a"), cpp_char("b"), cpp_char("c"), cpp_char("d"), cpp_char("e")], [cpp_char("f"), cpp_char("g"), cpp_char("h"), cpp_char("i"), cpp_char("j")], [cpp_char("k"), cpp_char("l"), cpp_char("m"), cpp_char("n"), cpp_char("o")], [cpp_char("p"), cpp_char("q"), cpp_char("r"), cpp_char("s"), cpp_char("t")], [cpp_char("u"), cpp_char("v"), cpp_char("w"), cpp_char("x"), cpp_char("y")], [cpp_char("z"), cpp_char("."), cpp_char("?"), cpp_char("!"), cpp_char(" ")]];
  while ((cin >> str))
  {
    if ((str.size() % 2))
    {
      write("NA\n");
      continue;
    }
    var ans = "";
    {
      var i = 0;
      while ((i < str.size()))
      {
        if (((((str[i] < cpp_char("1")) || (cpp_char("6") < str[i])) || (str[(i + 1)] < cpp_char("1"))) || (cpp_char("5") < str[(i + 1)])))
        {
          ans = "NA";
          break;
        }
        ans += code[(str[i] - cpp_char("1"))][(str[(i + 1)] - cpp_char("1"))];
        i += 2;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
