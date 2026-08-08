// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  read(s);
  {
    var i = 0;
    while ((i < s.size()))
    {
      if ((((i % 2) == 0) && (s[i] == cpp_char("L"))))
      {
        write("No", "\n");
        return 0;
      }
      if ((((i % 2) == 1) && (s[i] == cpp_char("R"))))
      {
        write("No", "\n");
        return 0;
      }
      i += 1;
    }
  }
  write("Yes", "\n");
}
