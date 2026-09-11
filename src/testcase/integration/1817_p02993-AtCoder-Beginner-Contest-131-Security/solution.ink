// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(s);
  {
    var i: dynamic = 1;
    while ((i < s.size()))
    {
      if ((s[i] == s[(i - 1)]))
      {
        write("Bad");
        exit(0);
      }
      i += 1;
    }
  }
  write("Good");
  return 0;
}
