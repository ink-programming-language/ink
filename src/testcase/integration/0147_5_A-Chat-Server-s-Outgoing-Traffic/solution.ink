// Translated from solution.cpp.

func main()
{
  var m = 0;
  var a = 0;
  {
    var i = 0;
    while ((i < 100))
    {
      var s: dynamic;
      getline(cin, s);
      if ((s[0] == cpp_char("+")))
      {
        m += 1;
        i += 1;
        continue;
      }
      if ((s[0] == cpp_char("-")))
      {
        m -= 1;
        i += 1;
        continue;
      }
      var c = 1;
      var t = 0;
      {
        var j = 0;
        while ((j < s.size()))
        {
          if (((s[j] == cpp_char(":")) && (c == 1)))
          {
            c = 0;
            j += 1;
            continue;
          }
          if ((c == 0))
          {
            a += (((s.size() - j)) * m);
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(a);
  return 0;
}
