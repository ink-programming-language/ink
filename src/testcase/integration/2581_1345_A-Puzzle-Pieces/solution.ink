// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    if (((n == 1) || (m == 1)))
    {
      write("YES", "\n");
    } else if (((n == 2) && (m == 2)))
    {
      write("YES", "\n");
    } else
    {
      write("NO", "\n");
    }
  }
  return 0;
}
