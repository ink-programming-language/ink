// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  read(s);
  var i: dynamic;
  var p = 0;
  var m = 0;
  {
    i = 0;
    while ((i < s.size()))
    {
      if (((((s[i] == cpp_char("A")) || (s[i] == cpp_char("T"))) || (s[i] == cpp_char("C"))) || (s[i] == cpp_char("G"))))
      {
        p += 1;
        if ((m < p))
        {
          m = p;
        }
      } else
      {
        p = 0;
      }
      i += 1;
    }
  }
  write(m, "\n");
  return 0;
}
