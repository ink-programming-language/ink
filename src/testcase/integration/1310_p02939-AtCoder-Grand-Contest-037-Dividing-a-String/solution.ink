// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var s1: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(s);
  {
    var i: dynamic = 0;
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
