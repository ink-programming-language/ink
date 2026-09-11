// Translated from solution.cpp.

func main() -> dynamic
{
  var ifile: dynamic = cpp_construct("input.txt");
  if (ifile)
  {
    freopen("input.txt", "rt", stdin);
  }
  if (ifile)
  {
    freopen("output.txt", "wt", stdout);
  }
  var s: dynamic = cpp_uninitialized();
  read(s);
  var r: dynamic = cpp_uninitialized();
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
