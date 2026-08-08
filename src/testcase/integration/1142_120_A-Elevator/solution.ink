// Translated from solution.cpp.

func main()
{
  var ifile = cpp_construct("input.txt");
  if (ifile)
  {
    freopen("input.txt", "rt", stdin);
  }
  if (ifile)
  {
    freopen("output.txt", "wt", stdout);
  }
  var s: dynamic;
  read(s);
  var r: dynamic;
  read(r);
  if ((s == "front"))
  {
    if ((r == 1))
    {
      write("L");
    } else
    {
      write("R");
    }
  } else
  {
    if ((r == 1))
    {
      write("R");
    } else
    {
      write("L");
    }
  }
  return 0;
}
