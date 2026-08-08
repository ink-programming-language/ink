// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  while (cpp_update(n, "--"))
  {
    var s: dynamic;
    read(s);
    var i = 0;
    var count = 0;
    while ((s[i] != cpp_char("\u{0}")))
    {
      count += 1;
      i += 1;
    }
    if (((s[(count - 2)] == cpp_char("s")) && (s[(count - 1)] == cpp_char("u"))))
    {
      write("JAPANESE", "\n");
    }
    if (((s[(count - 2)] == cpp_char("p")) && (s[(count - 1)] == cpp_char("o"))))
    {
      write("FILIPINO", "\n");
    }
    if (((s[(count - 2)] == cpp_char("d")) && (s[(count - 1)] == cpp_char("a"))))
    {
      write("KOREAN", "\n");
    }
  }
}
