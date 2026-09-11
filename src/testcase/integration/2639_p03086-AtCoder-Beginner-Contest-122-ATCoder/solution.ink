// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var i: dynamic = cpp_uninitialized();
  var p: dynamic = 0;
  var m: dynamic = 0;
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
