// Translated from solution.cpp.

var dx = [1, -1, 0, 0, 1, 1, -1, -1, 0];

var dy = [0, 0, 1, -1, 1, -1, 1, -1, 0];

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var s: dynamic;
  var i: dynamic;
  var f = 1;
  read(s);
  if ((s.length() == 1))
  {
    if (((((((s.front() == cpp_char("a")) || (s.front() == cpp_char("e"))) || (s.front() == cpp_char("i"))) || (s.front() == cpp_char("o"))) || (s.front() == cpp_char("u"))) || (s.front() == cpp_char("n"))))
    {
      write("YES");
    } else
    {
      write("NO");
    }
  } else
  {
    {
      i = 0;
      while ((i < (s.length() - 1)))
      {
        if (((((((s[i] != cpp_char("a")) && (s[i] != cpp_char("e"))) && (s[i] != cpp_char("o"))) && (s[i] != cpp_char("i"))) && (s[i] != cpp_char("u"))) && (s[i] != cpp_char("n"))))
        {
          if ((((((s[(i + 1)] != cpp_char("a")) && (s[(i + 1)] != cpp_char("e"))) && (s[(i + 1)] != cpp_char("i"))) && (s[(i + 1)] != cpp_char("o"))) && (s[(i + 1)] != cpp_char("u"))))
          {
            f = 0;
            break;
          } else
          {
            i += 1;
            continue;
          }
        } else
        {
          i += 1;
          continue;
        }
        i += 1;
      }
    }
    if (((((((s.back() != cpp_char("a")) && (s.back() != cpp_char("e"))) && (s.back() != cpp_char("i"))) && (s.back() != cpp_char("o"))) && (s.back() != cpp_char("u"))) && (s.back() != cpp_char("n"))))
    {
      f = 0;
    }
    if ((f == 1))
    {
      write("YES");
    } else
    {
      write("NO");
    }
  }
  return 0;
}
