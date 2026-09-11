// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var f: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(s);
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s[i] == cpp_char("S")))
      {
        f += 1;
      } else if ((f > 0))
      {
        f -= 1;
        c += 2;
      }
      i += 1;
    }
  }
  write((s.size() - c), "\n");
}
