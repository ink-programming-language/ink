// Translated from solution.cpp.

func main() -> dynamic
{
  var m: dynamic = 0;
  var a: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 100))
    {
      var s: dynamic = cpp_uninitialized();
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
      var c: dynamic = 1;
      var t: dynamic = 0;
      {
        var j: dynamic = 0;
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
