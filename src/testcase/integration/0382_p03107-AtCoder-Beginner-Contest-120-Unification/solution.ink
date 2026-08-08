// Translated from solution.cpp.

func main()
{
  var S: dynamic;
  read(S);
  var c0: dynamic;
  var c1 = 0;
  for (var i in S)
  {
    if ((i == cpp_char("0")))
    {
      c0 += 1;
    } else
    {
      c1 += 1;
    }
  }
  write((2 * min(c0, c1)), "\n");
}
