// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var t: dynamic;
  var s: dynamic;
  read(n, s);
  t = s;
  sort(s.begin(), s.end());
  var ret = ((1 << 21));
  {
    var i = 0;
    while ((i < (s.length() + 1)))
    {
      var dif = 0;
      {
        var i = 0;
        while ((i < s.length()))
        {
          if ((t[i] != s[i]))
          {
            dif += 1;
          }
          i += 1;
        }
      }
      ret = min(ret, dif);
      rotate(s.begin(), (s.begin() + 1), s.end());
      i += 1;
    }
  }
  write((ret / 2), "\n");
}
