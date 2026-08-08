// Translated from solution.cpp.

func main()
{
  var a = 0;
  var b = 0;
  var c = 0;
  var S: dynamic;
  read(S);
  var n = S.length();
  {
    var i = 0;
    while ((i < n))
    {
      var val = (S[i] - cpp_char("a"));
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
  var ab = abs((a - b));
  var bc = abs((b - c));
  var ca = abs((c - a));
  if ((((ab < 2) && (bc < 2)) && (ca < 2)))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
