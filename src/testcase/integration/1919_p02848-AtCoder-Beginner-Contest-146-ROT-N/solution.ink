// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var i: dynamic;
  var s: dynamic;
  var a: dynamic;
  read(n, s);
  a = n;
  {
    i = 0;
    while ((i < s.size()))
    {
      s[i] += a;
      if ((s[i] > cpp_char("Z")))
      {
        s[i] -= 26;
      }
      i += 1;
    }
  }
  write(s);
  return 0;
}
