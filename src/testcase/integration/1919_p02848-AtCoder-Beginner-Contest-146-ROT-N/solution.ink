// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  read(n, s);
  a = n;
  {
    i = 0;
    while ((i < s.size()))
    {
      s[i] += a;
      if ((s[i] > cpp_char("Z")))
      {
        s[i] -= 26;
      }
      i += 1;
    }
  }
  write(s);
  return 0;
}
