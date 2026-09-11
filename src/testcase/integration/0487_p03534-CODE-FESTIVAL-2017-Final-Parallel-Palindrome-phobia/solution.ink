// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = 0;
  var b: dynamic = 0;
  var c: dynamic = 0;
  var S: dynamic = cpp_uninitialized();
  read(S);
  var n: dynamic = S.length();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var val: dynamic = (S[i] - cpp_char("a"));
      if ((val == 0))
      {
        a += 1;
      }
      if ((val == 1))
      {
        b += 1;
      }
      if ((val == 2))
      {
        c += 1;
      }
      i += 1;
    }
  }
  var ab: dynamic = abs((a - b));
  var bc: dynamic = abs((b - c));
  var ca: dynamic = abs((c - a));
  if ((((ab < 2) && (bc < 2)) && (ca < 2)))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
