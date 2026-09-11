// Translated from solution.cpp.

func main() -> dynamic
{
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(s1, s2);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
       ((i == n)) ? (((cout << s1) << " ") << s2) : ((((cout << s1) << " ") << s2) << endl);
      var f: dynamic = cpp_uninitialized();
      var s: dynamic = cpp_uninitialized();
      read(f, s);
      if ((f == s1))
      {
        s1 = s;
      } else
      {
        s2 = s;
      }
      i += 1;
    }
  }
  write(s1, " ", s2, "\n");
  return 0;
}
