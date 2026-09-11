// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var sz: dynamic = s.size();
  {
    var i: dynamic = 0;
    while ((i < sz))
    {
      if ((s.substr(i) == t))
      {
        write("Yes", "\n");
        return 0;
      }
      s += s[i];
      i += 1;
    }
  }
  write("No", "\n");
}
