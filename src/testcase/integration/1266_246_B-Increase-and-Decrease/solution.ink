// Translated from solution.cpp.

func abs(a: dynamic) -> dynamic
{
  return ( (((a < 0))) ? (-a) : a);
}

func sqr(a: dynamic) -> dynamic
{
  return (a * a);
}

func solve1() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(n);
  var sum: dynamic = 0;
  {
    var i: dynamic = cpp_cast(0);
    while ((i < cpp_cast((n))))
    {
      read(t);
      sum += t;
      i += 1;
    }
  }
  if (((sum % n) == 0))
  {
    write(n);
  } else
  {
    write((n - 1));
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve1();
  }
  return 0;
}
