// Translated from solution.cpp.

func main()
{
  var t = 0;
  var s: dynamic;
  var a: dynamic;
  read(s);
  a = s[0];
  {
    var i = 1;
    while ((i < s.size()))
    {
      if ((s[i] == s[(i - 1)]))
      {
        t += 1;
        if ((s[(i - 1)] == cpp_char("1")))
        {
          s[i] = cpp_char("0");
        } else if ((s[(i - 1)] == cpp_char("0")))
        {
          s[i] = cpp_char("1");
        }
      }
      i += 1;
    }
  }
  write(t, "\n");
  return 0;
}
