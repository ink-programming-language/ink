// Translated from solution.cpp.

func main()
{
  var l: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var r: dynamic;
  read(l, r);
  if (((((r - l) < 2)) || ((((r - l) == 2) && ((l % 2) == 1)))))
  {
    write(-1);
  } else
  {
    if (((l % 2) == 0))
    {
      a = l;
      b = (a + 1);
      c = (b + 1);
    } else
    {
      a = (l + 1);
      b = (a + 1);
      c = (b + 1);
    }
    write(a, cpp_char(" "), b, cpp_char(" "), c);
  }
  return 0;
}
