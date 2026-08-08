// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var cR = 0;
  var cB = 0;
  read(n);
  while (cpp_update(n, "--"))
  {
    var s: dynamic;
    read(s);
    if ((s == cpp_char("R")))
    {
      cR += 1;
    } else
    {
      cB += 1;
    }
  }
  if ((cR > cB))
  {
    write("Yes");
  } else
  {
    write("No");
  }
  return 0;
}
