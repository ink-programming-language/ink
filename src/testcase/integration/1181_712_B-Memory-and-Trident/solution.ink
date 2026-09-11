// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var n: dynamic = s.length();
  if ((n & 1))
  {
    write(-1);
    return 0;
  }
  var a: dynamic = 0;
  var b: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < s.length()))
    {
      if ((s[i] == cpp_char("U")))
      {
        a += 1;
      }
      if ((s[i] == cpp_char("D")))
      {
        a -= 1;
      }
      if ((s[i] == cpp_char("L")))
      {
        b += 1;
      }
      if ((s[i] == cpp_char("R")))
      {
        b -= 1;
      }
      i += 1;
    }
  }
  a = abs(a);
  b = abs(b);
  var min_val: dynamic = min(a, b);
  var max_val: dynamic = max(a, b);
  var ans: dynamic = ((((max_val - min_val)) / 2) + min_val);
  write(ans);
}
