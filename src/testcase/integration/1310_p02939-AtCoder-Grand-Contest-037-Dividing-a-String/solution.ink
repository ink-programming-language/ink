// Translated from solution.cpp.

var s: dynamic;

var s1: dynamic;

var s2: dynamic;

var cnt: dynamic;

func main()
{
  read(s);
  {
    var i = 0;
    while ((i < s.length()))
    {
      s2 += s[i];
      if ((s2 != s1))
      {
        cnt += 1;
        s1 = s2;
        s2.erase(0, s2.length());
      }
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}
