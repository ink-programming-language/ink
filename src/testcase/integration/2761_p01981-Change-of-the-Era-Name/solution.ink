// Translated from solution.cpp.

func solve()
{
  while (true)
  {
    var str: dynamic;
    read(str);
    if ((str == "#"))
    {
      return;
    }
    var y: dynamic;
    var m: dynamic;
    var d: dynamic;
    read(y, m, d);
    if (((y < 31) || (((y == 31) && (m < 5)))))
    {
      write("HEISEI ", y, " ", m, " ", d, "\n");
    } else
    {
      write("? ", (y - 30), " ", m, " ", d, "\n");
    }
  }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  solve();
  return 0;
}
