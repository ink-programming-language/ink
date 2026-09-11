// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var cur: dynamic = 1;
  read(a, b);
  var s: dynamic = cpp_uninitialized();
  read(s);
  {
    i = 0;
    while ((i < ((a + b) + 1)))
    {
      if (((((i != a) && (s[i] == cpp_char("-")))) || (((i == a) && (s[i] != cpp_char("-"))))))
      {
        write("No");
        return 0;
      }
      i += 1;
    }
  }
  write("Yes");
}
