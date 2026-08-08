// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  var n: dynamic;
  read(n, s);
  var cnt0 = 0;
  var cnt1 = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((s[i] == cpp_char("0")))
      {
        cnt0 += 1;
      } else
      {
        cnt1 += 1;
      }
      i += 1;
    }
  }
  if ((cnt0 != cnt1))
  {
    write("1", "\n");
    write(s, "\n");
  } else
  {
    write("2", "\n");
    write(s[0], " ", s.substr(1, n));
  }
  return 0;
}
