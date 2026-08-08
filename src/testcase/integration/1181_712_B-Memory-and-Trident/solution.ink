// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  read(s);
  var n = s.length();
  if ((n & 1))
  {
    write(-1);
    return 0;
  }
  var a = 0;
  var b = 0;
  {
    var i = 0;
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
  var min_val = min(a, b);
  var max_val = max(a, b);
  var ans = ((((max_val - min_val)) / 2) + min_val);
  write(ans);
}
