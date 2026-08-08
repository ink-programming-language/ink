// Translated from solution.cpp.

var s: dynamic;

var c: dynamic;

var f: dynamic;

func main()
{
  read(s);
  {
    var i = 0;
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
