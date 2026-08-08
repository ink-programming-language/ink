// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  read(a);
  var ls = [];
  var u = 0;
  while ((cin >> a))
  {
    if ((a < 3200))
    {
      ls.set((a / 400));
    } else
    {
      u += 1;
    }
  }
  write(max(static_cast(ls.count()), 1), " ", (ls.count() + u), "\n");
  return 0;
}
