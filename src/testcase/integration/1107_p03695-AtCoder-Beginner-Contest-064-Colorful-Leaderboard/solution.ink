// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  read(a);
  var ls: dynamic = [];
  var u: dynamic = 0;
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
