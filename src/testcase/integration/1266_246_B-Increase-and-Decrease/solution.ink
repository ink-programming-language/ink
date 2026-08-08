// Translated from solution.cpp.

func abs(a: dynamic)
{
  return (if (((a < 0))) (-a) else a);
}

func sqr(a: dynamic)
{
  return (a * a);
}

func solve1()
{
  var n: dynamic;
  var t: dynamic;
  read(n);
  var sum = 0;
  {
    var i = cpp_cast(0);
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(0);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve1();
  }
  return 0;
}
