// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  t = s;
  sort(s.begin(), s.end());
  var ret: dynamic = ((1 << 21));
  {
    var i: dynamic = 0;
    while ((i < (s.length() + 1)))
    {
      var dif: dynamic = 0;
      {
        var i: dynamic = 0;
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
