// Translated from solution.cpp.

func choice(a: dynamic, b: dynamic)
{
  var opt1 = min((a / 2), b);
  a -= (2 * opt1);
  b -= opt1;
  var opt2 = min(a, (b / 2));
  return (opt1 + opt2);
}

func solve()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  if (((((a + b)) / 3) <= min(a, b)))
  {
    write((((a + b)) / 3), "\n");
    return;
  }
  write(min(a, b), "\n");
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
