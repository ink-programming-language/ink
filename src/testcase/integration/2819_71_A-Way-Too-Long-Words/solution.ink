// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  read(s);
  while ((cin >> s))
  {
    if ((s.length() < 11))
    {
      write(s, "\n");
    } else
    {
      write(s[0], (s.length() - 2), s[(s.length() - 1)], "\n");
    }
  }
}
