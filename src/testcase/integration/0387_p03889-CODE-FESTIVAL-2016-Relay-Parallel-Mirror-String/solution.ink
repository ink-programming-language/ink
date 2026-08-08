// Translated from solution.cpp.

var ch = cpp_array(256);

func main()
{
  ch[cpp_char("b")] = cpp_char("d");
  ch[cpp_char("d")] = cpp_char("b");
  ch[cpp_char("q")] = cpp_char("p");
  ch[cpp_char("p")] = cpp_char("q");
  var s: dynamic;
  read(s);
  var i: dynamic;
  {
    i = 0;
    while ((i < s.size()))
    {
      if ((ch[s[i]] != s[((s.size() - 1) - i)]))
      {
        cpp_goto("goto br;");
      }
      i += 1;
    }
  }
  printf("Yes\n");
  return 0;
  printf("No\n");
  return 0;
}
