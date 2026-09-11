// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_array(4);
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      read(s[i]);
      i += 1;
    }
  }
  sort(s, (s + 4));
  if ((((s[0] == s[1]) && (s[2] == s[3])) && (s[1] != s[2])))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
}
