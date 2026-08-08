// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  var t = "keyence";
  read(s);
  {
    var i = 0;
    while ((i < s.length()))
    {
      {
        var j = (i - 1);
        while ((j < s.length()))
        {
          if (((s.substr(0, i) + s.substr((j + 1), ((s.length() - j) + 1))) == t))
          {
            write("YES", "\n");
            return 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write("NO", "\n");
  return 0;
}
