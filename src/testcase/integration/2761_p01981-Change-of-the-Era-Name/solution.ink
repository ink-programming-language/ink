// Translated from solution.cpp.

func solve() -> dynamic
{
  while (true)
  {
    var str: dynamic = cpp_uninitialized();
    read(str);
    if ((str == "#"))
    {
      return;
    }
    var y: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var d: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  solve();
  return 0;
}
