// Translated from solution.cpp.

func main()
{
  var s1: dynamic;
  var s2: dynamic;
  read(s1, s2);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      if ((i == n)) (((cout << s1) << " ") << s2) else ((((cout << s1) << " ") << s2) << endl);
      var f: dynamic;
      var s: dynamic;
      read(f, s);
      if ((f == s1))
      {
        s1 = s;
      } else
      {
        s2 = s;
      }
      i += 1;
    }
  }
  write(s1, " ", s2, "\n");
  return 0;
}
