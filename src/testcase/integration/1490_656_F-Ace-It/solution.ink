// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  var i: dynamic;
  var l: dynamic;
  var z = 0;
  read(s);
  l = s.size();
  {
    i = 0;
    while ((i < l))
    {
      if ((s[i] == cpp_char("A")))
      {
        z += 1;
      } else if ((s[i] == cpp_char("1")))
      {
        z += 10;
      } else
      {
        z += (s[i] - cpp_char("0"));
      }
      i += 1;
    }
  }
  write(z);
  return 0;
}
