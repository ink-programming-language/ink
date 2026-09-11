// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = "keyence";
  read(s);
  {
    var i: dynamic = 0;
    while ((i < s.length()))
    {
      {
        var j: dynamic = (i - 1);
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
