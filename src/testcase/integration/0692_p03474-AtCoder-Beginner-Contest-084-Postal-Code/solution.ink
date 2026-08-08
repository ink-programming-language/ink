// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
  var cur = 1;
  read(a, b);
  var s: dynamic;
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
