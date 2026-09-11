// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(100009);

func main() -> dynamic
{
  read(s);
  var depth: dynamic = 0;
  c[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s[i] == cpp_char("(")))
      {
        depth += 1;
      }
      if ((s[i] == cpp_char(")")))
      {
        depth -= 1;
      }
      c[depth] += 1;
      i += 1;
    }
  }
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= s.size()))
    {
      ret += (((1 * c[i]) * ((c[i] - 1))) / 2);
      i += 1;
    }
  }
  write((ret - 1), "\n");
  return 0;
}
