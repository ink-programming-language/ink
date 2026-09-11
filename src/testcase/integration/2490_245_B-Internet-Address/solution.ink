// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(s);
  if ((((s[0] == cpp_char("f")) && (s[1] == cpp_char("t"))) && (s[2] == cpp_char("p"))))
  {
    write("ftp://");
    t = 3;
  } else
  {
    write("http://");
    t = 4;
  }
  {
    i = (t + 1);
    while ((i < (s.length() - 1)))
    {
      if (((s[i] == cpp_char("r")) && (s[(i + 1)] == cpp_char("u"))))
      {
        break;
      }
      i += 1;
    }
  }
  {
    var j: dynamic = t;
    while ((j < i))
    {
      write(s[j]);
      j += 1;
    }
  }
  write(".ru");
  if (((i + 2) < s.length()))
  {
    write("/");
    {
      var j: dynamic = (i + 2);
      while ((j < s.length()))
      {
        write(s[j]);
        j += 1;
      }
    }
  }
}
