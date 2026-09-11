// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = 0;
  var s: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  read(s);
  a = s[0];
  {
    var i: dynamic = 1;
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
